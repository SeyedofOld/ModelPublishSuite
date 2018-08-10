//
// ModelViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ModelViewer.h"
#include "ModelViewerDlg.h"
#include "afxdialogex.h"
#include "tlC3DGfx.h"
#include "Imgui/imgui_internal.h"
#include "ObjLoader.h"
#include "C3DScanFile.h"
#include "Obj2Model.h"
#include "CD3DModelUtils.h"
#include "C3DModelUtils.h"
#include "FreeImage.h"
#include "CModelWebServiceClient.h"
#include "GlobalDefines.h"
#include <thread>
#include <functional>
#include "CustomURLProtocolApp.h"

#include <gdiplus.h>


#define		UI_ELEM_THUMBNAIL			0
#define		UI_ELEM_NEXT_OFF			1
#define		UI_ELEM_NEXT_ON				2
#define		UI_ELEM_PREV_OFF			3
#define		UI_ELEM_PREV_ON				4
#define		UI_ELEM_SHARE_OFF			5
#define		UI_ELEM_SHARE_ON			6
#define		UI_ELEM_CLOSE_OFF			7
#define		UI_ELEM_CLOSE_ON			8
#define		UI_ELEM_SHOW_PANE_LEFT_OFF	9
#define		UI_ELEM_SHOW_PANE_LEFT_ON	10
#define		UI_ELEM_SHOW_PANE_RIGHT_OFF	11
#define		UI_ELEM_SHOW_PANE_RIGHT_ON	12
#define		UI_ELEM_HIDE_PANE_LEFT_OFF	13
#define		UI_ELEM_HIDE_PANE_LEFT_ON	14
#define		UI_ELEM_HIDE_PANE_RIGHT_OFF	15
#define		UI_ELEM_HIDE_PANE_RIGHT_ON	16
#define		UI_ELEM_HISTORY				17
#define		UI_ELEM_COMPARE				18


#define TOOLTIP_DELAY 400


using namespace Gdiplus ;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CModelViewerDlg* g_pDlg ;
extern wchar_t g_szExeName [ MAX_PATH ] ;

using namespace std ;

#define SIDE_PANE_WIDTH		(280.0f)

#ifndef SAFE_DELETE
/// For pointers allocated with new.
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
/// For arrays allocated with new [].
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
/// For use with COM pointers.
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif

// CModelViewerDlg dialog

void CopyToClipboard ( HWND hwnd, const std::string &s ) {
	OpenClipboard ( hwnd );
	EmptyClipboard ();
	HGLOBAL hg = GlobalAlloc ( GMEM_MOVEABLE, s.size () + 1 );
	if ( !hg ) {
		CloseClipboard ();
		return;
	}
	memcpy ( GlobalLock ( hg ), s.c_str (), s.size () + 1 );
	GlobalUnlock ( hg );
	SetClipboardData ( CF_TEXT, hg );
	CloseClipboard ();
	GlobalFree ( hg );
}

CModelViewerDlg::CModelViewerDlg(CWnd* pParent /*=NULL*/)
	: CRenderDialog(IDD_MODELVIEWER_DIALOG, pParent) 
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bFileOpened = false ;
	m_bHasFilename = false ;
	m_pd3dModel1 = NULL ;
	m_pModel1 = NULL ;
	m_pd3dModel2 = NULL ;
	m_pModel2 = NULL ;

	D3DXMatrixIdentity ( &m_matWorld1 ) ;
	D3DXMatrixIdentity ( &m_matWorld2 ) ;
	m_fYaw = 0.0f ;
	m_fPitch = 0.0f ;
	m_ptPos1 = vector3 ( 0, 0, 0 ) ;

	m_clrClear = float4_rgba{ 109.0f/255.0f, 169.0f/255.0f, 223.0f/255.0f, 0.0f } ;
	m_clrLight = float4_rgba { 1.0f, 1.0f, 1.0f, 1.0f } ;

	m_ppszTextureNames = NULL ;
	m_iTextureCount = 0 ;

	m_pAdTex = NULL ;

	m_pThumbTex = NULL ;


	m_ppThumbnails = NULL ;
	m_iTextureCount = 0 ;
	m_pstrModelFiles = NULL ;
	m_pstrAdFiles = NULL ;
	m_iThumbCount = 0 ;

	m_bInitialized = false ;
	m_bShowRecentLeft = false ;
	m_bShowRecentRight = false ;
	m_bShowToolbar = false ;
	m_bDualView = false ;
	m_bDownloading = false ;

	m_PointerPass [ 0 ].pData = NULL ;
	m_PointerPass [ 1 ].pData = NULL ;
	m_PointerPass [ 2 ].pData = NULL ;


	m_pCheckerVB = NULL ;
	m_pCheckerIB = NULL ;
// 	CustomURLProtocol prot ;
// 	prot.setProtocolName ( L"3dscan2" ) ;
// 	prot.setAppPath ( L"ModelViewer2.exe" ) ;
// 	prot.setCompanyName ( L"Seyedof" ) ;
// 	prot.CreateCustomProtocol () ;

	m_iCurPart1 = 0 ;
	m_iCurPart2 = 0 ;

	m_bShowSize = false ;

	ZeroMemory ( m_ptexUiElements, MAX_UI_ELEMENTS * sizeof ( IDirect3DTexture9* ) ) ;
}

void CModelViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CModelViewerDlg, CRenderDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED ( IDCANCEL, &CModelViewerDlg::OnBnClickedCancel )
	ON_BN_CLICKED ( IDOK, &CModelViewerDlg::OnBnClickedOk )
	ON_WM_SIZE ()
	ON_WM_ERASEBKGND ()
END_MESSAGE_MAP()


// CModelViewerDlg message handlers

