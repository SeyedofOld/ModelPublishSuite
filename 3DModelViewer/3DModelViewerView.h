
// 3DModelViewerView.h : interface of the CMy3DModelViewerView class
//

#pragma once

#include "tlCCamera.h"

#define WUM_INTERACTION_MSG			WM_USER + 1356

class CMy3DModelViewerView : public CView
{
public:
	struct BUTTON_STATUS {

		BOOL bLButton;
		BOOL bRButton;
		BOOL bMButton;

	};

	enum INTERACTION_EVENT { MOUSE_ENTER, MOUSE_LEAVE, MOUSE_LBDOWN, MOUSE_LBUP, MOUSE_RBDOWN, MOUSE_RBUP, MOUSE_MOVE, MOUSE_DRAG_START, MOUSE_DRAGGING, MOUSE_DRAG_END, MOUSE_WHEEL };

	struct VECTOR2 {
		VECTOR2() { };
		VECTOR2(float _x, float _y) {
			x = _x;
			y = _y;
		}
		float x, y;
	};

	struct INTERACTION_MSG_DATA {
		INTERACTION_EVENT		eEvent;
		BUTTON_STATUS			ButtonStatus;
		VECTOR2	ptCursor;
		int						nData;
	};

	struct RANGE {
		RANGE() { fMin = 0.0f; fMax = 1.0f; }
		RANGE(float min, float max) {
			fMin = min;
			fMax = max;
		}
		float fMin, fMax;
	};

	struct VIEWPORT {
		VIEWPORT();
		VIEWPORT(RANGE _Horiz, RANGE _Vert) {
			Horiz = _Horiz;
			Vert = _Vert;
		}
		VIEWPORT(float x1, float y1, float x2, float y2) {
			Horiz.fMin = x1;
			Horiz.fMax = x2;
			Vert.fMin = y1;
			Vert.fMax = y2;
		}
		RANGE Horiz;
		RANGE Vert;
	};

protected: // create from serialization only
	CMy3DModelViewerView();
	DECLARE_DYNCREATE(CMy3DModelViewerView)

// Attributes
public:
	CMy3DModelViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMy3DModelViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	VECTOR2 ScreenToRenderPort(CPoint pt);
	VECTOR2 ScreenToRenderPort(VECTOR2 pt);
	VECTOR2 RenderPortToScreen(VECTOR2 ptPos);
	CPoint RenderPortToScreenPixel(VECTOR2 ptPos);
	CPoint GetCursorPosPixel() const { return CPoint((int)m_ptCursor.x, (int)m_ptCursor.y); }
	VECTOR2 GetCursorPos() const { return m_ptCursor; }

	CMy3DModelViewerView::RANGE GetHorizRange() const { return m_HorizRange; }
	void SetHorizRange(CMy3DModelViewerView::RANGE val) { m_HorizRange = val; }

	CMy3DModelViewerView::RANGE GetVertRange() const { return m_HorizRange; }
	void SetVertRange(CMy3DModelViewerView::RANGE val) { m_HorizRange = val; }

	void SetRange(float x1, float y1, float x2, float y2);

	HWND GetCallbackHwnd() const { return m_hwndCallback; }
	void SetCallbackHwnd(HWND val) { m_hwndCallback = val; }
	BUTTON_STATUS GetButtonStatus();


protected:
	BUTTON_STATUS m_ButtonStatus;
	VECTOR2		m_ptCursor;
	BOOL		m_bDragging;
	RANGE		m_HorizRange;
	RANGE		m_VertRange;
	HWND		m_hwndCallback;
	CBitmap*	m_pBitmap;
	CCamera		m_Camera;
	ID3DXMesh*  m_pMesh;


	afx_msg void OnStnClicked();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	virtual void PreSubclassWindow();

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void ProcessMouseInput(INTERACTION_MSG_DATA& imd);

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	//afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	bool LoadModelFromMemory ( void * pData, DWORD dwDataSize );
	afx_msg void OnSize ( UINT nType, int cx, int cy );
};

#ifndef _DEBUG  // debug version in 3DModelViewerView.cpp
inline CMy3DModelViewerDoc* CMy3DModelViewerView::GetDocument() const
   { return reinterpret_cast<CMy3DModelViewerDoc*>(m_pDocument); }
#endif

