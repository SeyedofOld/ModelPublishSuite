/**************************************************************************************
*                                                                                     *
*   Copyright(C) 2014 Tochal Advanced Systems Co., Ltd. (www.tochalco.com)            *
*   All rights reserved.                                                              *
*                                                                                     *
***************************************************************************************/

// RenderPanel.cpp : implementation file
//

//#include "stdafx.h"
#include "CRenderDialog.h"
#include <minwindef.h>
#include <wtypes.h>


// CRenderPanel

//IMPLEMENT_DYNAMIC(CRenderPanel, CStatic)

CRenderDialog::CRenderDialog()
{
	m_bDragging = FALSE ;
	m_pBitmap = NULL ;
	m_hwndCallback = NULL ;
	m_pOnPaint = NULL ;
}

CRenderDialog::CRenderDialog ( UINT nIDTemplate, CWnd *pParent /*= NULL */ ) : CDialogEx ( nIDTemplate, pParent )
{

}

CRenderDialog::~CRenderDialog()
{
}


BEGIN_MESSAGE_MAP(CRenderDialog, /*CStatic*/CDialogEx)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CRenderPanel message handlers




CRenderDialog::VECTOR2 CRenderDialog::ScreenToRenderPort( VECTOR2 pt )
{
	float fX = pt.x ;
	float fY = pt.y ;

	CRect rcClient ;
	GetClientRect ( rcClient ) ;

	fX = fX * (m_HorizRange.fMax - m_HorizRange.fMin) / (float)rcClient.Width() + m_HorizRange.fMin ;
	fY = fY * (m_VertRange.fMax - m_VertRange.fMin) / (float)rcClient.Height() + m_VertRange.fMin ;

	return VECTOR2 ( fX , fY ) ;
}

CRenderDialog::VECTOR2 CRenderDialog::ScreenToRenderPort( CPoint pt )
{
	VECTOR2 pt2 ( (float)pt.x , (float)pt.y ) ;
	
	return ScreenToRenderPort ( pt2 ) ;
}

CRenderDialog::VECTOR2 CRenderDialog::RenderPortToScreen( VECTOR2 ptPos )
{
	CRect rcClient ;
	GetClientRect ( rcClient ) ;

	float fX = ( ptPos.x - m_HorizRange.fMin ) * rcClient.Width() / ( m_HorizRange.fMax - m_HorizRange.fMin ) ;
	float fY = ( ptPos.y - m_VertRange.fMin ) * rcClient.Height() / ( m_VertRange.fMax - m_VertRange.fMin ) ;

	VECTOR2 pt ( fX , fY ) ;

	return pt ;
}

CPoint CRenderDialog::RenderPortToScreenPixel ( VECTOR2 ptPos )
{
	VECTOR2 ptScreen = RenderPortToScreen ( ptPos ) ;
	CPoint pt ( (int)ptScreen.x , (int)ptScreen.y ) ;

	return pt ;
}

BOOL CRenderDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
// 	if ( /*message == WM_NCHITTEST ||*/ message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK)
// 		return CDialogEx::DefWindowProc(message, wParam, lParam);

	if ( message == WM_MOUSEHOVER ) {

		TRACKMOUSEEVENT tme ;
		tme.cbSize = sizeof ( TRACKMOUSEEVENT ) ;
		tme.dwFlags = /*TME_HOVER |*/ TME_LEAVE ;
		tme.hwndTrack = GetSafeHwnd () ;
		tme.dwHoverTime = 1 ;
		TrackMouseEvent ( &tme ) ;

		Invalidate ( ) ;
	}

	if ( message == WM_MOUSELEAVE ) {

		TRACKMOUSEEVENT tme ;
		tme.cbSize = sizeof ( TRACKMOUSEEVENT ) ;
		tme.dwFlags = TME_HOVER ;//| TME_LEAVE ;
		tme.hwndTrack = GetSafeHwnd () ;
		tme.dwHoverTime = 1 ;
		TrackMouseEvent ( &tme ) ;

		Invalidate ( ) ;
	}

	return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}

BUTTON_STATUS CRenderDialog::GetButtonStatus()
{
	m_ButtonStatus.bLButton = GetAsyncKeyState(VK_LBUTTON)&0x8000 ;
	m_ButtonStatus.bRButton = GetAsyncKeyState(VK_RBUTTON)&0x8000 ;
	m_ButtonStatus.bMButton = GetAsyncKeyState(VK_MBUTTON)&0x8000 ;

	return m_ButtonStatus ;
}

// void CRenderPanel::OnPaint()
// {
// 	return ;
// 
// 	CPaintDC dc(this); // device context for painting
// 	// TODO: Add your message handler code here
// 	// Do not call CStatic::OnPaint() for painting messages
// 
// 	if ( m_pOnPaint )
// 		m_pOnPaint ( &dc ) ;
// 
// //	if ( 0 )
// 
// // 	CBrush br ;
// // 	br.CreateSolidBrush(0) ;
// // 	CRect rc ;
// // 	GetClientRect ( rc ) ;
// // 	dc.FillRect ( rc , &br ) ;
// // 	br.DeleteObject() ;
// // 	CRect rc ;
// // 	GetClientRect(rc) ;
// // 	ClientToScreen(rc) ;
// // 	CDC* pDC = GetParent()->GetDC() ;
// // 	pDC->BitBlt(0 , 0 , rc.Width() , rc.Height() , &dc  , 0 , 0 , SRCCOPY) ;
// 
// }

void CRenderDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	// 	CStatic::OnMouseMove ( nFlags , point ) ;
	// 	return ;


	m_ptCursor = ScreenToRenderPort ( VECTOR2((float)point.x,(float)point.y) ) ;

	BOOL bMouseDown = FALSE ;
	bMouseDown |= GetAsyncKeyState ( VK_LBUTTON ) & 0x8000 ;
	bMouseDown |= GetAsyncKeyState ( VK_RBUTTON ) & 0x8000 ;
	bMouseDown |= GetAsyncKeyState ( VK_MBUTTON ) & 0x8000 ;

	BOOL bNewTrack = TRUE ;
	{
		TRACKMOUSEEVENT tme ;
		tme.cbSize = sizeof ( TRACKMOUSEEVENT ) ;
		tme.dwFlags = TME_QUERY ;
		tme.hwndTrack = GetSafeHwnd () ;
		tme.dwHoverTime = 0 ;
		TrackMouseEvent ( &tme ) ;

		if ( tme.dwFlags & TME_HOVER )
			bNewTrack = FALSE ;
		if ( tme.dwFlags & TME_LEAVE )
			bNewTrack = FALSE ;
	}

	if ( bNewTrack ) {
		TRACKMOUSEEVENT tme ;
		tme.cbSize = sizeof ( TRACKMOUSEEVENT ) ;
		tme.dwFlags = TME_HOVER ;//| TME_LEAVE ;
		tme.hwndTrack = GetSafeHwnd () ;
		tme.dwHoverTime = 0 ;
		TrackMouseEvent ( &tme ) ;

		INTERACTION_MSG_DATA imd ;
		imd.ButtonStatus = GetButtonStatus() ;
		imd.ptCursor = m_ptCursor ;
		imd.eEvent = MOUSE_ENTER ;
		::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
		SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

	} else {

		INTERACTION_MSG_DATA imd ;
		imd.ButtonStatus = GetButtonStatus() ;
		imd.ptCursor = m_ptCursor ;

		if ( !m_bDragging && bMouseDown ) {
			imd.eEvent = MOUSE_DRAG_START ;
			::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			m_bDragging = TRUE ;
		} else if ( m_bDragging && !bMouseDown ) {
			imd.eEvent = MOUSE_DRAG_END ;
			::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			m_bDragging = FALSE ;
		} else if ( m_bDragging && bMouseDown ) {
			imd.eEvent = MOUSE_DRAGGING ;
			::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
		} else {
			if ( !m_bDragging && !bMouseDown ) {
				imd.eEvent = MOUSE_MOVE ;
				::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
				SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
			}
		}

	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CRenderDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd ;
	imd.ButtonStatus = GetButtonStatus() ;
	imd.ptCursor = m_ptCursor ;
	imd.eEvent = MOUSE_LBDOWN ;
	::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
	SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;


	CDialogEx::OnLButtonDown(nFlags, point);
}

void CRenderDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	INTERACTION_MSG_DATA imd ;
	imd.ButtonStatus = GetButtonStatus() ;
	imd.ptCursor = m_ptCursor ;
	imd.eEvent = MOUSE_LBUP ;

	::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
	SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

	if ( m_bDragging ) {
		imd.eEvent = MOUSE_DRAG_END ;
		::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
		SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

		m_bDragging = FALSE ;
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CRenderDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd ;
	imd.ButtonStatus = GetButtonStatus() ;
	imd.ptCursor = m_ptCursor ;
	imd.eEvent = MOUSE_RBDOWN ;

	::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
	SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

	CDialogEx::OnRButtonDown(nFlags, point);
}

void CRenderDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd ;
	imd.ButtonStatus = GetButtonStatus() ;
	imd.ptCursor = m_ptCursor ;
	imd.eEvent = MOUSE_RBUP ;

	::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
	SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

	if ( m_bDragging ) {
		imd.eEvent = MOUSE_DRAG_END ;
		::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
		SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

		m_bDragging = FALSE ;
	}

	CDialogEx::OnRButtonUp(nFlags, point);
}

BOOL CRenderDialog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd ;
	imd.ButtonStatus = GetButtonStatus() ;
	imd.ptCursor = m_ptCursor ;
	imd.eEvent = MOUSE_WHEEL ;
	imd.nData = zDelta ;
	::SendMessage ( m_hwndCallback , WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;
	SendMessage ( WUM_INTERACTION_MSG , 0 , (LPARAM)&imd ) ;

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CRenderDialog::PreSubclassWindow()
{
	ModifyStyle ( 0 , SS_NOTIFY , 0 ) ;
	ModifyStyleEx ( WS_EX_LAYOUTRTL , 0 , 0 ) ;
	
	CRect rc ;
	GetClientRect ( &rc ) ;

	SetRange ( 0 , 0 , (float)rc.Width() , (float)rc.Height() ) ;

	m_hwndCallback = GetParent()->GetSafeHwnd() ;

	CDialogEx::PreSubclassWindow() ;
}

void CRenderDialog::SetRange( float x1 , float y1 , float x2 , float y2 )
{
	VECTOR2 pt = RenderPortToScreen ( m_ptCursor ) ;
	m_HorizRange = RANGE ( x1 , x2 ) ;
	m_VertRange  = RANGE ( y1 , y2 ) ;
	m_ptCursor = ScreenToRenderPort ( pt ) ;
}

BOOL CRenderDialog::OnEraseBkgnd(CDC* pDC)
{
	return TRUE ;

	return CDialogEx::OnEraseBkgnd(pDC);
}