BOOL CModelViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	g_pDlg = this ;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup ( &gdiplusToken, &gdiplusStartupInput, NULL );

	//GdiplusShutdown ( gdiplusToken );

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rc ;
	GetClientRect ( rc ) ;

	C3DGfx::CreateInstance () ;
	C3DGfx::GetInstance ()->Initialize ( GetSafeHwnd (), rc.Width(), rc.Height(), D3DFMT_A8R8G8B8, FALSE, 0, FALSE );

	m_Camera.Initialize ( D3DXToRadian ( 45.0f ), 4.0f / 3.0f, 0.1f, 10090.0f );
	m_Camera.SetMode ( CCamera::MODE_TARGET );
	m_Camera.SetPosition ( 0.0f, 0.0f, -10.0f );
	m_Camera.SetTarget ( 0.0f, 0.0f, 0.0f );

	CMyEffectInclude EffectInclude ;

	HRESULT hr = D3DXCreateEffectFromFileA ( C3DGfx::GetInstance ()->GetDevice (),
		"UberShader.fx",
		NULL,
		&EffectInclude,
		0,
		C3DGfx::GetInstance ()->GetEffectPool (),
		&m_pShader,
		NULL ) ;

	D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "thumb.png", &m_pThumbTex ) ;
	D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "compare.png", &m_pComareTex ) ;

	{ // Load gui textures
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_next_off.png", &m_ptexUiElements [ UI_ELEM_NEXT_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_next_on.png", &m_ptexUiElements [ UI_ELEM_NEXT_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_prev_off.png", &m_ptexUiElements[UI_ELEM_PREV_OFF] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_prev_on.png", &m_ptexUiElements [ UI_ELEM_PREV_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_share_off.png", &m_ptexUiElements [ UI_ELEM_SHARE_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_share_on.png", &m_ptexUiElements [ UI_ELEM_SHARE_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_close_off.png", &m_ptexUiElements [ UI_ELEM_CLOSE_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_close_on.png", &m_ptexUiElements [ UI_ELEM_CLOSE_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_show_left_off.png", &m_ptexUiElements [ UI_ELEM_SHOW_PANE_LEFT_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_show_left_on.png", &m_ptexUiElements [ UI_ELEM_SHOW_PANE_LEFT_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_show_right_off.png", &m_ptexUiElements [ UI_ELEM_SHOW_PANE_RIGHT_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_show_right_on.png", &m_ptexUiElements [ UI_ELEM_SHOW_PANE_RIGHT_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_hide_right_off.png", &m_ptexUiElements [ UI_ELEM_HIDE_PANE_LEFT_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_hide_right_on.png", &m_ptexUiElements [ UI_ELEM_HIDE_PANE_LEFT_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_hide_right_off.png", &m_ptexUiElements [ UI_ELEM_HIDE_PANE_RIGHT_OFF ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_hide_right_on.png", &m_ptexUiElements [ UI_ELEM_HIDE_PANE_RIGHT_ON ] ) ;

		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_history.png", &m_ptexUiElements [ UI_ELEM_HISTORY ] ) ;
		D3DXCreateTextureFromFile ( C3DGfx::GetInstance ()->GetDevice (), "gui_compare.png", &m_ptexUiElements [ UI_ELEM_COMPARE ] ) ;
	}

	{
		m_avCube [ 0 ] = vector3 ( 0.0f, 0.0f, 0.0f ) ;
		m_avCube [ 1 ] = vector3 ( 0.0f, 1.0f, 0.0f ) ;
		m_avCube [ 2 ] = vector3 ( 1.0f, 1.0f, 0.0f ) ;
		m_avCube [ 3 ] = vector3 ( 1.0f, 0.0f, 0.0f ) ;
		m_avCube [ 4 ] = vector3 ( 0.0f, 0.0f, 1.0f ) ;
		m_avCube [ 5 ] = vector3 ( 0.0f, 1.0f, 1.0f ) ;
		m_avCube [ 6 ] = vector3 ( 1.0f, 1.0f, 1.0f ) ;
		m_avCube [ 7 ] = vector3 ( 1.0f, 0.0f, 1.0f ) ;

		for ( int i = 0 ; i < 8 ; i++ )	{ 
			m_avCube [ i ].x -= 0.5f ;
			m_avCube [ i ].y -= 0.5f ;
			m_avCube [ i ].z -= 0.5f ;
		}
	}

	CGuiRenderer::Initialize ( C3DGfx::GetInstance()->GetDevice(), rc.Width(), rc.Height() ) ;

	//m_SettingsGui.Initialize() ;
	//CGuiRenderer::Update ( 0.01f ) ;
	{
		D3DVIEWPORT9 vp ;
		vp.Width = rc.Width () ;
		vp.Height = rc.Height () ;
		vp.MinZ = 0.0f ;
		vp.MaxZ = 1.0f ;
		vp.X = 0 ;
		vp.Y = 0 ;
		m_pView = new C3DViewContext ( L"View1",
			GetSafeHwnd (),
			vp,
			NULL,
			&m_Camera,
			false,
			false,
			true ) ;
	}

	{
		D3DVIEWPORT9 vp ;
		vp.Width = 256 ;
		vp.Height = 256 ;
		vp.MinZ = 0.0f ;
		vp.MaxZ = 1.0f ;
		vp.X = 0 ;
		vp.Y = 0 ;
		m_pThumbView = new C3DViewContext ( L"ViewThumb",
			GetSafeHwnd (),
			vp,
			NULL,
			new CCamera,
			true,
			true,
			true ) ;

		m_pThumbView->CreateRenderTarget ( 256, 256 ) ;

		m_pThumbView->GetCamera()->Initialize ( D3DXToRadian ( 60.0f ), 1.0f / 1.0f, 0.1f, 10000.0f );
		m_pThumbView->GetCamera()->SetMode ( CCamera::MODE_TARGET );
		m_pThumbView->GetCamera()->SetPosition ( 0.0f, 0.0f, -10.0f );
		m_pThumbView->GetCamera()->SetTarget ( 0.0f, 0.0f, 0.0f );
	}

	//SetWindowPos ( NULL, 0, 0, 1200, 675, SWP_NOMOVE ) ;

	ModifyStyleEx ( 0, SS_NOTIFY ) ;

	CModelServiceWebClient::m_hCallbackWnd = GetSafeHwnd() ;

	{ // Set current directory to where the exe exists
		wchar_t szDrive [ MAX_PATH ], szDir [ MAX_PATH ]  ;
		_wsplitpath ( g_szExeName, szDrive, szDir, NULL, NULL ) ;

		wchar_t szFolder [ MAX_PATH ] ;
		wcscpy ( szFolder, szDrive ) ;
		wcscat ( szFolder, szDir ) ;

		SetCurrentDirectoryW ( szFolder ) ;
		//wcscat ( szFolder, L"Cache\\" ) ;

		m_Cache.SetCacheFolder ( L"StuffsNet\\" ) ;
		m_Cache.Initialize () ;

		SetCurrentDirectoryW ( szFolder ) ;

		//m_Cache.EnableCacheRead ( false ) ;
		FillThumbArray() ;
	}

	//CreateChecker ( 10, 5 ) ;

	m_bInitialized = true ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CModelViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
// 		CPaintDC dc ( this ) ;
// 		dc.SelectObject ( GetStockObject ( NULL_BRUSH ) ) ;
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CModelViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CModelViewerDlg::OnBnClickedCancel ()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnCancel ();
	PostQuitMessage ( 0 ) ;
}


void CModelViewerDlg::OnBnClickedOk ()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnOK ();
}

void CModelViewerDlg::Update()
{
	//m_SettingsGui.Update() ;

	if ( GetAsyncKeyState ( VK_LEFT ) & 0x8000 )
		m_fYaw -= 0.005f ;
	if ( GetAsyncKeyState ( VK_RIGHT ) & 0x8000 )
		m_fYaw += 0.005f ;

	if ( GetAsyncKeyState ( VK_UP ) & 0x8000 )
		m_fPitch -= 0.005f ;
	if ( GetAsyncKeyState ( VK_DOWN ) & 0x8000 )
		m_fPitch += 0.005f ;

	if ( GetAsyncKeyState ( 'W' ) & 0x8000 )
		m_Camera.SetPosition ( m_Camera.GetPosition () + m_Camera.GetDirection () * 0.01f ) ;
	if ( GetAsyncKeyState ( 'S' ) & 0x8000 )
		m_Camera.SetPosition ( m_Camera.GetPosition () - m_Camera.GetDirection () * 0.01f ) ;

	UpdateWorldMatrix () ;

	UpdateGui() ;
}

void CModelViewerDlg::ShowExampleMenuFile ()
{
	return ;

	if ( 0 )
	if (ImGui::MenuItem("Import Obj File", "Ctrl+I")) {
		char szFilters[] = "3D Scan Files (*.obj)|*.iobj||";
		CFileDialog dlg(TRUE, "obj", "*.obj", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd());
		if (dlg.DoModal() == IDOK) {
			char szOldPath [ MAX_PATH ] ;
			GetCurrentDirectory ( MAX_PATH, szOldPath ) ;
			SetCurrentDirectory ( dlg.GetFolderPath() ) ;
			MY_OBJ obj ;
			LoadObj2 ( dlg.GetPathName(), &obj ) ;
			m_pModel1 = new TD_SCAN_MODEL ;
			ConvertObjTo3DModel ( obj, *m_pModel1 ) ;
			SetCurrentDirectory ( szOldPath ) ;

			m_pd3dModel1 = new D3D_MODEL ;
			if ( ! CD3DModelUtils::CreateFromTDModel ( C3DGfx::GetInstance ()->GetDevice (), C3DGfx::GetInstance ()->GetEffectPool (), *m_pModel1, *m_pd3dModel1 ) ) {
				delete m_pd3dModel1 ;
				m_pd3dModel1 = NULL ;
				delete m_pModel1 ;
				m_pModel1 = NULL ;
			}
			else {
				FillTextureList () ;
				m_bFileOpened = true ;
				m_bHasFilename = false ;
			}
		}

	}

	if ( ImGui::MenuItem ( "Open 3D Scan File", "Ctrl+O" ) ) {
		char szFilters[] = "3D Scan Files (*.3dscan)|*.3dscan||";
		CFileDialog dlg ( TRUE, "3dscan", "*.3dscan", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
		if (dlg.DoModal() == IDOK) {

			Load3DScanFile ( dlg.GetPathName () ) ;
		}
	}

	if ( ImGui::MenuItem ( "Get 3D Scan From Server", "" ) ) {
		
		CModelServiceWebClient client ;
		bool b = false ;
		char* p = NULL ;
		Load3DScanFromUrl ( ( CString )"http://localhost:5617/3dscan/get?subsid=123dsff&clientid=pcc" ) ;
		//Load3DScanFromUrl ( (CString)"http://51.254.82.69:5617/3dscan/get?subsid=123dsff&clientid=pcc" ) ;
	}

	if ( ImGui::BeginMenu ( "Open Recent" ) ) {
		ImGui::MenuItem ( "fish_hat.c" );
		ImGui::MenuItem ( "fish_hat.inl" );
		ImGui::MenuItem ( "fish_hat.h" );
		ImGui::EndMenu ();
	}
	
	if ( 0 )
	if ( ImGui::MenuItem ( "Save 3D Scan File", "Ctrl+S", false, m_bFileOpened ) ) {
		if ( m_bFileOpened && m_pModel1 ) {
			if ( ! m_bHasFilename ) {
				char szFilters[] = "3D Scan Files (*.3dscan)|*.3dscan||";
				CFileDialog dlg ( FALSE, "3dscan", "*.3dscan", OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
				if ( dlg.DoModal () == IDOK ) {
					CalcTextureAverages() ;
					m_bHasFilename = true ;
					m_strFilename = dlg.GetPathName() ;
					C3DScanFile::Save3DScanModel ( m_strFilename.GetBuffer(), m_pModel1 ) ;
				}
			} 
			else { // No need to ask for filename
				C3DScanFile::Save3DScanModel ( m_strFilename.GetBuffer (), m_pModel1 ) ;
			}
		}
	}
	
	if ( 0 )
	if ( ImGui::MenuItem ( "Save 3D Scan File As...", "", false, m_bFileOpened ) ) {
		if ( m_bFileOpened && m_pModel1 ) {
			char szFilters[] = "3D Scan Files (*.3dscan)|*.3dscan||";
			CFileDialog dlg ( FALSE, "3dscan", "*.3dscan", OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
			if ( dlg.DoModal () == IDOK ) {
				CalcTextureAverages () ;
				m_bHasFilename = true ;
				m_strFilename = dlg.GetPathName () ;
				C3DScanFile::Save3DScanModel ( m_strFilename.GetBuffer (), m_pModel1 ) ;
			}
		}
	}

	ImGui::Separator ();

/*	if ( ImGui::BeginMenu ( "Options" ) ) {
		static bool enabled = true;
		ImGui::MenuItem ( "Enabled", "", &enabled );
		ImGui::BeginChild ( "child", ImVec2 ( 0, 60 ), true );
		for ( int i = 0; i < 10; i++ )
			ImGui::Text ( "Scrolling Text %d", i );
		ImGui::EndChild ();
		static float f = 0.5f;
		static int n = 0;
		static bool b = true;
		ImGui::SliderFloat ( "Value", &f, 0.0f, 1.0f );
		ImGui::InputFloat ( "Input", &f, 0.1f );
		ImGui::Combo ( "Combo", &n, "Yes\0No\0Maybe\0\0" );
		ImGui::Checkbox ( "Check", &b );
		ImGui::EndMenu ();
	}	
	
	if ( ImGui::BeginMenu ( "Colors" ) ) {
		ImGui::PushStyleVar ( ImGuiStyleVar_FramePadding, ImVec2 ( 0, 0 ) );
		for ( int i = 0; i < ImGuiCol_COUNT; i++ )
		{
			//const char* name = ImGui::GetStyleColorName ( (ImGuiCol)i );
			//ImGui::ColorButton ( name, ImGui::GetStyleColorVec4 ( (ImGuiCol)i ) );
			//ImGui::SameLine ();
			//ImGui::MenuItem ( name );
		}
		ImGui::PopStyleVar ();
		ImGui::EndMenu ();
	}

	if ( ImGui::BeginMenu ( "Disabled", false ) ) // Disabled
	{
		IM_ASSERT ( 0 );
	}
	if ( ImGui::MenuItem ( "Checked", NULL, true ) ) {}*/
	if ( ImGui::MenuItem ( "Quit", "Alt+F4" ) ) {
		PostQuitMessage ( 0 ) ;
	}
}

void CModelViewerDlg::Render()
{
	//bool bDualView = false ;
	//bool bSingleView = true ;

	C3DGfx::GetInstance ()->BeginFrame ();

	D3DXCOLOR clrClear ( m_clrClear.r, m_clrClear.g, m_clrClear.b, 0.0f ) ;
	C3DGfx::GetInstance ()->Clear ( clrClear ) ;

	m_pView->SelectView() ;


	//C3DGfx::GetInstance ()->GetDevice ()->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;


	// 		shared float4x4 g_matSunLight : SUNLIGHT ;		// Sun Light characteristics
	// 														// Row1 : Direction
	// 														// Row2 : Diffuse
	// 														// Row3 : Ambient
	// 														// Row4 : Reserved
	

	m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix () ) ;

	vector4 vLightDir ( 0.0f, 0.0f, 1.0f, 0.0f ) ;
	D3DXVec4Normalize ( &vLightDir, &vLightDir ) ;
	vector4 vAmbLight ( 0.5f, 0.5f, 0.5f, 0.0f ) ;

	m_pShader->SetVector ( "g_vSunLightDir", &vLightDir ) ;
	m_pShader->SetVector ( "g_f4SunLightDiffuse", (D3DXVECTOR4*)&m_clrLight ) ;
	m_pShader->SetVector ( "g_f4SunLightAmbient", &vAmbLight ) ;

	D3DVIEWPORT9 vp_orig = m_pView->GetViewport() ;

	if ( m_bDualView ) {
		m_Camera.SetAspect ( 0.5f * (float)vp_orig.Width / vp_orig.Height ) ;
		m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;

		D3DVIEWPORT9 vp = vp_orig ;
		vp.Width /= 2 ;
		vp.X = 0 ;
		pDevice->SetViewport ( &vp ) ;
		if ( m_pd3dModel1 ) {
			m_pShader->SetMatrix ( "g_matWorld", &m_matWorld1 ) ;
			CD3DModelUtils::RenderD3DModel ( pDevice, *m_pd3dModel1 ) ;

			m_pShader->SetMatrix ( "g_matWorld", &m_matChecker1 ) ;
			DrawGrid () ;
			if ( m_bShowSize )
				DrawBounding ( m_pd3dModel1, m_matWorld1 ) ;
		}


		C3DGfx::GetInstance ()->GetD3DXLine ()->Begin () ;
		vector2 points [ 2 ] = { { (float)vp.Width - 1.0f, 0.0f }, { (float)vp.Width - 1.0f, (float)vp.Height} } ;
		C3DGfx::GetInstance ()->GetD3DXLine ()->Draw ( points, 2, 0x80ffffff ) ;
		C3DGfx::GetInstance ()->GetD3DXLine ()->End () ;

 		vp = vp_orig ;
 		vp.Width /= 2 ;
 		vp.X = vp.Width ;
 		pDevice->SetViewport ( &vp ) ;


		if ( m_pd3dModel2 ) {
			m_pShader->SetMatrix ( "g_matWorld", &m_matWorld2 ) ;
			CD3DModelUtils::RenderD3DModel ( pDevice, *m_pd3dModel2 ) ;

			m_pShader->SetMatrix ( "g_matWorld", &m_matChecker2 ) ;
			DrawGrid () ;
			if ( m_bShowSize )
				DrawBounding ( m_pd3dModel2, m_matWorld2 ) ;
		}
		C3DGfx::GetInstance ()->GetD3DXLine ()->Begin () ;
		vector2 points2 [ 2 ] = { { 0.0f, 0.0f }, { 0.0f, (float)vp.Height} } ;
		C3DGfx::GetInstance ()->GetD3DXLine ()->Draw ( points2, 2, 0x80ffffff ) ;
		C3DGfx::GetInstance ()->GetD3DXLine ()->End () ;

		pDevice->SetViewport ( &vp_orig ) ;
		m_Camera.SetAspect ( 1.0f * (float)vp_orig.Width / vp_orig.Height ) ;
		m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;
	}
	else {
		m_Camera.SetAspect ( 1.0f * (float)vp_orig.Width / vp_orig.Height ) ;
		m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;

		pDevice->SetViewport ( &vp_orig ) ;

		if ( m_pd3dModel1 ) {
			m_pShader->SetMatrix ( "g_matWorld", &m_matWorld1 ) ;
			CD3DModelUtils::RenderD3DModel ( pDevice, *m_pd3dModel1 ) ;

			m_pShader->SetMatrix ( "g_matWorld", &m_matChecker1 ) ;
			DrawGrid () ;
			if ( m_bShowSize )
				DrawBounding ( m_pd3dModel1, m_matWorld1 ) ;
		}

	}

	if ( m_pAdTex && ! m_bDualView ) {
		CRect rc ;
		GetClientRect ( rc ) ;
		D3DSURFACE_DESC desc ;
		m_pAdTex->GetLevelDesc ( 0, &desc ) ;

		matrix matScale ;
		D3DXMatrixScaling ( &matScale, (float)m_iAdWidth1 / desc.Width, (float)m_iAdHeight1 / desc.Height, 1.0f ) ;
		matrix matTrans ;
		D3DXMatrixTranslation ( &matTrans, 0.0f, (float)rc.Height() - m_iAdHeight1, 0.0f ) ;

		matrix matResult = matScale * matTrans ;
		C3DGfx::GetInstance ()->GetD3DXSprite ()->SetTransform ( &matResult ) ;

		C3DGfx::GetInstance ()->GetD3DXSprite ()->Begin ( D3DXSPRITE_ALPHABLEND ) ;
		C3DGfx::GetInstance ()->GetD3DXSprite ()->Draw ( m_pAdTex, NULL, NULL, /*&D3DXVECTOR3 (0, (float)rc.Height() - desc.Height, 0)*/NULL, 0xffffffff ) ;
		C3DGfx::GetInstance ()->GetD3DXSprite ()->End () ;
	}

	ImGui::Render () ;

	C3DGfx::GetInstance ()->EndFrame ();
	m_pView->ShowFrame( ) ;
}


void CModelViewerDlg::OnSize ( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize ( nType, cx, cy );

	// TODO: Add your message handler code here
	SetRange ( 0.0f, 0.0f, D3DX_PI*2.0f, D3DX_PI ) ;

	if ( C3DGfx::GetInstance () && C3DGfx::GetInstance ()->IsInitialized () ) {
		if ( cx && cy ) {
			m_pView->CreateRenderTarget ( cx, cy, false ) ;
			D3DVIEWPORT9 vp = m_pView->GetViewport () ;
			vp.Width = cx ;
			vp.Height = cy ;
			m_pView->SetViewport ( vp ) ;

			m_pView->GetCamera ()->SetAspect ( (float)cx / cy ) ;

			ImGui::GetIO ().DisplaySize.x = (float)cx - 1 ;
			ImGui::GetIO ().DisplaySize.y = (float)cy - 1 ;
		}
	}
}

using namespace ImGui ;

//extern struct ImGuiState*             GImGui ;

LRESULT ImGui_ImplWin32_WndProcHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

LRESULT CModelViewerDlg::WindowProc ( UINT message, WPARAM wParam, LPARAM lParam )
{
	// TODO: Add your specialized code here and/or call the base class
	//if ( CGuiRenderer::s_pDevice )
		//CGuiRenderer::WndProc ( GetSafeHwnd(), message, wParam, lParam ) ;

	ImGui_ImplWin32_WndProcHandler ( GetSafeHwnd (), message, wParam, lParam ) ;

	if ( message == WM_USER_HTTP_PROGRESS ) {
		if ( m_iFileSize != 0 ) {
			m_fDownloadProgress = (float)lParam / m_iFileSize * 2.0f;
			if ( m_fDownloadProgress > 1.0f )
				m_fDownloadProgress = 1.0f ;
		}
		else
			m_fDownloadProgress = 0.0f ;
// 		CString s ;
// 		s.Format ( "%d\n", lParam ) ;
// 		OutputDebugString ( s ) ;
	} 
	else if ( message == WM_USER_MODEL_DOWNLOADED ) {
		TD_SCAN_MODEL* pModel = C3DScanFile::Load3DScanModelFromMemory ( m_PointerPass[1].pData, m_PointerPass[1].iSize ) ;
		if ( pModel ) {
			D3D_MODEL* pd3dModel = new D3D_MODEL ;
			if ( ! CD3DModelUtils::CreateFromTDModel ( C3DGfx::GetInstance ()->GetDevice (), C3DGfx::GetInstance ()->GetEffectPool (), *pModel, *pd3dModel ) ) {
				SAFE_DELETE ( pd3dModel ) ;
				C3DModelUtils::FreeModel ( *pModel ) ;
				SAFE_DELETE ( pModel ) ;
			}
			else {
				if ( m_pd3dModel1 ) {
					CD3DModelUtils::FreeD3DModel ( *m_pd3dModel1 ) ;
					SAFE_DELETE ( m_pd3dModel1 ) ;
				}
				if ( m_pModel1 ) {
					C3DModelUtils::FreeModel ( *m_pModel1 ) ;
					SAFE_DELETE ( m_pModel1 ) ;
				}
				m_pModel1 = pModel ;
				m_pd3dModel1 = pd3dModel ;
				
				OnModelLoaded1 () ;

				ResetView () ;

				FillTextureList () ;
				m_bFileOpened = true ;
				m_bHasFilename = false ;

				if ( lParam == 0 ) {
					void* pThumbData = NULL ;
					int iThumbSize = 0 ;
					GenerateThumnail ( &pThumbData, &iThumbSize ) ;

					if ( m_Cache.AddModelToCache ( m_strSubsid, m_PointerPass [ 1 ].pData, m_PointerPass [ 1 ].iSize, NULL, 0, m_strAdUrl, pThumbData, iThumbSize ) ) {
					}
					FillThumbArray () ;
				}
			}
		}


		SAFE_DELETE ( m_PointerPass [ 1 ].pData ) ;
		m_bDownloading = false ;
	} 
	else if ( message == WM_USER_AD_DOWNLOADED ) {
		D3DXCreateTextureFromFileInMemoryEx ( C3DGfx::GetInstance ()->GetDevice (), 
			m_PointerPass[2].pData, 
			m_PointerPass[2].iSize, 
			0,
			0,
			1,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_MANAGED,
			D3DX_DEFAULT,
			D3DX_DEFAULT,
			0,
			NULL,
			NULL,
			&m_pAdTex ) ;

		IStream *pStream = SHCreateMemStream ( (BYTE*)m_PointerPass[2].pData, m_PointerPass[2].iSize ) ;
		// Do what you want
		Gdiplus::Image* img = Gdiplus::Image::FromStream ( pStream, false );
		if ( img ) {
			m_iAdWidth1		= img->GetWidth () ;
			m_iAdHeight1	= img->GetHeight () ;
			delete img ;
		}
		pStream->Release ();

		if ( lParam == 0 ) {
			m_Cache.AddAdToCache ( m_strSubsid, m_PointerPass [ 2 ].pData, m_PointerPass [ 2 ].iSize, m_strAdUrl ) ;
		}

		SAFE_DELETE ( m_PointerPass [ 2 ].pData ) ;
	}
	else if ( message == WM_USER_MODEL_INFO ) {
		m_iFileSize = lParam ;
		SAFE_DELETE ( m_PointerPass [ 0 ].pData ) ;

// 		DownloadModel ( m_strModel ) ;
// 		DownloadAd ( m_strAd ) ;
	}

	// Handle mouse wheel for zooming
	if ( message == WM_MOUSEWHEEL && !ImGui::IsAnyWindowHovered () ) {
		float fDelta = (float) GET_WHEEL_DELTA_WPARAM ( wParam ) / (float)WHEEL_DELTA ;

		float fSpeed = 1.0f ;
		if ( m_pd3dModel1 ) {
			vector3 vSize (	m_pd3dModel1->ptMax.x - m_pd3dModel1->ptMin.x,
							m_pd3dModel1->ptMax.y - m_pd3dModel1->ptMin.y,
							m_pd3dModel1->ptMax.z - m_pd3dModel1->ptMin.z ) ;
			float fSize = D3DXVec3Length ( &vSize ) ;
			fSpeed = fSize / 10.0f ;
		}

		m_Camera.SetPosition ( m_Camera.GetPosition () + m_Camera.GetDirection () * fDelta / 5.0f * fSpeed ) ;
	}


	if ( message == WUM_INTERACTION_MSG ) {
		INTERACTION_MSG_DATA& imd = *((INTERACTION_MSG_DATA*)lParam) ;

		static VECTOR2 s_ptStart ( 0.0f, 0.0f ) ;

		if ( imd.eEvent == MOUSE_DRAG_START ) {
			s_ptStart = imd.ptCursor ;
		}
		else if ( imd.eEvent == MOUSE_DRAGGING && ! ImGui::IsAnyWindowHovered() ) {
			float fDeltaX = imd.ptCursor.x - s_ptStart.x ;
			float fDeltaY = imd.ptCursor.y - s_ptStart.y ;

			if ( imd.ButtonStatus.bLButton ) {
				m_fPitch -= fDeltaY ;
				if ( IsNegativePitch() )
					m_fYaw += fDeltaX ;
				else
					m_fYaw -= fDeltaX ;

				UpdateWorldMatrix () ;
			}
			
			if ( imd.ButtonStatus.bRButton ) {
				float fDelta = - fDeltaY * 2.0f ;
				m_Camera.SetPosition ( m_Camera.GetPosition () + m_Camera.GetDirection () * fDelta ) ;
			}

			if ( imd.ButtonStatus.bMButton ) {
				D3DXMATRIX matView = m_Camera.GetViewMatrix() ;
				float fScale = 1.0f ;
				if ( m_pd3dModel1 ) {
					float fSize = D3DXVec3Length ( &m_vModelSize1 ) ;
					fScale *= fSize / 10.0f ;
				}
				D3DXVECTOR3 vUp = D3DXVECTOR3 ( matView._21, matView._22, matView._23 ) ;
				D3DXVECTOR3 vRight = D3DXVECTOR3 ( matView._11, matView._12, matView._13 ) ;

				m_ptPos1 += fDeltaX * vRight * fScale - fDeltaY * vUp * fScale ;

				UpdateWorldMatrix () ;
			}

			s_ptStart = imd.ptCursor ;
		}
		else if ( imd.eEvent == MOUSE_DRAG_END ) {

		}
		else if ( imd.eEvent == MOUSE_LBDOWN ) {
			if ( !ImGui::IsAnyWindowHovered () ) {
				if ( m_pModel1 ) {
					CPoint ptScreen = RenderPortToScreenPixel ( imd.ptCursor ) ;
					D3DXVECTOR3 vDir, ptPos ;

					CRect rc ;
					GetClientRect ( rc ) ;

					m_Camera.GetRayFromScreen ( ptScreen, &vDir, (float)rc.Width (), (float)rc.Height () ) ;

					D3DXMATRIX matInvWorld ;
					D3DXMatrixInverse ( &matInvWorld, NULL, &m_matWorld1 ) ;

					D3DXVec3TransformNormal ( &vDir, &vDir, &matInvWorld ) ;

					ptPos = m_Camera.GetPosition () ;
					D3DXVec3TransformCoord ( &ptPos, &ptPos, &matInvWorld ) ;

					float3 ptHit ;
					D3DMODEL_SUBSET* pSubset = NULL ;
					if ( CD3DModelUtils::IntersectRay ( float3{ ptPos.x, ptPos.y, ptPos.z }, float3{ vDir.x, vDir.y, vDir.z }, *m_pd3dModel1, &ptHit, &pSubset ) ) {
						//pSubset->bSelected = ! pSubset->bSelected ;
					}
				}

				if ( m_pAdTex ) {
					CRect rc ;
					GetClientRect ( rc ) ;

					CPoint ptScreen = RenderPortToScreenPixel ( imd.ptCursor ) ;

					if ( ptScreen.x >= 0 && ptScreen.x < m_iAdWidth1 && ptScreen.y >= rc.Height () - m_iAdHeight1 && ptScreen.y < rc.Height () ) {
						HINSTANCE h = ShellExecuteW ( NULL, L"open", m_strAdUrl.c_str(), NULL, NULL, SW_SHOWNORMAL ) ;
					}
				}

			}
		}


	}

	return CRenderDialog::WindowProc ( message, wParam, lParam );
}


BOOL CModelViewerDlg::OnEraseBkgnd ( CDC* pDC )
{
	// TODO: Add your message handler code here and/or call default
	//return TRUE ;
	pDC->SelectObject ( GetStockObject ( NULL_BRUSH ) ) ;
	return TRUE ;

	return CDialogEx::OnEraseBkgnd ( pDC );
}

void CModelViewerDlg::UpdateGui ()
{
	static float s_fPrevTime = 0.0f ;
	float fCurTime = (float)GetTickCount () / 1000.0f ;
	float fDeltaTime = fCurTime - s_fPrevTime ;
	ImGui::GetIO ().DeltaTime = fDeltaTime ;
	if ( fDeltaTime == 0.0f )
		return ;

	s_fPrevTime = fCurTime ;

	ImGui::NewFrame() ;

	CRect rc ;
	GetClientRect ( rc ) ;

	if ( 0 )
	if ( ImGui::BeginMainMenuBar () ) {
		if ( ImGui::BeginMenu ( "File" ) ) {
			ShowExampleMenuFile ();
			ImGui::EndMenu ();
		}

		if ( ImGui::BeginMenu ( "Edit" ) ) {
			if ( ImGui::MenuItem ( "Undo", "CTRL+Z" ) ) {}
			if ( ImGui::MenuItem ( "Redo", "CTRL+Y", false, false ) ) {}  // Disabled item
			ImGui::Separator ();
			if ( ImGui::MenuItem ( "Cut", "CTRL+X" ) ) {}
			if ( ImGui::MenuItem ( "Copy", "CTRL+C" ) ) {}
			if ( ImGui::MenuItem ( "Paste", "CTRL+V" ) ) {}
			ImGui::EndMenu ();
		}

		if ( ImGui::BeginMenu ( "Options" ) ) {

			ImGui::ColorEdit3 ( "Background Color", (float*)&m_clrClear ) ;
			ImGui::ColorEdit3 ( "Light Color", (float*)&m_clrLight ) ;

			ImGui::EndMenu ();
		}

		ImGui::EndMainMenuBar ();
	}

	ImGuiWindowFlags flags = 0;
	//flags |= ImGuiWindowFlags_NoMove;
	//flags |= ImGuiWindowFlags_NoResize;
	//flags |= ImGuiWindowFlags_NoCollapse;
	//flags |= ImGuiWindowFlags_MenuBar;
	//flags |= ImGuiWindowFlags_ShowBorders;
	//flags |= ImGuiWindowFlags_NoTitleBar;

	static bool opn = false ;

	//SetNextWindowSize ( ImVec2 ( 300, 400 ), ImGuiCond_FirstUseEver );
	//static bool s_b = false ;
	//if ( ImGui::Begin ( "Parts", NULL, ImVec2(200,100)) ) {
	//	
	//	if ( m_pd3dModel1 )
	//	for ( uint32_t iPart = 0 ; iPart < m_pd3dModel1->Parts.size () ; iPart++ ) {

	//		D3D_MODEL& model = *m_pd3dModel1 ;

	//		D3DMODEL_PART& part = model.Parts [ iPart ] ;
	//		ImGui::Checkbox( part.pBase->sName.c_str(), &part.bVisible );
	//	}

	//	ImGui::End ();
	//}

	flags = 0;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoInputs;
	flags |= ImGuiWindowFlags_NoTitleBar;

	if ( m_bDownloading ) {
		CRect rc ;
		GetClientRect ( rc ) ;

		ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_Alpha, 0.5f ) ;
		if ( ImGui::Begin ( "Bar", NULL, flags ) ) {
			ImGui::SetWindowSize ( ImVec2 ( (float)rc.Width (), 50 ) ) ;
			ImGui::SetWindowPos ( ImVec2(0, (float)rc.Height () - 35 ) )  ;
			//ImGui::ProgressBar ( (float)( GetTickCount () % 10000 ) / 10000.0f ) ;
			ImGui::ProgressBar ( m_fDownloadProgress ) ;
			ImGui::End ();
		}
		ImGui::PopStyleVar () ;
		ImGui::PopStyleColor () ;
	}

	{ // Left Pane

		flags = 0 ;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		//flags |= ImGuiWindowFlags_NoCollapse;
		//flags |= ImGuiWindowFlags_NoInputs;
		flags |= ImGuiWindowFlags_NoTitleBar;
		if ( m_bShowRecentLeft ) {
			if ( ImGui::Begin ( "RecentLeft", NULL, ImVec2 ( SIDE_PANE_WIDTH, (float)rc.Height () ), -1, flags ) ) {
				ImGui::SetWindowPos ( ImVec2 ( 0, 0 ) ) ;
				ImGui::SetWindowSize ( ImVec2 ( SIDE_PANE_WIDTH, (float)rc.Height () ) ) ;
// 				if ( ImGui::GetIO ().MousePos.x > SIDE_PANE_WIDTH )
// 					m_bShowRecentLeft = false ;

				ImGui::Image ( m_ptexUiElements [ UI_ELEM_HISTORY ], ImVec2(260,40) ) ;

				static int pressed_count = 0;
				for ( int i = 0 ; i < m_iThumbCount ; i++ ) {

					ImTextureID my_tex_id = m_ppThumbnails [ i ] ;
					if ( my_tex_id == NULL )
						my_tex_id = m_pThumbTex ;

					D3DSURFACE_DESC desc ;
					( (IDirect3DTexture9*)my_tex_id )->GetLevelDesc ( 0, &desc ) ;
					float my_tex_w = (float)desc.Width ;
					float my_tex_h = (float)desc.Height ;

					if ( ImGui::ImageButton ( my_tex_id, ImVec2 ( 256, 256 ), ImVec2 ( 0, 0 ), ImVec2 ( 256.0f / my_tex_w, 256.0f / my_tex_h ), 2, ImColor ( 0, 0, 0, 255 ) ) ) {
						Load3DScanFile ( m_pstrModelFiles [ i ] ) ;
						D3DXCreateTextureFromFileW ( C3DGfx::GetInstance ()->GetDevice (), m_pstrAdFiles [ i ].c_str (), &m_pAdTex ) ;
						CModelCache::CACHE_ENTRY e ;
						m_Cache.GetEntry ( m_Cache.GetEntryCount() - 1 - i, e ) ;
						m_strAdUrl = e.szAdUrl ;

						Gdiplus::Image* img = Gdiplus::Image::FromFile ( m_pstrAdFiles [ i ].c_str (), false );
						if ( img ) {
							m_iAdWidth1 = img->GetWidth () ;
							m_iAdHeight1 = img->GetHeight () ;
							delete img ;
						}

					}
					pressed_count += 1;
				}

				ImGui::End () ;
			}

			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
			ImGui::SetNextWindowPos ( ImVec2 ( SIDE_PANE_WIDTH, rc.Height () / 2.0f ) ) ;
			if ( ImGui::Begin ( "hl", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				static bool s_b = false ;

				ImTextureID texId = m_ptexUiElements [ UI_ELEM_HIDE_PANE_LEFT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_HIDE_PANE_LEFT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImColor ( 0, 0, 0, 0 ) ) ) {
					m_bShowRecentLeft = false ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				ImGui::End () ;
			}
			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;

		}
		else {
			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
			ImGui::SetNextWindowPos ( ImVec2 ( 0.0f, rc.Height () / 2.0f ) ) ;
			if ( ImGui::Begin ( "sl", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;
				
				static bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_SHOW_PANE_LEFT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_SHOW_PANE_LEFT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_bShowRecentLeft = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					ImGui::TextUnformatted ( "History" );
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}
			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;

		}
	}

	{ // Right Pane
		if ( m_bShowRecentRight ) {
			if ( ImGui::Begin ( "RecentRight", NULL, ImVec2 ( SIDE_PANE_WIDTH, (float)rc.Height () ), -1, flags ) ) {
				ImGui::SetWindowPos ( ImVec2 ( (float)rc.Width () - SIDE_PANE_WIDTH, 0.0f ) ) ;
				ImGui::SetWindowSize ( ImVec2 ( SIDE_PANE_WIDTH, (float)rc.Height () ) ) ;
// 				if ( ImGui::GetIO ().MousePos.x < rc.Width () - SIDE_PANE_WIDTH )
// 					m_bShowRecentRight = false ;
				
				ImGui::Image ( m_ptexUiElements [ UI_ELEM_COMPARE ], ImVec2 ( 260, 40 ) ) ;

				static int pressed_count = 0;
				for ( int i = 0 ; i < m_iThumbCount ; i++ ) {

					ImTextureID my_tex_id = m_ppThumbnails [ i ] ;
					if ( my_tex_id == NULL )
						my_tex_id = m_pThumbTex ;

					D3DSURFACE_DESC desc ;
					( (IDirect3DTexture9*)my_tex_id )->GetLevelDesc ( 0, &desc ) ;
					float my_tex_w = (float)desc.Width ;
					float my_tex_h = (float)desc.Height ;

					if ( ImGui::ImageButton ( my_tex_id, ImVec2 ( 256, 256 ), ImVec2 ( 0, 0 ), ImVec2 ( 256.0f / my_tex_w, 256.0f / my_tex_h ), 2, ImColor ( 0, 0, 0, 255 ) ) ) {
						Load3DScanFile2 ( m_pstrModelFiles [ i ] ) ;
						m_bDualView = true ;
						m_bShowRecentRight = false ;

						Gdiplus::Image* img = Gdiplus::Image::FromFile ( m_pstrAdFiles [ i ].c_str (), false );
						if ( img ) {
							// 						m_iAdWidth2 = img->GetWidth () ;
							// 						m_iAdHeight2 = img->GetHeight () ;
							delete img ;
						}
					}
					pressed_count += 1;
				}

				ImGui::End () ;
			}

			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () - 30.0f - SIDE_PANE_WIDTH, rc.Height () / 2.0f ) ) ;
			if ( ImGui::Begin ( "hr", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				static bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_HIDE_PANE_RIGHT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_HIDE_PANE_RIGHT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_bShowRecentRight = false ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				ImGui::End () ;
			}
			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;


		}
		else {
			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width ()- 30.0f, rc.Height () / 2.0f ) ) ;
			if ( ImGui::Begin ( "sr", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				static bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_SHOW_PANE_RIGHT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_SHOW_PANE_RIGHT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_bShowRecentRight = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					ImGui::TextUnformatted ( "Compare" );
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}
			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;

		}
	}

	if ( m_bShowToolbar ) {
		if ( ImGui::Begin ( "Toolbar", NULL, ImVec2 ( (float)rc.Width(), 100.0f ), -1, flags ) ) {
			ImGui::SetWindowPos ( ImVec2 ( 0, 0 ) ) ;
			ImGui::SetWindowSize ( ImVec2 ( (float)rc.Width(), 100.0f ) ) ;
			if ( ImGui::GetIO().MousePos.y > 100 )
				m_bShowToolbar = false ;

			static int pressed_count = 0;
			for ( int i = 0 ; i < 1 ; i++ ) {

				ImTextureID my_tex_id = m_pComareTex ;
				D3DSURFACE_DESC desc ;
				( (IDirect3DTexture9*)my_tex_id )->GetLevelDesc ( 0, &desc ) ;
				float my_tex_w = (float)desc.Width ;
				float my_tex_h = (float)desc.Height ;

				ImGui::SameLine () ;
				if ( ImGui::ImageButton ( my_tex_id, ImVec2 ( 64, 64 ), ImVec2 ( 0, 0 ), ImVec2 ( 128.0f / my_tex_w, 128.0f / my_tex_h ), 2, ImColor ( 0, 0, 0, 255 ) ) ) {
					m_bDualView = ! m_bDualView ;
				}
				pressed_count += 1;
			}

			ImGui::End () ;
		}
	}


	// Close button
	if ( m_bDualView || 1 ) {
		flags = 0 ;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		flags |= ImGuiWindowFlags_NoCollapse;
		//flags |= ImGuiWindowFlags_NoInputs;
		flags |= ImGuiWindowFlags_NoTitleBar;
		flags |= ImGuiWindowFlags_NoScrollbar;
		ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
		ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () / 2.0f + 20.0f, 20.0f ) ) ;
		if ( ImGui::Begin ( "c", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
			SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

			static bool s_b = false ;
			ImTextureID texId = m_ptexUiElements [ UI_ELEM_CLOSE_OFF ] ;
			if ( s_b )
				texId = m_ptexUiElements [ UI_ELEM_CLOSE_ON ] ;
			if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
				m_bDualView = false ;
			}
			if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
				s_b = true ;
			else
				s_b = false ;

			static int s_i = GetTickCount () ;
			if ( s_b && (GetTickCount () - s_i) >= TOOLTIP_DELAY ) {
				ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
				ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
				ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
				ImGui::BeginTooltip ();
				//ImGui::PushTextWrapPos ( 450.0f );
				ImGui::TextUnformatted ( "Close" );
				//ImGui::PopTextWrapPos ();
				ImGui::EndTooltip ();
				ImGui::PopStyleColor () ;
				ImGui::PopStyleVar ( 2 ) ;
			}
			else {
				if ( ! s_b )
					s_i = GetTickCount() ;
			}

			ImGui::End () ;
 		}
		ImGui::PopStyleVar(6) ;
		ImGui::PopStyleColor () ;
	}

	{ // Share button

		static bool s_bShowShare = false ;

		flags = 0 ;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		flags |= ImGuiWindowFlags_NoCollapse;
		//flags |= ImGuiWindowFlags_NoInputs;
		flags |= ImGuiWindowFlags_NoTitleBar;
		flags |= ImGuiWindowFlags_NoScrollbar;
		ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;
		ImGui::SetNextWindowPos ( ImVec2 ( 20.0f, /*rc.Height () -*/ 20.0f ) ) ;
		if ( ImGui::Begin ( "s", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
			SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

			static bool s_b = false ;
			ImTextureID texId = m_ptexUiElements [ UI_ELEM_SHARE_OFF ] ;
			if ( s_b )
				texId = m_ptexUiElements [ UI_ELEM_SHARE_ON ] ;
			if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2(0,0), ImVec2(1,1), 0, ImVec4(0,0,0,0) ) ) {
				if ( m_pd3dModel1 )
					s_bShowShare = true ;
			}
			
			if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
				s_b = true ;
			else
				s_b = false ;

			static int s_i = GetTickCount () ;
			if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
				ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
				ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
				ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
				ImGui::BeginTooltip ();
				//ImGui::PushTextWrapPos ( 450.0f );
				ImGui::TextUnformatted ( "Share" );
				//ImGui::PopTextWrapPos ();
				ImGui::EndTooltip ();
				ImGui::PopStyleColor () ;
				ImGui::PopStyleVar ( 2 ) ;
			}
			else {
				if ( !s_b )
					s_i = GetTickCount () ;
			}

			ImGui::End () ;
		}
		ImGui::PopStyleVar ( 6 ) ;
		ImGui::PopStyleColor () ;

		if ( s_bShowShare )
			ImGui::OpenPopup ( "Share Link" );

		ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 10, 10 ) ) ;
		ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 15, 15 ) ) ;
		if ( ImGui::BeginPopupModal ( "Share Link", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			char szSubsid [ MAX_PATH ] ;
			int i = WideCharToMultiByte ( CP_ACP, 0, m_strSubsid.c_str (), m_strSubsid.length (), szSubsid, MAX_PATH, NULL, NULL ) ;
			szSubsid [ i ] = 0 ;

			char szLink [ MAX_PATH ] ;
			sprintf ( szLink, "http://www.stuffsnet.com/models/%s", szSubsid ) ;

			ImGui::PushItemWidth ( 310.0f ) ;
			ImGui::InputText ( "", szLink, strlen(szLink) );

			if ( ImGui::Button ( "Close", ImVec2 ( 150, 0 ) ) ) { 
				ImGui::CloseCurrentPopup (); 
			}
			ImGui::SameLine ();
			if ( ImGui::Button ( "Copy to clipboard", ImVec2 ( 150, 0 ) ) ) { 
				CopyToClipboard ( GetSafeHwnd (), szLink ) ;
				ImGui::CloseCurrentPopup (); 
			}
			ImGui::EndPopup ();
		}
		ImGui::PopStyleVar (2 ) ;
		s_bShowShare = false ;

	}

	{ // Prev/Next for primary model
		if ( m_pd3dModel1 && m_pd3dModel1->Parts.size () > 1 ) {
			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;

			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () / 4.0f - 30.0f , 20.0f ) ) ;
			if ( ImGui::Begin ( "pp", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				static bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_PREV_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_PREV_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_pd3dModel1->Parts [ m_iCurPart1 ].bVisible = false ;
					m_iCurPart1 = ( m_iCurPart1 - 1 ) % m_pd3dModel1->Parts.size () ;
					m_pd3dModel1->Parts [ m_iCurPart1 ].bVisible = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					//ImGui::PushTextWrapPos ( 450.0f );
					ImGui::TextUnformatted ( "Previous Part" );
					//ImGui::PopTextWrapPos ();
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}

			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () / 4.0f + 30.0f, 20.0f ) ) ;
			if ( ImGui::Begin ( "pn", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				static bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_NEXT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_NEXT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_pd3dModel1->Parts [ m_iCurPart1 ].bVisible = false ;
					m_iCurPart1 = ( m_iCurPart1 + 1 ) % m_pd3dModel1->Parts.size () ;
					m_pd3dModel1->Parts [ m_iCurPart1 ].bVisible = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					//ImGui::PushTextWrapPos ( 450.0f );
					ImGui::TextUnformatted ( "Next Part" );
					//ImGui::PopTextWrapPos ();
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}


			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;
		}
	}

	{ // Prev/Next for secondary model
		if ( m_pd3dModel2 && m_bDualView && m_pd3dModel2->Parts.size () > 1 ) {
			flags = 0 ;
			flags |= ImGuiWindowFlags_NoMove;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_NoCollapse;
			//flags |= ImGuiWindowFlags_NoInputs;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor ( ImGuiCol_WindowBg, ImVec4 ( 0, 0, 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_ItemInnerSpacing, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowBorderSize, 0.0f ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 0, 0 ) ) ;
			ImGui::PushStyleVar ( ImGuiStyleVar_WindowMinSize, ImVec2 ( 0, 0 ) ) ;

			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () * 3.0f / 4.0f - 30.0f, 20.0f ) ) ;
			if ( ImGui::Begin ( "sp", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_PREV_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_PREV_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_pd3dModel2->Parts [ m_iCurPart2 ].bVisible = false ;
					m_iCurPart2 = ( m_iCurPart2 - 1 ) % m_pd3dModel2->Parts.size () ;
					m_pd3dModel2->Parts [ m_iCurPart2 ].bVisible = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					//ImGui::PushTextWrapPos ( 450.0f );
					ImGui::TextUnformatted ( "Previous Part" );
					//ImGui::PopTextWrapPos ();
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}

			ImGui::SetNextWindowPos ( ImVec2 ( rc.Width () * 3.0f / 4.0f + 30.0f, 20.0f ) ) ;
			if ( ImGui::Begin ( "sn", NULL, ImVec2 ( 0.0f, 0.0f ), -1, flags ) ) {
				SetWindowSize ( ImVec2 ( 30, 30 ) ) ;

				bool s_b = false ;
				ImTextureID texId = m_ptexUiElements [ UI_ELEM_NEXT_OFF ] ;
				if ( s_b )
					texId = m_ptexUiElements [ UI_ELEM_NEXT_ON ] ;
				if ( ImGui::ImageButton ( texId, ImVec2 ( 30.0f, 30.0f ), ImVec2 ( 0, 0 ), ImVec2 ( 1, 1 ), 0, ImVec4 ( 0, 0, 0, 0 ) ) ) {
					m_pd3dModel2->Parts [ m_iCurPart2 ].bVisible = false ;
					m_iCurPart2 = ( m_iCurPart2 + 1 ) % m_pd3dModel2->Parts.size () ;
					m_pd3dModel2->Parts [ m_iCurPart2 ].bVisible = true ;
				}
				if ( IsItemHovered ( ImGuiHoveredFlags_RectOnly ) )
					s_b = true ;
				else
					s_b = false ;

				static int s_i = GetTickCount () ;
				if ( s_b && GetTickCount () - s_i >= TOOLTIP_DELAY ) {
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowPadding, ImVec2 ( 5, 5 ) ) ;
					ImGui::PushStyleVar ( ImGuiStyleVar_WindowRounding, 4.0f ) ;
					ImGui::PushStyleColor ( ImGuiCol_PopupBg, ImVec4 ( 0, 0, 0, 0.5f ) ) ;
					ImGui::BeginTooltip ();
					//ImGui::PushTextWrapPos ( 450.0f );
					ImGui::TextUnformatted ( "Next Part" );
					//ImGui::PopTextWrapPos ();
					ImGui::EndTooltip ();
					ImGui::PopStyleColor () ;
					ImGui::PopStyleVar ( 2 ) ;
				}
				else {
					if ( !s_b )
						s_i = GetTickCount () ;
				}

				ImGui::End () ;
			}


			ImGui::PopStyleVar ( 6 ) ;
			ImGui::PopStyleColor () ;
		}
	}

	{ // Size Info
		flags = 0 ;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		flags |= ImGuiWindowFlags_NoCollapse;
		flags |= ImGuiWindowFlags_NoTitleBar;
		flags |= ImGuiWindowFlags_NoScrollbar;
		flags |= ImGuiWindowFlags_AlwaysAutoResize ;

		SetNextWindowPos ( ImVec2 ( 70.0f, 20.0f ) ) ;
		if ( ImGui::Begin ( "Size", NULL, flags ) ) {

			ImGui::Checkbox ( "Size Info", &m_bShowSize ) ;
			if ( m_bShowSize && m_pd3dModel1 ) {
				ImGui::PushStyleColor ( ImGuiCol_Text, ImVec4 ( 1.0f, 0, 0, 1.0f ) ) ;
				ImGui::Text ( "X = %.2f\"", m_vModelSize1.x ) ;
				ImGui::PushStyleColor ( ImGuiCol_Text, ImVec4 ( 0, 1.0f, 0, 1.0f ) ) ;
				ImGui::Text ( "Y = %.2f\"", m_vModelSize1.y ) ;
				ImGui::PushStyleColor ( ImGuiCol_Text, ImVec4 ( 0, 0.5f, 1.0f, 1.0f ) ) ;
				ImGui::Text ( "Z = %.2f\"", m_vModelSize1.z ) ;
				ImGui::PopStyleColor ( 3 ) ;
			}

			ImGui::End ();
		}
	}


	ImGui::EndFrame () ;
}

