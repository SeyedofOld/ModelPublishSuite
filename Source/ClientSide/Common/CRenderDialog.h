/**************************************************************************************
*                                                                                     *
*   Copyright(C) 2014 Tochal Advanced Systems Co., Ltd. (www.tochalco.com)            *
*   All rights reserved.                                                              *
*                                                                                     *
***************************************************************************************/

#pragma once

// CRenderPanel

#include <afxwin.h>
#include <afxdialogex.h>


#define WUM_INTERACTION_MSG			WM_USER + 1356


typedef void (*FOnPaint)( CDC*pDc ) ;


struct BUTTON_STATUS {

	BOOL bLButton ;
	BOOL bRButton ;
	BOOL bMButton ;

} ;

enum INTERACTION_EVENT { MOUSE_ENTER , MOUSE_LEAVE , MOUSE_LBDOWN , MOUSE_LBUP , MOUSE_RBDOWN , MOUSE_RBUP , MOUSE_MOVE , MOUSE_DRAG_START , MOUSE_DRAGGING , MOUSE_DRAG_END , MOUSE_WHEEL } ;

class CRenderDialog ;



class CRenderDialog : public CDialogEx
{
	//DECLARE_DYNAMIC(CRenderPanel)
public:
	struct VECTOR2 {
		VECTOR2() { } ;
		VECTOR2(float _x , float _y) {
			x = _x ;
			y = _y ;
		}
		float x , y ;
	};

	struct RANGE {
		RANGE() { fMin = 0.0f ; fMax = 1.0f ; } 
		RANGE(float min , float max) {
			fMin = min ;
			fMax = max ;
		}
		float fMin , fMax ;
	};

	struct VIEWPORT {
		VIEWPORT() ;
		VIEWPORT ( RANGE _Horiz , RANGE _Vert ) {
			Horiz = _Horiz ;
			Vert = _Vert ;
		}
		VIEWPORT ( float x1 , float y1 , float x2 , float y2 ) {
			Horiz.fMin = x1 ;
			Horiz.fMax = x2 ;
			Vert.fMin = y1 ;
			Vert.fMax = y2 ;
		}
		RANGE Horiz ;
		RANGE Vert ;
	};

public:
	CRenderDialog();
	CRenderDialog ( UINT nIDTemplate, CWnd *pParent = NULL );
	virtual ~CRenderDialog();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnStnClicked();
//	afx_msg void OnPaint();

	void SetBitmap ( CBitmap *pBitmap ) { m_pBitmap = pBitmap ; }
	VECTOR2 ScreenToRenderPort ( CPoint pt ) ;
	VECTOR2 ScreenToRenderPort ( VECTOR2 pt ) ;
	VECTOR2 RenderPortToScreen ( VECTOR2 ptPos ) ;
	CPoint RenderPortToScreenPixel ( VECTOR2 ptPos ) ;
	CPoint GetCursorPosPixel() const { return CPoint((int)m_ptCursor.x , (int)m_ptCursor.y); }
	VECTOR2 GetCursorPos() const { return m_ptCursor; }

	CRenderDialog::RANGE GetHorizRange() const { return m_HorizRange; }
	void SetHorizRange(CRenderDialog::RANGE val) { m_HorizRange = val; }

	CRenderDialog::RANGE GetVertRange() const { return m_HorizRange; }
	void SetVertRange(CRenderDialog::RANGE val) { m_HorizRange = val; }

	void SetRange ( float x1 , float y1 , float x2 , float y2 ) ;

	HWND GetCallbackHwnd() const { return m_hwndCallback; }
	void SetCallbackHwnd(HWND val) { m_hwndCallback = val; }

private:
	
	VECTOR2 m_ptCursor ;
	BUTTON_STATUS m_ButtonStatus ;
	BUTTON_STATUS GetButtonStatus() ;
	BOOL	m_bDragging ;

protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	CBitmap* m_pBitmap ;
	RANGE m_HorizRange ;
	RANGE m_VertRange ;
	HWND m_hwndCallback ;
	FOnPaint m_pOnPaint ;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	virtual void PreSubclassWindow() ;
	void SetOnPaint(FOnPaint val) { m_pOnPaint = val; }

afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

struct INTERACTION_MSG_DATA {
	INTERACTION_EVENT		eEvent ;
	BUTTON_STATUS			ButtonStatus ;
	CRenderDialog::VECTOR2	ptCursor ;
	int						nData ;
} ;

