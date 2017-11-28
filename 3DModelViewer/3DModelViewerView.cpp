//
// 3DModelViewerView.cpp : implementation of the CMy3DModelViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "3DModelViewer.h"
#endif
#include "MainFrm.h"
#include "3DModelViewerDoc.h"
#include "3DModelViewerView.h"
#include "tlC3DGfx.h"
#include "tlCGuiRenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CSettingsGui* g_pSettings ;
extern CMy3DModelViewerView* g_pView ;
extern float g_fYaw ;

// CMy3DModelViewerView

IMPLEMENT_DYNCREATE(CMy3DModelViewerView, CView)

BEGIN_MESSAGE_MAP(CMy3DModelViewerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMy3DModelViewerView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	////////////////////////////
	ON_CONTROL_REFLECT(STN_CLICKED, &OnStnClicked)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE ()
END_MESSAGE_MAP()

// CMy3DModelViewerView construction/destruction

CMy3DModelViewerView::CMy3DModelViewerView()
{
	m_bDragging = FALSE;
	m_pBitmap = NULL;
	m_hwndCallback = NULL;

	m_pMesh = NULL ;

	m_ptPos = vector3 ( 0, 0, 0 ) ;
	m_fPitch = 0.0f ;
	m_fYaw = 0.0f ;
	D3DXMatrixIdentity ( &m_matWorld ) ;
}

CMy3DModelViewerView::~CMy3DModelViewerView()
{
}

BOOL CMy3DModelViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	m_Camera.Initialize ( D3DXToRadian ( 45.0f ), 4.0f / 3.0f, 0.01f, 1000.0f );
	m_Camera.SetMode ( CCamera::MODE_TARGET );
	m_Camera.SetPosition ( 0.0f, 0.0f, -10.0f );
	m_Camera.SetTarget ( 0.0f, 0.0f, 0.0f );

	CCamera *pCamera = new CCamera;
	pCamera->Initialize(D3DXToRadian(45.0f), 4.0f / 3.0f, 0.01f, 1000.0f);
	pCamera->SetPosition(0.0f, 0.0f, -2.0f);
	pCamera->SetTarget(0.0f, 0.0f, 0.0f);
	pCamera->SetMode(CCamera::MODE_TARGET);
	pCamera->SetMode(CCamera::MODE_FREE);


	return CView::PreCreateWindow(cs);
}

// CMy3DModelViewerView drawing

void CMy3DModelViewerView::OnDraw(CDC* pDC)
{
	CMy3DModelViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	g_pSettings = &m_SettingsGui ;

	bool bSingleView = GetDocument ()->m_d3dMesh1.Parts.size () != 0 ;
	bool bDualView = bSingleView && (GetDocument()->m_d3dMesh2.Parts.size() != 0) ;

	UpdateObjectMatrix();

	if ( bDualView ) {
		m_Camera.SetAspect ( 0.5f * (float)C3DGfx::GetInstance()->GetFullscreenViewport().Width / C3DGfx::GetInstance()->GetFullscreenViewport().Height ) ;
	}

	if ( C3DGfx::GetInstance () ) {
		C3DGfx::GetInstance ()->BeginFrame ();
		C3DGfx::GetInstance ()->Clear ( 0xff200040 ) ;

		m_Camera.SetD3DCamera ( C3DGfx::GetInstance ()->GetDevice () ) ;

		//C3DGfx::GetInstance ()->GetDevice ()->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;



// 		shared float4x4 g_matSunLight : SUNLIGHT ;		// Sun Light characteristics
// 														// Row1 : Direction
// 														// Row2 : Diffuse
// 														// Row3 : Ambient
// 														// Row4 : Reserved
		( (CMainFrame*)AfxGetMainWnd() )->m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix () ) ;
		( (CMainFrame*)AfxGetMainWnd() )->m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;
		( (CMainFrame*)AfxGetMainWnd() )->m_pShader->SetMatrix ( "g_matWorld", &m_matWorld ) ;

		matrix matLight ;

		vector4 vLightDir ( 0.0f, 0.0f, 1.0f, 0.0f ) ;
		D3DXVec4Normalize ( &vLightDir, &vLightDir ) ;
		CopyMemory ( &matLight[0], &vLightDir, 4*sizeof(float) ) ;
		
		vector4 vLightColor ( 1.0f, 1.0f, 1.0f, 0.0f ) ;
		CopyMemory ( &matLight[4], &vLightColor, 4*sizeof(float) ) ;
		
		vector4 vAmbLight ( 0.5f, 0.5f, 0.5f, 0.0f ) ;
		CopyMemory ( &matLight[8], &vAmbLight, 4*sizeof(float) ) ;

		((CMainFrame*)AfxGetMainWnd())->m_pShader->SetMatrix ( "g_matSunLight", &matLight ) ;

		if ( bDualView ) {
			D3DVIEWPORT9 vp = C3DGfx::GetInstance()->GetFullscreenViewport() ;
			vp.Width /= 2 ;
			vp.X = 0 ;
			pDevice->SetViewport ( &vp ) ;
			CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh1 ) ;

			vp = C3DGfx::GetInstance ()->GetFullscreenViewport () ;
			vp.Width /= 2 ;
			vp.X = vp.Width ;
			pDevice->SetViewport ( &vp ) ;
			CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh2 ) ;

			pDevice->SetViewport ( &C3DGfx::GetInstance()->GetFullscreenViewport() ) ;
		}
		else if ( bSingleView ) {
			pDevice->SetViewport ( &C3DGfx::GetInstance ()->GetFullscreenViewport () ) ;
			CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh1 ) ;
		}
		else
			if ( m_pMesh )
				m_pMesh->DrawSubset ( 0 );

		//CGuiRenderer::Update ( 0.01f ) ;
		//m_SettingsGui.Update () ;
		//CGuiRenderer::Render () ;

		C3DGfx::GetInstance ()->EndFrame ();
		C3DGfx::GetInstance ()->ShowFrame ( NULL, NULL, GetSafeHwnd() );
	}