void CModelViewerDlg::CreateChecker ( float w, float h )
{
	return ;
	SAFE_DELETE ( m_pCheckerVB ) ;
	SAFE_DELETE ( m_pCheckerIB ) ;

	int iw = (int)(floorf ( w + 0.5f )) ;
	int ih = (int)(floorf ( h + 0.5f )) ;

	m_pCheckerVB = new vector3 [ (iw+1) * (ih+1) ] ;

	int iIndex = 0 ;
	for ( float j = 0.0f ; j <= ih ; j++ ) {
		for ( float i = 0.0f ; i <= iw ; i++ ) {

			m_pCheckerVB [ iIndex ] = vector3 ( i, 0.0f, j ) ;

			iIndex++ ;
		}
	}

	m_iCheckerVertCount = (iw+1) * (ih+1) ;

	m_pCheckerIB = new int [ ( iw - 0 ) * ( ih - 0 ) * 6 ] ;

	iIndex = 0 ;
	for ( int j = 0 ; j < ih ; j++ ) {
		for ( int i = 0 ; i < iw ; i++ ) {

 			if ( (i + j) % 2 == 0 )
 				continue ; 

			m_pCheckerIB [ iIndex++ ] = i + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + (j+1) * (iw+1) ;

			m_pCheckerIB [ iIndex++ ] = i + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + ( j + 1 ) * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + (j+1) * (iw+1) ;
		}
	}

	for ( int j = 0 ; j < ih ; j++ ) {
		for ( int i = 0 ; i < iw ; i++ ) {

			if ( (i + j) % 2 != 0 )
				continue ; 

			m_pCheckerIB [ iIndex++ ] = i + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + ( j + 1 ) * (iw+1) ;

			m_pCheckerIB [ iIndex++ ] = i + j * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + 1 + ( j + 1 ) * (iw+1) ;
			m_pCheckerIB [ iIndex++ ] = i + ( j + 1 ) * (iw+1) ;
		}
	}

	m_iCheckerIndexCount = ( iw ) * ( ih ) * 6 ;
}

