
// 3DModelViewerView.cpp : implementation of the CMy3DModelViewerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "3DModelViewer.h"
#endif

#include "3DModelViewerDoc.h"
#include "3DModelViewerView.h"
#include "miniz/miniz_zip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
END_MESSAGE_MAP()

// CMy3DModelViewerView construction/destruction

CMy3DModelViewerView::CMy3DModelViewerView()
{
	m_bDragging = FALSE;
	m_pBitmap = NULL;
	m_hwndCallback = NULL;
}

CMy3DModelViewerView::~CMy3DModelViewerView()
{
}

BOOL CMy3DModelViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMy3DModelViewerView drawing

void CMy3DModelViewerView::OnDraw(CDC* pDC)
{
	CMy3DModelViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here

	m_Gfx.BeginFrame();
	m_Gfx.Clear ( 0xff200040 ) ;

	m_Camera.SetD3DCamera ( m_Gfx.GetDevice() ) ;

	m_Gfx.GetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	m_pMesh->DrawSubset(0);

	m_Gfx.EndFrame();
	m_Gfx.ShowFrame();

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
	// 	return ;


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
			float fYaw = m_Camera.GetYaw();
			fYaw += fDeltaX / 100.0f;

			float fPitch = m_Camera.GetPitch();
			fPitch += fDeltaY / 100.0f;

			m_Camera.SetYaw(fYaw);
			m_Camera.SetPitch(fPitch);

			RedrawWindow();
		}
		else if (imd.ButtonStatus.bRButton) {
			vector3 vDist = m_Camera.GetTarget() - m_Camera.GetPosition();
			float fDist = D3DXVec3Length(&vDist);

			fDist += fDeltaY/10.0f;

			m_Camera.SetDistance(fDist);

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
	CView::OnInitialUpdate();

	CRect rc;
	GetClientRect(rc);

	m_Gfx.Initialize(GetSafeHwnd(), rc.Width(), rc.Height(), D3DFMT_A8R8G8B8, FALSE, 0, TRUE);
	
	m_Camera.Initialize(D3DXToRadian(45.0f), 4.0f/3.0f, 0.01f, 100.0f);
	m_Camera.SetMode(CCamera::MODE_TARGET);
	m_Camera.SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera.SetTarget(0.0f, 0.0f, 0.0f);

	//SetRange(-1.0f, 1.0f, 1.0f, -1.0f);
	SetRange(0,0,(float)rc.Width(),(float)rc.Height());

	D3DXCreateTeapot ( m_Gfx.GetDevice(), &m_pMesh, NULL ) ;
}

bool CMy3DModelViewerView::LoadModelFromMemory ( void* pData, DWORD dwDataSize )
{
/*	if ( ! pData )
		return false;

	mz_zip_archive archive;

	ZeroMemory ( &archive, sizeof ( archive ) );

	if ( !mz_zip_reader_init_mem ( &archive, pData, dwDataSize, 0 ) )
		return false;

	{
		size_t iDecompressed = 0;
		void* pDecompressedData = mz_zip_reader_extract_file_to_heap ( &archive, "header.json", &iDecompressed, 0 );

		// TODO: Find a better way if any
		char* pszJsonText = new char [ iDecompressed + 1 ];
		assert ( pszJsonText != NULL );
		memcpy_s ( pszJsonText, iDecompressed, pDecompressedData, iDecompressed );
		pszJsonText [ iDecompressed ] = 0;
		delete pDecompressedData;

		LoadobjectFromJson ( pszJsonText, pObjectSlot );
		delete pszJsonText;
	}

	mz_zip_reader_end ( &archive );

	// 		char szSystemTempPath [ MAX_PATH ] ;
	// 		GetTempPathA ( MAX_PATH, szSystemTempPath ) ;
	// 		strcpy_s ( szSystemTempPath, MAX_PATH, ".\\Temp\\" ) ;

	char szSystemTempPath [ MAX_PATH ];
	if ( 0 == GetTempPathA ( MAX_PATH, szSystemTempPath ) ) {
		strcpy_s ( szSystemTempPath, MAX_PATH, "." );
	}

	char szTempName [ MAX_PATH ];
	if ( 0 == GetTempFileNameA ( NULL, NULL, 0, szTempName ) ) {
		strcpy_s ( szTempName, MAX_PATH, "\\tmp" );
	}

	char szPath [ MAX_PATH ];
	strcpy_s ( szPath, MAX_PATH, szSystemTempPath );
	strcat_s ( szPath, MAX_PATH, szTempName );

	BOOL bResult = CreateDirectoryA ( szPath, NULL );

	strcat_s ( szPath, MAX_PATH, "\\" );

	if ( !bResult ) {
		strcpy_s ( szPath, MAX_PATH, ".\\tmp\\" );
	}

	ZeroMemory ( &archive, sizeof ( archive ) );

	if ( !mz_zip_reader_init_mem ( &archive, pData, dwDataSize, 0 ) )
		return false;

	int iFileCount = mz_zip_reader_get_num_files ( &archive );

	for ( int iFile = 0; iFile < iFileCount; iFile++ ) {

		char szSrcFile [ MAX_PATH ];
		mz_zip_reader_get_filename ( &archive, iFile, szSrcFile, MAX_PATH );

		char szDestFile [ MAX_PATH ];
		strcpy_s ( szDestFile, MAX_PATH, szPath );
		strcat_s ( szDestFile, MAX_PATH, szSrcFile );

		mz_zip_reader_extract_file_to_file ( &archive, szSrcFile, szDestFile, 0 );
	}

	mz_zip_reader_end ( &archive );

	char szMeshFilename [ MAX_PATH ];
	strcpy_s ( szMeshFilename, MAX_PATH, szPath );
	strcat_s ( szMeshFilename, MAX_PATH, pObjectSlot->szMeshFilename );

	bool bRes = CXFileUtil::LoadXFile ( C3DGfx::Instance ()->GetDevice (),
		szMeshFilename,
		szPath,
		"Data/Shader/",
		&pObjectSlot->pMesh );

	{ // Clean temp files
		char szFind [ MAX_PATH ];
		strcpy_s ( szFind, MAX_PATH, szPath );
		strcat_s ( szFind, "*.*" );

		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA ( szFind, &fd );
		while ( FindNextFileA ( hFind, &fd ) ) {
			char szFile [ MAX_PATH ];
			strcpy_s ( szFile, MAX_PATH, szPath );
			strcat_s ( szFile, fd.cFileName );

			BOOL bb = DeleteFileA ( szFile );
			bb = 0;
		}
		FindClose ( hFind );

		RemoveDirectoryA ( szPath );
	}*/

	return true;
}