// 	CRect rc ;
// 	GetClientRect ( rc ) ;
// 	CBrush br ;
// 	br.CreateSolidBrush ( RGB ( 32, 0, 64 ) ) ;
// 
// 	pDC->FillRect ( rc, &br ) ;
}


// CMy3DModelViewerView printing


void CMy3DModelViewerView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMy3DModelViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMy3DModelViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMy3DModelViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMy3DModelViewerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	INTERACTION_MSG_DATA imd;
	imd.ButtonStatus = GetButtonStatus();
	imd.ptCursor = m_ptCursor;
	imd.eEvent = MOUSE_RBUP;

	::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
	SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	if (m_bDragging) {
		imd.eEvent = MOUSE_DRAG_END;
		::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
		SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

		m_bDragging = FALSE;
	}


	ClientToScreen(&point);
	OnContextMenu(this, point);
	//CView::OnRButtonUp(nFlags, point); // Mine
}

void CMy3DModelViewerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	return ;
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMy3DModelViewerView diagnostics

#ifdef _DEBUG
void CMy3DModelViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CMy3DModelViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMy3DModelViewerDoc* CMy3DModelViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy3DModelViewerDoc)));
	return (CMy3DModelViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CMy3DModelViewerView message handlers
void CMy3DModelViewerView::OnStnClicked()
{
	// TODO: Add your control notification handler code here
}


CMy3DModelViewerView::VECTOR2 CMy3DModelViewerView::ScreenToRenderPort(VECTOR2 pt)
{
	float fX = pt.x;
	float fY = pt.y;

	CRect rcClient;
	GetClientRect(rcClient);

	fX = fX * (m_HorizRange.fMax - m_HorizRange.fMin) / (float)rcClient.Width() + m_HorizRange.fMin;
	fY = fY * (m_VertRange.fMax - m_VertRange.fMin) / (float)rcClient.Height() + m_VertRange.fMin;

	return VECTOR2(fX, fY);
}

CMy3DModelViewerView::VECTOR2 CMy3DModelViewerView::ScreenToRenderPort(CPoint pt)
{
	VECTOR2 pt2((float)pt.x, (float)pt.y);

	return ScreenToRenderPort(pt2);
}

CMy3DModelViewerView::VECTOR2 CMy3DModelViewerView::RenderPortToScreen(VECTOR2 ptPos)
{
	CRect rcClient;
	GetClientRect(rcClient);

	float fX = (ptPos.x - m_HorizRange.fMin) * rcClient.Width() / (m_HorizRange.fMax - m_HorizRange.fMin);
	float fY = (ptPos.y - m_VertRange.fMin) * rcClient.Height() / (m_VertRange.fMax - m_VertRange.fMin);

	VECTOR2 pt(fX, fY);

	return pt;
}

CPoint CMy3DModelViewerView::RenderPortToScreenPixel(VECTOR2 ptPos)
{
	VECTOR2 ptScreen = RenderPortToScreen(ptPos);
	CPoint pt((int)ptScreen.x, (int)ptScreen.y);

	return pt;
}

BOOL CMy3DModelViewerView::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (/*message == WM_NCHITTEST ||*/ message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK)
		return ::DefWindowProc(m_hWnd, message, wParam, lParam);

	if (message == WM_MOUSEHOVER) {

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = /*TME_HOVER |*/ TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);

		Invalidate();
	}

	if (message == WM_MOUSELEAVE) {

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER;//| TME_LEAVE ;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);

		Invalidate();
	}

	if (message == WUM_INTERACTION_MSG) {
		ProcessMouseInput(*((INTERACTION_MSG_DATA*)lParam));
	}
	//CGuiRenderer::WndProc(message, wParam, lParam);

	return CView::OnWndMsg(message, wParam, lParam, pResult);
}