// void CModelViewerDlg::UpdateWorldMatrix2 ()
// {
// 	if ( ! m_bDualView || ( m_bDualView && m_pd3dModel2 == NULL ) ) { // If single view
// 		vector3 ptOfs ;
// 		vector3 vBounding1 ;
// 		vector3 ptCenter1 ;
// 		if ( m_pd3dModel1 ) {
// 			vBounding1 = { m_pd3dModel1->ptMax.x - m_pd3dModel1->ptMin.x,
// 				m_pd3dModel1->ptMax.y - m_pd3dModel1->ptMin.y,
// 				m_pd3dModel1->ptMax.z - m_pd3dModel1->ptMin.z } ;
// 
// 			ptOfs.x = -( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
// 			ptOfs.z = -( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;
// 			ptOfs.y = -( m_pd3dModel1->ptMin.y ) ;
// 
// 			ptCenter1.x = ( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
// 			ptCenter1.z = ( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;
// 			ptCenter1.y = ( m_pd3dModel1->ptMax.y + m_pd3dModel1->ptMin.y ) / 2.0f ;
// 		}
// 
// 		D3DXMATRIX matYaw ;
// 		D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
// 		D3DXMATRIX matPitch ;
// 		D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;
// 
// 		D3DXMATRIX matTrans ;
// 		//D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;
// 		D3DXMatrixTranslation ( &matTrans, ptOfs.x, ptOfs.y, ptOfs.z ) ;
// 
// 		//m_matWorld = matYaw * matPitch * matTrans ;
// 		m_matWorld1 = matTrans * matYaw * matPitch ;
// 
// 		//m_matChecker1 = matYaw * matPitch ;
// 
// 		vector3 ptTarget ( 0.0f, vBounding1.y / 2.0f, 0.0f ) ;
// 		ptCenter1.y += ptOfs.y ;
// 		//m_Camera.SetTarget ( ptTarget ) ;
// 	}
// 	else {
// 		vector3 ptOfs1 ;
// 		//vector3 vBounding1 ;
// 		vector3 ptCenter1 ;
// 
// 		vector3 vBounding1 = { m_pd3dModel1->ptMax.x - m_pd3dModel1->ptMin.x,
// 			m_pd3dModel1->ptMax.y - m_pd3dModel1->ptMin.y,
// 			m_pd3dModel1->ptMax.z - m_pd3dModel1->ptMin.z } ;
// 
// 		ptOfs1.x = -( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
// 		ptOfs1.z = -( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;
// 		ptOfs1.y = -( m_pd3dModel1->ptMin.y ) ;
// 
// 		ptCenter1.x = ( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
// 		ptCenter1.z = ( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;
// 		ptCenter1.y = ( m_pd3dModel1->ptMax.y + m_pd3dModel1->ptMin.y ) / 2.0f ;
// 
// 		vector3 ptOfs2 ;
// 		//vector3 vBounding1 ;
// 		vector3 ptCenter2 ;
// 
// 		ptOfs2.x = -( m_pd3dModel2->ptMax.x + m_pd3dModel2->ptMin.x ) / 2.0f ;
// 		ptOfs2.z = -( m_pd3dModel2->ptMax.z + m_pd3dModel2->ptMin.z ) / 2.0f ;
// 		ptOfs2.y = -( m_pd3dModel2->ptMin.y ) ;
// 
// 		ptCenter2.x = ( m_pd3dModel2->ptMax.x + m_pd3dModel2->ptMin.x ) / 2.0f ;
// 		ptCenter2.z = ( m_pd3dModel2->ptMax.z + m_pd3dModel2->ptMin.z ) / 2.0f ;
// 		ptCenter2.y = ( m_pd3dModel2->ptMax.y + m_pd3dModel2->ptMin.y ) / 2.0f ;
// 
// 		D3DXMATRIX matYaw ;
// 		D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
// 		D3DXMATRIX matPitch ;
// 		D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;
// 
// 		D3DXMATRIX matTrans ;
// 		//D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;
// 		D3DXMatrixTranslation ( &matTrans, ptOfs1.x, ptOfs1.y, ptOfs1.z ) ;
// 
// 		//m_matWorld = matYaw * matPitch * matTrans ;
// 		m_matWorld1 = matTrans * matYaw * matPitch ;
// 
// 		D3DXMATRIX matTrans2 ;
// 		//D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;
// 		//D3DXMatrixTranslation ( &matTrans2, ptOfs2.x + m_pd3dModel1->ptMax.x + m_pd3dModel2->ptMax.x, ptOfs2.y, ptOfs2.z ) ;
// 		D3DXMatrixTranslation ( &matTrans2, ptOfs2.x, ptOfs2.y, ptOfs2.z ) ;
// 
// 		m_matWorld2 = matTrans2 * matYaw * matPitch ;
// 
// // 		m_matChecker1 = matYaw * matPitch ;
// // 		m_matChecker2 = matYaw * matPitch ;
// 
// 		vector3 ptCenterAll ;
// 		ptCenterAll.x = ( ptCenter1.x + ptCenter2.x ) / 2.0f ;
// 		ptCenterAll.y = ( ptCenter1.y + ptCenter2.y ) / 2.0f ;
// 		ptCenterAll.z = ( ptCenter1.z + ptCenter2.z ) / 2.0f ;
// 
// 		vector3 ptTarget ( 0.0f, vBounding1.y / 2.0f, 0.0f ) ;
// // 		ptCenter1.y += ptOfs.y ;
// // 		ptCenter1.y += ptOfs1.y ;
// 		//m_Camera.SetTarget ( ptTarget ) ;
// 		//ptCenterAll.y += ptOfs1.y ;
// 		//m_Camera.SetTarget ( ptCenterAll ) ;
// 	}
// 
// 	if ( m_pd3dModel1 || m_pd3dModel2 ) {
// 		D3DXMATRIX matTrans ;
// 		D3DXMatrixTranslation ( &matTrans, m_ptCheckerPos.x, 0.0f, m_ptCheckerPos.z ) ;
// 
// 		D3DXMATRIX matYaw ;
// 		D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
// 		D3DXMATRIX matPitch ;
// 		D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;
// 
// 		m_matChecker1 = matTrans * matYaw * matPitch ;
// 		m_matChecker2 = matTrans * matYaw * matPitch ;
// 	}
// 
// }