CMy3DModelViewerView::BUTTON_STATUS CMy3DModelViewerView::GetButtonStatus()
{
	m_ButtonStatus.bLButton = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	m_ButtonStatus.bRButton = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
	m_ButtonStatus.bMButton = GetAsyncKeyState(VK_MBUTTON) & 0x8000;

	return m_ButtonStatus;
}

// void CMy3DModelViewerView::OnPaint()
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

void CMy3DModelViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	// 	CStatic::OnMouseMove ( nFlags , point ) ;

	m_ptCursor = ScreenToRenderPort(VECTOR2((float)point.x, (float)point.y));

	BOOL bMouseDown = FALSE;
	bMouseDown |= GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	bMouseDown |= GetAsyncKeyState(VK_RBUTTON) & 0x8000;
	bMouseDown |= GetAsyncKeyState(VK_MBUTTON) & 0x8000;

	BOOL bNewTrack = TRUE;
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_QUERY;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = 0;
		TrackMouseEvent(&tme);

		if (tme.dwFlags & TME_HOVER)
			bNewTrack = FALSE;
		if (tme.dwFlags & TME_LEAVE)
			bNewTrack = FALSE;
	}

	if (bNewTrack) {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER;//| TME_LEAVE ;
		tme.hwndTrack = GetSafeHwnd();
		tme.dwHoverTime = 0;
		TrackMouseEvent(&tme);

		INTERACTION_MSG_DATA imd;
		imd.ButtonStatus = GetButtonStatus();
		imd.ptCursor = m_ptCursor;
		imd.eEvent = MOUSE_ENTER;
		::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
		SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	}
	else {

		INTERACTION_MSG_DATA imd;
		imd.ButtonStatus = GetButtonStatus();
		imd.ptCursor = m_ptCursor;

		if (!m_bDragging && bMouseDown) {
			imd.eEvent = MOUSE_DRAG_START;
			::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			m_bDragging = TRUE;
		}
		else if (m_bDragging && !bMouseDown) {
			imd.eEvent = MOUSE_DRAG_END;
			::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			m_bDragging = FALSE;
		}
		else if (m_bDragging && bMouseDown) {
			imd.eEvent = MOUSE_DRAGGING;
			::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
		}
		else {
			if (!m_bDragging && !bMouseDown) {
				imd.eEvent = MOUSE_MOVE;
				::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
				SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
			}
		}

	}

	CView::OnMouseMove(nFlags, point);
}

void CMy3DModelViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd;
	imd.ButtonStatus = GetButtonStatus();
	imd.ptCursor = m_ptCursor;
	imd.eEvent = MOUSE_LBDOWN;
	::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
	SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	CView::OnLButtonDown(nFlags, point);
}

void CMy3DModelViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	INTERACTION_MSG_DATA imd;
	imd.ButtonStatus = GetButtonStatus();
	imd.ptCursor = m_ptCursor;
	imd.eEvent = MOUSE_LBUP;

	::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
	SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	if (m_bDragging) {
		imd.eEvent = MOUSE_DRAG_END;
		::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
		SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

		m_bDragging = FALSE;
	}

	CView::OnLButtonUp(nFlags, point);
}