void CModelViewerDlg::FillTextureList ()
{
	if ( m_ppszTextureNames ) {
		for ( int i = 0 ; i < m_iTextureCount ; i++ ) {
			SAFE_DELETE ( m_ppszTextureNames [ i ] ) ;
		}
		SAFE_DELETE ( m_ppszTextureNames ) ;
		m_iTextureCount = 0 ;
	}

	if ( m_pd3dModel1 ) {

		m_iTextureCount = m_pd3dModel1->Textures.size() + 1 ;
		m_ppszTextureNames = new char* [ m_iTextureCount ] ;

		for ( uint32_t iTex = 0 ; iTex < m_pd3dModel1->Textures.size() ; iTex++ ) {
			auto i = m_pd3dModel1->Textures.begin () ;
			advance ( i, iTex ) ;

			m_ppszTextureNames [ iTex ] = new char [ i->second.pBase->sName.size () + 1 ] ;
			strncpy ( m_ppszTextureNames [ iTex ], i->second.pBase->sName.c_str (), i->second.pBase->sName.size () + 1 ) ;
		}
		m_ppszTextureNames [ m_iTextureCount - 1 ] = new char [ 3 ] ;
		strncpy ( m_ppszTextureNames [ m_iTextureCount - 1 ], "", 1 ) ;
	}
}

void CModelViewerDlg::CalcTextureAverages()
{
	if ( ! m_pd3dModel1 )
		return ;

	for ( uint32_t iTex = 0 ; iTex < m_pd3dModel1->Textures.size () ; iTex++ ) {
		auto i = m_pd3dModel1->Textures.begin () ;
		advance ( i, iTex ) ;

		FIMEMORY* pMem = FreeImage_OpenMemory ( (BYTE*)i->second.pBase->pData, i->second.pBase->uiSize )	;
		FREE_IMAGE_FORMAT fmt = FreeImage_GetFileTypeFromMemory ( pMem ) ;
		FIBITMAP* pBmp = FreeImage_LoadFromMemory ( fmt, pMem ) ;
		FreeImage_CloseMemory ( pMem ) ;

		if ( pBmp ) {

			uint64_t R = 0 ;
			uint64_t G = 0 ;
			uint64_t B = 0 ;
			uint64_t A = 0 ;

			int w = FreeImage_GetWidth ( pBmp ) ;
			int h = FreeImage_GetHeight ( pBmp ) ;

			for ( int y = 0 ; y < h ; y++ ) {
				for ( int x = 0 ; x < w ; x++ ) {
					RGBQUAD c ;
					FreeImage_GetPixelColor ( pBmp, x, y, &c ) ;

					R += c.rgbRed ;
					G += c.rgbGreen ;
					B += c.rgbBlue ;
					A += c.rgbReserved ;
				}
			}

			R /= ( w * h ) ;
			G /= ( w * h ) ;
			B /= ( w * h ) ;
			A /= ( w * h ) ;

			i->second.pBase->clrAvgColor = float4_rgba{ (float)R / 255.0f, (float)G / 255.0f, (float)B / 255.0f, (float)A / 255.0f } ;
			int mm = 1 ;
		}

	}

}

bool CModelViewerDlg::Load3DScanFile ( CString& strPathName )
{
	TD_SCAN_MODEL* pModel = C3DScanFile::Load3DScanModelFromFile ( strPathName.GetBuffer () ) ;
	if ( pModel ) {
		D3D_MODEL* pd3dModel = new D3D_MODEL ;
		if ( ! CD3DModelUtils::CreateFromTDModel ( C3DGfx::GetInstance ()->GetDevice (), C3DGfx::GetInstance ()->GetEffectPool (), *pModel, *pd3dModel ) ) {
			SAFE_DELETE ( pd3dModel ) ;
			C3DModelUtils::FreeModel ( *pModel ) ;
			SAFE_DELETE ( pModel ) ;
		}
		else {
			if ( m_pd3dModel1 ) {
				CD3DModelUtils::FreeD3DModel ( *m_pd3dModel1 ) ;
				SAFE_DELETE ( m_pd3dModel1 ) ;
			}
			if ( m_pd3dModel1 ) {
				C3DModelUtils::FreeModel ( *m_pModel1 ) ;
				SAFE_DELETE ( m_pModel1 ) ;
			}

			m_pd3dModel1 = pd3dModel ;
			m_pModel1 = pModel ;

			OnModelLoaded1 () ;

			ResetView () ;

			FillTextureList () ;
			m_strFilename = strPathName ;
			m_bFileOpened = true ;
			m_bHasFilename = true ;
		}
	}
	
	return true ;
}

bool CModelViewerDlg::Load3DScanFile2 ( CString& strPathName )
{
	TD_SCAN_MODEL* pModel = C3DScanFile::Load3DScanModelFromFile ( strPathName.GetBuffer () ) ;
	if ( pModel ) {
		D3D_MODEL* pd3dModel = new D3D_MODEL ;
		if ( !CD3DModelUtils::CreateFromTDModel ( C3DGfx::GetInstance ()->GetDevice (), C3DGfx::GetInstance ()->GetEffectPool (), *pModel, *pd3dModel ) ) {
			SAFE_DELETE ( pd3dModel ) ;
			C3DModelUtils::FreeModel ( *pModel ) ;
			SAFE_DELETE ( pModel ) ;
		}
		else {
			if ( m_pd3dModel2 ) {
				CD3DModelUtils::FreeD3DModel ( *m_pd3dModel2 ) ;
				SAFE_DELETE ( m_pd3dModel2 ) ;
			}
			if ( m_pd3dModel2 ) {
				C3DModelUtils::FreeModel ( *m_pModel2 ) ;
				SAFE_DELETE ( m_pModel2 ) ;
			}

			m_pd3dModel2 = pd3dModel ;
			m_pModel2 = pModel ;

			OnModelLoaded2() ;

			ResetView () ;

			FillTextureList () ;
			//m_strFilename = strPathName ;
			m_bFileOpened = true ;
			//m_bHasFilename = true ;
		}
	}

	return true ;
}

bool CModelViewerDlg::Load3DScanFile ( std::wstring& strPathName )
{
	char szFile [ MAX_PATH ] ;
	int iLen = WideCharToMultiByte ( CP_ACP, 0, strPathName.c_str (), strPathName.length (), szFile, MAX_PATH, "", NULL ) ;
	szFile [ iLen ] = 0 ;
	
	return Load3DScanFile ( (CString)szFile ) ;
}

bool CModelViewerDlg::Load3DScanFile2 ( std::wstring& strPathName )
{
	char szFile [ MAX_PATH ] ;
	int iLen = WideCharToMultiByte ( CP_ACP, 0, strPathName.c_str (), strPathName.length (), szFile, MAX_PATH, "", NULL ) ;
	szFile [ iLen ] = 0 ;

	return Load3DScanFile2 ( (CString)szFile ) ;
}

bool CModelViewerDlg::Load3DScanFromUrl ( CString& strUrl )
{
	strUrl.Replace ( "3dscan://", MODEL_SERVICE_SCHEME"://" ) ;

	wchar_t szUrl [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, strUrl.GetBuffer (), strUrl.GetLength (), szUrl, 1000 ) ;
	szUrl [ iLen ] = 0 ;

	web::uri original ( szUrl );

	web::uri_builder builder ( original ) ;
	auto query = builder.query () ;
	auto query_split = web::uri::split_query ( query ) ;

	m_strSubsid = query_split [ L"subsid" ] ;

	bool bDownloadModel = true ;
	bool bDownloadInfo	= true ;
	bool bDownloadAd	= true ;

	{
		void* pThumbData = NULL ;
		int iThumbSize = 0 ;
		if ( m_Cache.LoadModel ( query_split[L"subsid"], &m_PointerPass [ 1 ].pData, &m_PointerPass [ 1 ].iSize, &m_PointerPass [ 2 ].pData, &m_PointerPass [ 2 ].iSize, &pThumbData, &iThumbSize ) ) {

			PostMessage ( WM_USER_MODEL_DOWNLOADED, 1, 1 ) ;
			PostMessage ( WM_USER_AD_DOWNLOADED, 2, 1 ) ;

			bDownloadModel = false ;
			bDownloadInfo = false ;
		}
		if ( m_PointerPass [ 2 ].pData && m_PointerPass [ 2 ].iSize )
			bDownloadAd = false ;
	}

	wstring strInfo ;
	if ( bDownloadInfo ) {
		web::uri_builder builder_mdl ;
		builder_mdl.set_scheme ( U(MODEL_SERVICE_SCHEME) ) ;
		builder_mdl.set_host ( builder.host () ) ;
		builder_mdl.set_port ( builder.port () ) ;
		builder_mdl.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
		builder_mdl.append_path ( U ( MODEL_API_GET_INFO ) ) ;
		builder_mdl.append_query ( L"subsid", query_split [ L"subsid" ] ) ;
		builder_mdl.append_query ( L"magic", U (MODEL_API_MAGIC) ) ;

		wstring strUrl2 = builder_mdl.to_string() ;

		strInfo = strUrl2 ;
		
		DownloadInfo ( strInfo ) ;
	}

	if ( bDownloadModel ) {
		web::uri_builder builder_mdl ;
		builder_mdl.set_scheme ( U ( MODEL_SERVICE_SCHEME ) ) ;
		builder_mdl.set_host ( builder.host () ) ;
		builder_mdl.set_port ( builder.port () ) ;
		builder_mdl.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
		builder_mdl.append_path ( U ( MODEL_API_GET ) ) ;
		builder_mdl.append_query ( L"subsid", query_split [ L"subsid" ] ) ;
		builder_mdl.append_query ( L"magic", U ( MODEL_API_MAGIC ) ) ;

		wstring strUrl2 = builder_mdl.to_string() ;

		m_strModel = strUrl2 ;

		DownloadModel ( m_strModel ) ;
	}

	if ( bDownloadAd ) {
		web::uri_builder builder_ad ;
		builder_ad.set_scheme ( U ( MODEL_SERVICE_SCHEME ) ) ;
		builder_ad.set_host ( builder.host () ) ;
		builder_ad.set_port ( builder.port () ) ;
		builder_ad.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
		builder_ad.append_path ( U ( MODEL_API_GET_AD ) ) ;
		builder_ad.append_query ( L"subsid", query_split [ L"subsid" ] ) ;
		builder_ad.append_query ( L"magic", U ( MODEL_API_MAGIC ) ) ;

		wstring strUrl2 = builder_ad.to_string () ;
		m_strAd = strUrl2 ;

		DownloadAd ( m_strAd ) ;
	}


	return true ;
}