void CMy3DModelViewerView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd;
	imd.ButtonStatus = GetButtonStatus();
	imd.ptCursor = m_ptCursor;
	imd.eEvent = MOUSE_RBDOWN;

	::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
	SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	CView::OnRButtonDown(nFlags, point);
}

BOOL CMy3DModelViewerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	INTERACTION_MSG_DATA imd;
	imd.ButtonStatus = GetButtonStatus();
	imd.ptCursor = m_ptCursor;
	imd.eEvent = MOUSE_WHEEL;
	imd.nData = zDelta;
	::SendMessage(m_hwndCallback, WUM_INTERACTION_MSG, 0, (LPARAM)&imd);
	SendMessage(WUM_INTERACTION_MSG, 0, (LPARAM)&imd);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMy3DModelViewerView::PreSubclassWindow()
{

	ModifyStyle(0, SS_NOTIFY, 0);
	//ModifyStyleEx ( WS_EX_LAYOUTRTL , 0 , 0 ) ;

	CRect rc;
	GetClientRect(&rc);

	SetRange(0, 0, (float)rc.Width(), (float)rc.Height());

	m_hwndCallback = GetParent()->GetSafeHwnd();

	CView::PreSubclassWindow();
}

void CMy3DModelViewerView::SetRange(float x1, float y1, float x2, float y2)
{
	VECTOR2 pt = RenderPortToScreen(m_ptCursor);
	m_HorizRange = RANGE(x1, x2);
	m_VertRange = RANGE(y1, y2);
	m_ptCursor = ScreenToRenderPort(pt);
}

BOOL CMy3DModelViewerView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	return CView::OnEraseBkgnd(pDC);
}


void CMy3DModelViewerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	/*
	if (bActivate)
		if (CViewManager::GetInstance())
			if (CViewManager::GetInstance()->GetView(L"Editor")) {
				CViewManager::GetInstance()->GetView(L"Editor")->SetHWnd(pActivateView->GetSafeHwnd());

				C3DViewContext* pView = CViewManager::GetInstance()->GetView(L"Editor");

				if (pView) {
					CRect rcView;
					pActivateView->GetClientRect(rcView);
					pView->CreateRenderTarget(rcView.Width(), rcView.Height(), false);
					D3DVIEWPORT9 vp = pView->GetViewport();
					vp.Width = rcView.Width();
					vp.Height = rcView.Height();
					pView->SetViewport(vp);

					pView->GetCamera()->SetAspect((float)rcView.Width() / rcView.Height());

					SetRange(0, 0, (float)rcView.Width(), (float)rcView.Height());

					CEditorStation::GetInstance()->GetPostEffectManager().ResizeBuffers(rcView.Width(), rcView.Height());
					ImGui::GetIO().DisplaySize = ImVec2((float)rcView.Width(), (float)rcView.Height());
				}
			}
	*/
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CMy3DModelViewerView::ProcessMouseInput(INTERACTION_MSG_DATA& imd)
{
	static CMy3DModelViewerView::VECTOR2 s_ptStart = imd.ptCursor;
	static bool s_bCameraMoveEnabled = true;

	if (imd.eEvent == MOUSE_DRAG_START) {
		s_ptStart = imd.ptCursor;
		//s_bCameraMoveEnabled = !ImGui::IsMouseHoveringAnyWindow();
	}

	if (imd.eEvent == MOUSE_DRAGGING && s_bCameraMoveEnabled) {

		float fDeltaX = imd.ptCursor.x - s_ptStart.x;
		float fDeltaY = imd.ptCursor.y - s_ptStart.y;

		if (imd.ButtonStatus.bLButton) { // Dragging Left Button
// 			float fYaw = m_Camera.GetYaw();
// 			fYaw += fDeltaX / 100.0f;
// 
// 			float fPitch = m_Camera.GetPitch();
// 			fPitch += fDeltaY / 100.0f;
// 
// 			m_Camera.SetYaw(fYaw);
// 			m_Camera.SetPitch(fPitch);

			float fYaw = m_fYaw ;
			fYaw -= fDeltaX / 100.0f;
			m_fYaw = fYaw ;

			float fPitch = m_fPitch;
			fPitch -= fDeltaY / 100.0f;
			m_fPitch = fPitch ;

			UpdateObjectMatrix() ;

			RedrawWindow();
		}
		else if (imd.ButtonStatus.bRButton) {
			vector3 vDist = m_Camera.GetTarget() - m_Camera.GetPosition();
			float fDist = D3DXVec3Length(&vDist);

			fDist += fDeltaY/10.0f;

			m_Camera.SetDistance(fDist);

			((CMainFrame*)AfxGetMainWnd())->m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix() ) ;
			((CMainFrame*)AfxGetMainWnd())->m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix() ) ;

			RedrawWindow();
		}

		s_ptStart = imd.ptCursor;
	}

}