static bool IsAnyMouseButtonDown ()
{
	ImGuiIO& io = ImGui::GetIO ();
	for ( int n = 0; n < ARRAYSIZE ( io.MouseDown ); n++ )
		if ( io.MouseDown [ n ] )
			return true;
	return false;
}

LRESULT ImGui_ImplWin32_WndProcHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CRect rc ;

	ImGuiIO& io = ImGui::GetIO ();
	switch ( msg )
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		int button = 0;
		if ( msg == WM_LBUTTONDOWN ) button = 0;
		if ( msg == WM_RBUTTONDOWN ) button = 1;
		if ( msg == WM_MBUTTONDOWN ) button = 2;
		if ( !IsAnyMouseButtonDown () && GetCapture () == NULL )
			SetCapture ( hwnd );
		io.MouseDown [ button ] = true;
		return 0;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		int button = 0;
		if ( msg == WM_LBUTTONUP ) button = 0;
		if ( msg == WM_RBUTTONUP ) button = 1;
		if ( msg == WM_MBUTTONUP ) button = 2;
		io.MouseDown [ button ] = false;
		if ( !IsAnyMouseButtonDown () && GetCapture () == hwnd )
			ReleaseCapture ();
		return 0;
	}
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM ( wParam ) > 0 ? +1.0f : -1.0f;
		return 0;
	case WM_MOUSEMOVE:
		g_pDlg->GetClientRect ( rc ) ;
		io.MousePos.x = (signed short)( lParam );
		io.MousePos.y = (signed short)( lParam >> 16 );
// 		if ( io.MousePos.x < 50 && io.MousePos.y > 50 && ! io.MouseDown [ 0 ] && ! g_pDlg->m_bShowToolbar ) {
// 			if ( ! g_pDlg->m_bShowRecentLeft ) {
// 				g_pDlg->m_bShowRecentLeft = true ;
// 
// 			}
// 		}
// 		if ( io.MousePos.x > rc.Width() - 50 && io.MousePos.y > 50 && !io.MouseDown [ 0 ] && !g_pDlg->m_bShowToolbar /*&& g_pDlg->m_bDualView */) {
// 			if ( !g_pDlg->m_bShowRecentRight ) {
// 				g_pDlg->m_bShowRecentRight = true ;
// 
// 			}
// 		}
// 		if ( 0 )
// 		if ( io.MousePos.y < 30 && !io.MouseDown [ 0 ] && ! g_pDlg->m_bShowRecentLeft ) {
// 			if ( !g_pDlg->m_bShowToolbar ) {
// 				g_pDlg->m_bShowToolbar = true ;
// 
// 			}
// 		}
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if ( wParam < 256 )
			io.KeysDown [ wParam ] = 1;
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ( wParam < 256 )
			io.KeysDown [ wParam ] = 0;
		return 0;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if ( wParam > 0 && wParam < 0x10000 )
			io.AddInputCharacter ( (unsigned short)wParam );
		return 0;
	}
	return 0;
}

void CModelViewerDlg::ResetView()
{
	if ( m_pd3dModel1 ) {

		// Create checker
		vector3 vBounding1 ;

		vBounding1 = { m_pd3dModel1->ptMax.x - m_pd3dModel1->ptMin.x,
			m_pd3dModel1->ptMax.y - m_pd3dModel1->ptMin.y,
			m_pd3dModel1->ptMax.z - m_pd3dModel1->ptMin.z } ;

		vBounding1.x = roundf ( vBounding1.x ) * 2.0f ;
		vBounding1.z = roundf ( vBounding1.z ) * 2.0f ;

		CreateChecker ( vBounding1.x, vBounding1.z ) ;

		m_ptCheckerPos.x = -vBounding1.x / 2.0f ;
		m_ptCheckerPos.z = -vBounding1.z / 2.0f ;

// 		vector3 ptTarget ( 0.0f, vBounding1.y / 2.0f, 0.0f ) ;
// 		m_Camera.SetTarget ( ptTarget ) ;

		// Set optimal distance
		float dy = 1.0f / tanf ( m_Camera.GetFovY () / 2.0f ) ;
		float zy = dy * ( m_pModel1->ptMax.y - m_pModel1->ptMin.y ) * 1.4f / 2.0f ;

		float dx = 1.0f / tanf ( m_Camera.GetFovX () / 2.0f ) ;
		float zx = dx * ( m_pModel1->ptMax.x - m_pModel1->ptMin.x ) * 1.4f / 2.0f ;

		float z = zy ;
		if ( zx > zy )
			z = zx ;

		m_Camera.SetDistance ( z ) ;

		m_fYaw = 0.0f ;
		m_fPitch = 0.0f ;
		m_ptPos1 = vector3 ( 0.0f, 0.0f, 0.0f ) ;

		UpdateWorldMatrix () ;
	}
}

void MyInfoCallback ( int iResult, char* pData, int iSize, int iFileSize )
{
	g_pDlg->m_PointerPass [ 0 ].pData = pData ;
	g_pDlg->m_PointerPass [ 0 ].iSize = iSize ;

	g_pDlg->PostMessage ( WM_USER_MODEL_INFO, 0, iFileSize ) ;
}

void MyModelCallback ( int iResult, char* pData, int iSize )
{
	g_pDlg->m_PointerPass [ 1 ].pData = pData ;
	g_pDlg->m_PointerPass [ 1 ].iSize = iSize ;

	g_pDlg->PostMessage ( WM_USER_MODEL_DOWNLOADED, 1, 0 ) ;
}

void MyAdCallback ( int iResult, char* pData, int iSize, std::wstring& strAdUrl )
{
	g_pDlg->m_PointerPass [ 2 ].pData = pData ;
	g_pDlg->m_PointerPass [ 2 ].iSize = iSize ;

	g_pDlg->m_strAdUrl = strAdUrl ;

	g_pDlg->PostMessage ( WM_USER_AD_DOWNLOADED, 2, 0 ) ;
}

std::thread InfoThread ;
std::thread ModelThread ;
std::thread AdThread ;

void CModelViewerDlg::DownloadInfo ( wstring& strUrl )
{
	char* szClientId = new char [ 100 ] ;
	strcpy ( szClientId, MODEL_CLIENT_ID_PCWIN ) ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;
	wcscpy ( szUrl, strUrl.c_str () ) ;

	int* piFileSize = new int ;
	*piFileSize = 0 ;

	std::function<void ( int iResult, char* pHdr, int iSize, int iFileSize )> myCallback = MyInfoCallback ;
	InfoThread = std::thread ( [ = ]( wchar_t* pUrl, char* pClientId, int* pFileSize ) {
		CModelServiceWebClient client ;
		char* pData = NULL ;
		int iSize = 0 ;
		int iRes = client.GetModelInfo ( pUrl, pClientId, &pData, &iSize, pFileSize ) ;
		if ( myCallback )
			myCallback ( iRes, pData, iSize, *piFileSize ) ;
		SAFE_DELETE ( pUrl ) ;
		SAFE_DELETE ( pClientId ) ;
		SAFE_DELETE ( pFileSize ) ;
	}, szUrl, szClientId, piFileSize ) ;
	InfoThread.detach () ;
}

void CModelViewerDlg::DownloadModel ( wstring& strUrl )
{
	m_bDownloading = true ;

	char* szClientId = new char [ 100 ] ;
	strcpy ( szClientId, MODEL_CLIENT_ID_PCWIN ) ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;

	wcscpy ( szUrl, strUrl.c_str() ) ;

	std::function<void ( int iResult, char* pszOrderId, int iSize )> myCallback = MyModelCallback ;
	ModelThread = std::thread ( [ = ]( wchar_t* pUrl, char* pClientId ) {
		CModelServiceWebClient client ;
		char* pData = NULL ;
		int iSize = 0 ;
		int iRes = client.GetModel ( pUrl, pClientId, &pData, &iSize ) ;
		if ( myCallback )
			myCallback ( iRes, pData, iSize ) ;
		SAFE_DELETE ( pUrl ) ;
		SAFE_DELETE ( pClientId ) ;
	}, szUrl, szClientId ) ;
	ModelThread.detach() ;
}

void CModelViewerDlg::DownloadAd ( wstring& strUrl )
{
	char* szClientId = new char [ 100 ] ;
	strcpy ( szClientId, MODEL_CLIENT_ID_PCWIN ) ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;
	wcscpy ( szUrl, strUrl.c_str () ) ;

	std::function<void ( int iResult, char* pData, int iSize, std::wstring& strAdUrl )> myCallback = MyAdCallback ;
	AdThread = std::thread ( [ = ]( wchar_t* pUrl, char* pClientId ) {
		CModelServiceWebClient client ;
		char* pData = NULL ;
		int iSize = 0 ;
		std::wstring strAdUrl ;
		int iRes = client.GetAd ( pUrl, pClientId, &pData, iSize, strAdUrl ) ;
		if ( myCallback )
			myCallback ( iRes, pData, iSize, strAdUrl ) ;
		SAFE_DELETE ( pUrl ) ;
		SAFE_DELETE ( pClientId ) ;
	}, szUrl, szClientId ) ;
	AdThread.detach () ;
}

void CModelViewerDlg::GenerateThumnail ( void** ppData, int* piSize ) 
{
	if ( ! m_pd3dModel1 ) 
		return ;

	CCamera* pCamera = m_pThumbView->GetCamera() ;

	float dy = 1.0f / tanf ( pCamera->GetFovY () / 2.0f ) ;
	float zy = dy * ( m_pModel1->ptMax.y - m_pModel1->ptMin.y ) * 1.25f / 2.0f ;

	float dx = 1.0f / tanf ( m_Camera.GetFovX () / 2.0f ) ;
	float zx = dx * ( m_pModel1->ptMax.x - m_pModel1->ptMin.x ) * 1.25f / 2.0f ;

	float z = zy ;
	if ( zx > zy )
		z = zx ;

	pCamera->SetDistance ( z ) ;

	m_pThumbView->SelectView() ;
	C3DGfx::GetInstance()->Clear ( 0x20202020 ) ;
	IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;

	matrix matI ;
	D3DXMatrixIdentity ( &matI ) ;

	m_pShader->SetMatrix ( "g_matView", &pCamera->GetViewMatrix () ) ;
	m_pShader->SetMatrix ( "g_matProj", &pCamera->GetProjectionMatrix () ) ;
	m_pShader->SetMatrix ( "g_matWorld", &matI ) ;

	vector4 vLightDir ( 0.0f, 0.0f, 0.0f, 1.0f ) ;
	D3DXVec4Normalize ( &vLightDir, &vLightDir ) ;
	vector4 vAmbLight ( 0.5f, 0.5f, 0.5f, 0.0f ) ;

	m_pShader->SetVector ( "g_vSunLightDir", &vLightDir ) ;
	m_pShader->SetVector ( "g_f4SunLightDiffuse", (D3DXVECTOR4*)&m_clrLight ) ;
	m_pShader->SetVector ( "g_f4SunLightAmbient", &vAmbLight ) ;

	CD3DModelUtils::RenderD3DModel ( pDevice, *m_pd3dModel1 ) ;

	ID3DXBuffer* pBuf = NULL ;
	D3DXSaveSurfaceToFileInMemory ( &pBuf,
		D3DXIFF_PNG,
		m_pThumbView->GetRenderTarget(),
		NULL,
		NULL ) ;

	if ( pBuf ) {
		*piSize = pBuf->GetBufferSize () ;
		*ppData = new uint8_t [ pBuf->GetBufferSize () ]  ;
		CopyMemory ( *ppData, pBuf->GetBufferPointer (), pBuf->GetBufferSize () ) ;

// 		D3DXSaveSurfaceToFile ( "thumb_.png",
// 		 	D3DXIFF_PNG,
// 		 	m_pThumbView->GetRenderTarget (),
// 		 	NULL,
// 		 	NULL ) ;

		pBuf->Release () ;
	}

}

void CModelViewerDlg::FillThumbArray ()
{
	if ( m_ppThumbnails ) {
		for ( int i = 0 ; i < m_iThumbCount ; i++ ) {
			if ( m_ppThumbnails [ i ] )
				m_ppThumbnails [ i ]->Release () ;
		}
		delete m_ppThumbnails ;
		m_ppThumbnails = NULL ;
		m_iTextureCount = 0 ;
	}

	if ( m_pstrModelFiles )
		delete[]m_pstrModelFiles ;
	m_pstrModelFiles = NULL ;

	if ( m_pstrAdFiles )
		delete []m_pstrAdFiles ;
	m_pstrAdFiles = NULL ;

	m_iThumbCount = m_Cache.GetEntryCount () ;

	if ( m_iThumbCount ) {
			m_ppThumbnails = new IDirect3DTexture9* [ m_iThumbCount ] ;
		ZeroMemory ( m_ppThumbnails, m_iThumbCount * sizeof ( void* ) ) ;

		m_pstrModelFiles = new std::wstring [ m_iThumbCount ] ;
		m_pstrAdFiles = new std::wstring [ m_iThumbCount ] ;

		for ( int i = 0 ; i < m_iThumbCount ; i++ ) {
			CModelCache::CACHE_ENTRY entry ;
			m_Cache.GetEntry ( m_iThumbCount - 1 - i, entry ) ;

			D3DXCreateTextureFromFileW ( C3DGfx::GetInstance ()->GetDevice (), entry.szThumbFile, &m_ppThumbnails [ i ] ) ;
			m_pstrModelFiles [ i ] = entry.szModelFile ;
			m_pstrAdFiles [ i ] = entry.szAdFile ;
		}
	}
}

bool CModelViewerDlg::IsNegativePitch ()
{
	vector3 v ( 0.0f, 0.0f , 1.0f ) ;
	matrix matRot ;
	D3DXMatrixRotationX ( &matRot, m_fPitch ) ;
	D3DXVec3TransformNormal ( &v, &v, &matRot ) ;

	return v.z < 0.0f ;
}

void CModelViewerDlg::DrawGrid ()
{
	return ;
	if ( ! m_pCheckerVB )
		return ;

// 	matrix matI ;
// 	D3DXMatrixIdentity ( &matI ) ;
// 	m_pShader->SetMatrix ( "g_matWorld", &matI ) ;

	IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;

	pDevice->SetFVF ( D3DFVF_XYZ ) ;

	m_pShader->SetFloatArray ( "g_f4AmbientColor", (float*)&float4_rgba { 0, 0, 0, 0 }, 4 ) ;
	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 1, 1, 1, 0 }, 4 ) ;
	m_pShader->SetFloat ( "g_fTransparency", 0.1f ) ;
	m_pShader->SetFloat ( "g_fGlossiness", 0.0f ) ;
	m_pShader->SetFloat ( "g_fSpecularIntensity", 0.0f ) ;

	m_pShader->SetBool ( "g_bHasNormal", FALSE ) ;
	m_pShader->SetBool ( "g_bHasUv", FALSE ) ;
	m_pShader->SetBool ( "g_bHasDiffTex", false ) ;
	m_pShader->SetBool ( "g_bHasAlphaTex", false ) ;
	m_pShader->SetBool ( "g_bHasNormTex", false ) ;
	m_pShader->SetBool ( "g_bHasSpecTex", false ) ;
	m_pShader->SetBool ( "g_bHasReflTex", false ) ;

	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 1, 1, 1, 0 }, 4 ) ;

	//pDevice->SetRenderState ( D3DRS_ZWRITEENABLE, FALSE ) ;

	UINT uiPassCount = 0 ;
	m_pShader->Begin ( &uiPassCount, 0 ) ;
	for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
		m_pShader->BeginPass ( iPass ) ;

		// 				pDevice->DrawPrimitiveUP ( D3DPT_TRIANGLELIST,
		// 					subset.pBase->uiTriCount,
		// 					subset.pBase->pVB,
		// 					C3DScanFileUtils::GetVertexSize (subset.pBase->uiVertexFmt) ) ;

		pDevice->DrawIndexedPrimitiveUP ( D3DPT_TRIANGLELIST,
			0,
			m_iCheckerVertCount,
			m_iCheckerIndexCount/3/2,
			m_pCheckerIB,
			D3DFMT_INDEX32,
			m_pCheckerVB,
			sizeof(vector3) ) ;

		m_pShader->EndPass () ;
	}
	m_pShader->End () ;

	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 0, 0, 0, 0 }, 4 ) ;

	//return ;

	uiPassCount = 0 ;
	m_pShader->Begin ( &uiPassCount, 0 ) ;
	for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
		m_pShader->BeginPass ( iPass ) ;

		pDevice->DrawIndexedPrimitiveUP ( D3DPT_TRIANGLELIST,
			0,
			m_iCheckerVertCount,
			m_iCheckerIndexCount / 3 / 2 ,
			(int*)m_pCheckerIB+m_iCheckerIndexCount/2,
			D3DFMT_INDEX32,
			m_pCheckerVB,
			sizeof ( vector3 ) ) ;

		m_pShader->EndPass () ;
	}
	m_pShader->End () ;

	//pDevice->SetRenderState ( D3DRS_ZWRITEENABLE, TRUE ) ;

}

void CModelViewerDlg::OnModelLoaded1 ()
{
	m_iCurPart1 = 0 ;

	m_ptModelCenter1.x = ( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
	m_ptModelCenter1.y = ( m_pd3dModel1->ptMax.y + m_pd3dModel1->ptMin.y ) / 2.0f ;
	m_ptModelCenter1.z = ( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;

	m_vModelSize1 = vector3 ( m_pd3dModel1->ptMax.x - m_pd3dModel1->ptMin.x, m_pd3dModel1->ptMax.y - m_pd3dModel1->ptMin.y, m_pd3dModel1->ptMax.z - m_pd3dModel1->ptMin.z ) ;

	m_fYaw = 0.0f ;
	m_fPitch = 0.0f ;
	m_ptPos1 = vector3 ( 0.0f, 0.0f, 0.0f ) ;

	D3DXMatrixTranslation ( &m_matCenter1, -m_ptModelCenter1.x, -m_ptModelCenter1.y, -m_ptModelCenter1.z ) ;

	ResetView() ;
}

void CModelViewerDlg::OnModelLoaded2 ()
{
	m_iCurPart2 = 0 ;

	m_ptModelCenter2.x = ( m_pd3dModel2->ptMax.x + m_pd3dModel2->ptMin.x ) / 2.0f ;
	m_ptModelCenter2.y = ( m_pd3dModel2->ptMax.y + m_pd3dModel2->ptMin.y ) / 2.0f ;
	m_ptModelCenter2.z = ( m_pd3dModel2->ptMax.z + m_pd3dModel2->ptMin.z ) / 2.0f ;

	m_vModelSize2 = vector3 ( m_pd3dModel2->ptMax.x - m_pd3dModel2->ptMin.x, m_pd3dModel2->ptMax.y - m_pd3dModel2->ptMin.y, m_pd3dModel2->ptMax.z - m_pd3dModel2->ptMin.z ) ;

	D3DXMatrixTranslation ( &m_matCenter2, -m_ptModelCenter2.x, -m_ptModelCenter2.y, -m_ptModelCenter2.z ) ;
}

void CModelViewerDlg::DrawBounding ( D3D_MODEL* pModel, matrix& matWorld )
{
	if ( ! pModel )
		return ;

	int aXAxis[] = { 1, 2, 0, 3, 5, 6, 4, 7 } ;
	int aYAxis[] = { 0, 1, 2, 3, 4, 5, 6, 7 } ;
	int aZAxis[] = { 0, 4, 1, 5, 2, 6, 3, 7 } ;

	vector3 ptMin ( pModel->ptMin.x, pModel->ptMin.y, pModel->ptMin.z ) ;
	vector3 vSize ( pModel->ptMax.x - pModel->ptMin.x, pModel->ptMax.y - pModel->ptMin.y, pModel->ptMax.z - pModel->ptMin.z ) ;

	matrix matT ;
	D3DXMatrixTranslation ( &matT, ptMin.x, ptMin.y, ptMin.z ) ;
	matrix matS ;
	D3DXMatrixScaling ( &matS, vSize.x, vSize.y, vSize.z ) ;

	matrix matResult = matS * m_matRot * m_matMove ;
	m_pShader->SetMatrix ( "g_matWorld", &matResult ) ;

	IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;

	pDevice->SetFVF ( D3DFVF_XYZ ) ;

	pDevice->SetRenderState ( D3DRS_CULLMODE, D3DCULL_NONE ) ;

	m_pShader->SetFloatArray ( "g_f4AmbientColor", (float*)&float4_rgba { 0, 0, 0, 0 }, 4 ) ;
	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 1, 1, 1, 0 }, 4 ) ;
	m_pShader->SetFloat ( "g_fTransparency", 1.0f ) ;
	m_pShader->SetFloat ( "g_fGlossiness", 0.0f ) ;
	m_pShader->SetFloat ( "g_fSpecularIntensity", 0.0f ) ;

	m_pShader->SetBool ( "g_bHasNormal", FALSE ) ;
	m_pShader->SetBool ( "g_bHasUv", FALSE ) ;
	m_pShader->SetBool ( "g_bHasDiffTex", false ) ;
	m_pShader->SetBool ( "g_bHasAlphaTex", false ) ;
	m_pShader->SetBool ( "g_bHasNormTex", false ) ;
	m_pShader->SetBool ( "g_bHasSpecTex", false ) ;
	m_pShader->SetBool ( "g_bHasReflTex", false ) ;

	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 1, 0, 0, 1 }, 4 ) ;

	UINT uiPassCount = 0 ;
	m_pShader->Begin ( &uiPassCount, 0 ) ;
	for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
		m_pShader->BeginPass ( iPass ) ;

		pDevice->DrawIndexedPrimitiveUP ( 
			D3DPT_LINELIST,
			0,
			8,
			4,
			aXAxis,
			D3DFMT_INDEX32,
			m_avCube,
			sizeof ( vector3 ) ) ;

		m_pShader->EndPass () ;
	}
	m_pShader->End () ;


	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 0, 1, 0, 1 }, 4 ) ;

	uiPassCount = 0 ;
	m_pShader->Begin ( &uiPassCount, 0 ) ;
	for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
		m_pShader->BeginPass ( iPass ) ;

		pDevice->DrawIndexedPrimitiveUP (
			D3DPT_LINELIST,
			0,
			8,
			4,
			aYAxis,
			D3DFMT_INDEX32,
			m_avCube,
			sizeof ( vector3 ) ) ;

		m_pShader->EndPass () ;
	}
	m_pShader->End () ;

	m_pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&float4_rgba { 0, 0, 1, 1 }, 4 ) ;

	uiPassCount = 0 ;
	m_pShader->Begin ( &uiPassCount, 0 ) ;
	for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
		m_pShader->BeginPass ( iPass ) ;

		pDevice->DrawIndexedPrimitiveUP (
			D3DPT_LINELIST,
			0,
			8,
			4,
			aZAxis,
			D3DFMT_INDEX32,
			m_avCube,
			sizeof ( vector3 ) ) ;

		m_pShader->EndPass () ;
	}
	m_pShader->End () ;

	pDevice->SetRenderState ( D3DRS_CULLMODE, D3DCULL_CCW ) ;
}

 void CModelViewerDlg::UpdateWorldMatrix ()
 {
 	if ( /*! m_bDualView && */m_pd3dModel1 ) { // If single view
 		//vector3 ptOfs ;
 
 // 		ptOfs.x = -( m_pd3dModel1->ptMax.x + m_pd3dModel1->ptMin.x ) / 2.0f ;
 // 		ptOfs.z = -( m_pd3dModel1->ptMax.z + m_pd3dModel1->ptMin.z ) / 2.0f ;
 // 		ptOfs.y = -( m_pd3dModel1->ptMin.y ) ;
 // 
 		D3DXMATRIX matYaw ;
 		D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
 		D3DXMATRIX matPitch ;
 		D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;
 
 		m_matRot = matYaw * matPitch ;
 
 		D3DXMatrixTranslation ( &m_matMove, m_ptPos1.x, m_ptPos1.y, m_ptPos1.z ) ;
 
 		//D3DXMATRIX matTrans ;
 		//D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;
 		//D3DXMatrixTranslation ( &matTrans, ptOfs.x, ptOfs.y, ptOfs.z ) ;
 
 		//m_matWorld = matYaw * matPitch * matTrans ;
 		m_matWorld1 = m_matCenter1 * m_matRot * m_matMove ;
 		m_matWorld2 = m_matCenter2 * m_matRot * m_matMove ;
 
 		//m_matChecker1 = matYaw * matPitch ;
 
 // 		vector3 ptTarget ( 0.0f, vBounding1.y / 2.0f, 0.0f ) ;
 // 		ptCenter1.y += ptOfs.y ;
 		//m_Camera.SetTarget ( ptTarget ) ;
 	}
 
 	if ( m_pd3dModel1 || m_pd3dModel2 ) {
 		D3DXMATRIX matTrans ;
 		D3DXMatrixTranslation ( &matTrans, m_ptCheckerPos.x, 0.0f, m_ptCheckerPos.z ) ;
 
 		D3DXMATRIX matYaw ;
 		D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
 		D3DXMATRIX matPitch ;
 		D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;
 
 		m_matChecker1 = matTrans * matYaw * matPitch ;
 		m_matChecker2 = matTrans * matYaw * matPitch ;
 	}
 
 }