void CMy3DModelViewerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
// 	CEditorStation* pEditor = CEditorStation::GetInstance();
// 	if (pEditor) {
// 		if (nChar == VK_F1)
// 			pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_HELP] = !pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_HELP];
// 		if (nChar == VK_F2)
// 			pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_IO] = !pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_IO];
// 		if (nChar == VK_F3)
// 			pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_DEBUG1] = !pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_DEBUG1];
// 		if (nChar == VK_F4)
// 			pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_TERRAIN] = !pEditor->m_bDebugWindowsMask[CEditorStation::PAGE_TERRAIN];
// 	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMy3DModelViewerView::OnInitialUpdate()
{
	ModifyStyle ( 0, SS_NOTIFY ) ;
	if ( GetDocument()->m_d3dMesh1.Parts.size() ) {
		float dy = 1.0f / tanf ( m_Camera.GetFovY () / 2.0f ) ;
		float zy = dy * ( GetDocument ()->m_d3dMesh1.ptMax.y - GetDocument ()->m_d3dMesh1.ptMin.y ) * 1.25f / 2.0f ;

		float dx = 1.0f / tanf ( m_Camera.GetFovX () / 2.0f ) ;
		float zx = dx * ( GetDocument ()->m_d3dMesh1.ptMax.x - GetDocument ()->m_d3dMesh1.ptMin.x ) * 1.25f / 2.0f ;

		float z = zy ;
		if ( zx > zy )
			z = zx ;

		m_Camera.SetDistance ( z ) ;

		//m_SettingsGui.Initialize () ;
		//m_SettingsGui.Update () ;
		//m_SettingsGui.Show ( true ) ;

		m_fYaw = 0.0f ;
		m_fPitch = 0.0f ;
		UpdateObjectMatrix() ;
	}

	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix () ) ;
	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;

	g_pView = this ;

	CView::OnInitialUpdate();
}

bool CMy3DModelViewerView::LoadModelFromMemory ( void* pData, DWORD dwDataSize )
{

	return true;
}


void CMy3DModelViewerView::OnSize ( UINT nType, int cx, int cy )
{
	CView::OnSize ( nType, cx, cy );

	//SetRange(-1.0f, 1.0f, 1.0f, -1.0f);
	CRect rc;
	GetClientRect ( rc );
	SetRange ( 0, 0, (float)rc.Width (), (float)rc.Height () );

	if ( C3DGfx::GetInstance () && C3DGfx::GetInstance ()->IsInitialized () ) {
		if ( cx && cy ) {
			C3DGfx::GetInstance ()->Resize ( cx, cy ) ;
			m_Camera.SetAspect ( (float)cx/cy ) ;

// 			ImGui::GetIO ().DisplaySize.x = (float)cx ;
// 			ImGui::GetIO ().DisplaySize.y = (float)cy ;

		}
		if ( ! m_pMesh  )
			D3DXCreateTeapot ( C3DGfx::GetInstance ()->GetDevice (), &m_pMesh, NULL ) ;
	}

}

void CMy3DModelViewerView::UpdateObjectMatrix ()
{
	matrix matRotYaw ;
	D3DXMatrixRotationY ( &matRotYaw, m_fYaw + g_fYaw ) ;

	matrix matRotPitch ;
	D3DXMatrixRotationX ( &matRotPitch, m_fPitch ) ;

	matrix matTrans ;
	D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;

	m_matWorld = matRotYaw * matRotPitch * matTrans ;

	((CMainFrame*)AfxGetMainWnd())->m_pShader->SetMatrix ( "g_matWorld", &m_matWorld ) ;
}


LRESULT CMy3DModelViewerView::WindowProc ( UINT message, WPARAM wParam, LPARAM lParam )
{
	// TODO: Add your specialized code here and/or call the base class
	//CGuiRenderer::WndProc ( GetSafeHwnd (), message, wParam, lParam ) ;

	return CView::WindowProc ( message, wParam, lParam );
}
