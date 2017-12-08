
// ModelViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ModelViewer.h"
#include "ModelViewerDlg.h"
#include "afxdialogex.h"
#include "tlC3DGfx.h"
#include "Imgui/imgui_internal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CModelViewerDlg dialog



CModelViewerDlg::CModelViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MODELVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CModelViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CModelViewerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED ( IDCANCEL, &CModelViewerDlg::OnBnClickedCancel )
	ON_BN_CLICKED ( IDOK, &CModelViewerDlg::OnBnClickedOk )
	ON_WM_SIZE ()
	ON_WM_ERASEBKGND ()
END_MESSAGE_MAP()


// CModelViewerDlg message handlers
class CMyEffectInclude2 : public ID3DXInclude
{
public:

	CMyEffectInclude2 () : ID3DXInclude ()
	{
		//assert ( pResMan ) ;
		//m_pResMan = pResMan ;
		m_szIncludePath [ 0 ] = 0 ;
	}
	~CMyEffectInclude2 ()
	{
	}
	STDMETHOD ( Open )( THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes )
	{
		char szFile [ MAX_PATH ] ;
		sprintf_s ( szFile, MAX_PATH, "%s%s", m_szIncludePath, pFileName ) ;

		FILE* pFile ;
		fopen_s ( &pFile, szFile, "rb" ) ;
		fseek ( pFile, 0, SEEK_END ) ;
		*pBytes = ftell ( pFile ) ;

		*ppData = new BYTE [ *pBytes ] ;

		fseek ( pFile, 0, SEEK_SET ) ;
		fread ( (void*)*ppData, 1, *pBytes, pFile ) ;

		fclose ( pFile ) ;

		// 		DWORD dwSize ;
		// 
		// 		BYTE* pData = m_pResMan->LoadDataFile ( szFile , &dwSize ) ;
		// 		if ( !pData ) 
		// 		{
		// 			return E_FAIL ;
		// 		}
		// 		*ppData = pData ;
		// 		*pBytes = dwSize ;
		return S_OK ;
	}
	STDMETHOD ( Close )( THIS_ LPCVOID pData )
	{
		//Sm_pResMan->ReleaseDataFile ( (BYTE*)pData ) ;
		if ( pData )
			delete pData ;
		pData = NULL ;
		return S_OK ;
	}
	void SetIncludePath ( char* pszIncludePath )
	{
		if ( !pszIncludePath )
			m_szIncludePath [ 0 ] = 0 ;
		else
			strcpy_s ( m_szIncludePath, MAX_PATH, pszIncludePath ) ;
	}
private:
	char m_szIncludePath [ MAX_PATH ] ;
};


BOOL CModelViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rc ;
	GetClientRect ( rc ) ;

	C3DGfx::CreateInstance () ;
	C3DGfx::GetInstance ()->Initialize ( GetSafeHwnd (), rc.Width(), rc.Height(), D3DFMT_A8R8G8B8, FALSE, 0, FALSE );


	m_Camera.Initialize ( D3DXToRadian ( 45.0f ), 4.0f / 3.0f, 0.01f, 1000.0f );
	m_Camera.SetMode ( CCamera::MODE_TARGET );
	m_Camera.SetPosition ( 0.0f, 0.0f, -10.0f );
	m_Camera.SetTarget ( 0.0f, 0.0f, 0.0f );

	CMyEffectInclude2 EffectInclude ;

	HRESULT hr = D3DXCreateEffectFromFileA ( C3DGfx::GetInstance ()->GetDevice (),
		"DiffuseMapSpec_trans.fx",
		NULL,
		&EffectInclude,
		0,
		C3DGfx::GetInstance ()->GetEffectPool (),
		&m_pShader,
		NULL ) ;


	CGuiRenderer::Initialize ( C3DGfx::GetInstance()->GetDevice(), rc.Width(), rc.Height() ) ;

	m_SettingsGui.Initialize() ;
	//CGuiRenderer::Update ( 0.01f ) ;
	D3DVIEWPORT9 vp ;
	vp.Width = rc.Width() ;
	vp.Height = rc.Height() ;
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


	SetWindowPos ( NULL, 0, 0, 1200, 675, SWP_NOMOVE ) ;

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
}

static void ShowExampleMenuFile ()
{
	ImGui::MenuItem ( "(dummy menu)", NULL, false, false );
	if ( ImGui::MenuItem ( "New" ) ) {}
	if ( ImGui::MenuItem ( "Open", "Ctrl+O" ) ) {
		wchar_t szFilters[] = L"3D Scan Files (*.3dscan)|*.3dscan||";
		CFileDialog dlg ( TRUE, L"3dscan", L"*.3dscan", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
		if ( dlg.DoModal () != IDOK )
			return ;
	}
	if ( ImGui::BeginMenu ( "Open Recent" ) )
	{
		ImGui::MenuItem ( "fish_hat.c" );
		ImGui::MenuItem ( "fish_hat.inl" );
		ImGui::MenuItem ( "fish_hat.h" );
		if ( ImGui::BeginMenu ( "More.." ) )
		{
			ImGui::MenuItem ( "Hello" );
			ImGui::MenuItem ( "Sailor" );
			if ( ImGui::BeginMenu ( "Recurse.." ) )
			{
				ShowExampleMenuFile ();
				ImGui::EndMenu ();
			}
			ImGui::EndMenu ();
		}
		ImGui::EndMenu ();
	}
	if ( ImGui::MenuItem ( "Save", "Ctrl+S" ) ) {}
	if ( ImGui::MenuItem ( "Save As.." ) ) {}
	ImGui::Separator ();
	if ( ImGui::BeginMenu ( "Options" ) )
	{
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
	if ( ImGui::BeginMenu ( "Colors" ) )
	{
		ImGui::PushStyleVar ( ImGuiStyleVar_FramePadding, ImVec2 ( 0, 0 ) );
		for ( int i = 0; i < ImGuiCol_COUNT; i++ )
		{
			//const char* name = ImGui::GetStyleColorName ( (ImGuiCol)i );
			//ImGui::ColorButton ( name, ImGui::GetStyleColorVec4 ( (ImGuiCol)i ) );
			ImGui::SameLine ();
			//ImGui::MenuItem ( name );
		}
		ImGui::PopStyleVar ();
		ImGui::EndMenu ();
	}
	if ( ImGui::BeginMenu ( "Disabled", false ) ) // Disabled
	{
		IM_ASSERT ( 0 );
	}
	if ( ImGui::MenuItem ( "Checked", NULL, true ) ) {}
	if ( ImGui::MenuItem ( "Quit", "Alt+F4" ) ) {
		PostQuitMessage ( 0 ) ;
	}
}

void CModelViewerDlg::Render()
{
	bool bDualView = false ;
	bool bSingleView = true ;

	C3DGfx::GetInstance ()->BeginFrame ();
	//C3DGfx::GetInstance ()->Clear ( 0xff200040 ) ;

	m_pView->SelectView() ;

/*
	//C3DGfx::GetInstance ()->GetDevice ()->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	IDirect3DDevice9* pDevice = C3DGfx::GetInstance ()->GetDevice () ;


	// 		shared float4x4 g_matSunLight : SUNLIGHT ;		// Sun Light characteristics
	// 														// Row1 : Direction
	// 														// Row2 : Diffuse
	// 														// Row3 : Ambient
	// 														// Row4 : Reserved
	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix () ) ;
	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;
	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matWorld", &m_matWorld ) ;

	matrix matLight ;

	vector4 vLightDir ( 0.0f, 0.0f, 1.0f, 0.0f ) ;
	D3DXVec4Normalize ( &vLightDir, &vLightDir ) ;
	CopyMemory ( &matLight [ 0 ], &vLightDir, 4 * sizeof ( float ) ) ;

	vector4 vLightColor ( 1.0f, 1.0f, 1.0f, 0.0f ) ;
	CopyMemory ( &matLight [ 4 ], &vLightColor, 4 * sizeof ( float ) ) ;

	vector4 vAmbLight ( 0.5f, 0.5f, 0.5f, 0.0f ) ;
	CopyMemory ( &matLight [ 8 ], &vAmbLight, 4 * sizeof ( float ) ) ;

	( (CMainFrame*)AfxGetMainWnd () )->m_pShader->SetMatrix ( "g_matSunLight", &matLight ) ;

	if ( bDualView ) {
		D3DVIEWPORT9 vp = C3DGfx::GetInstance ()->GetFullscreenViewport () ;
		vp.Width /= 2 ;
		vp.X = 0 ;
		pDevice->SetViewport ( &vp ) ;
		CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh1 ) ;

		vp = C3DGfx::GetInstance ()->GetFullscreenViewport () ;
		vp.Width /= 2 ;
		vp.X = vp.Width ;
		pDevice->SetViewport ( &vp ) ;
		CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh2 ) ;

		pDevice->SetViewport ( &C3DGfx::GetInstance ()->GetFullscreenViewport () ) ;
	}
	else if ( bSingleView ) {
		pDevice->SetViewport ( &C3DGfx::GetInstance ()->GetFullscreenViewport () ) ;
		CD3DMesh2::RenderD3DMesh ( pDevice, GetDocument ()->m_d3dMesh1 ) ;
	}
	else
		if ( m_pMesh )
			m_pMesh->DrawSubset ( 0 );
		*/
	CGuiRenderer::Update ( 0.01f ) ;
	
	//m_SettingsGui.Update () ;

	{
		CRect rc ;
		GetClientRect ( rc ) ;

		ImGuiWindowFlags flags = 0;
		flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_NoResize;
		flags |= ImGuiWindowFlags_NoCollapse;
		flags |= ImGuiWindowFlags_MenuBar;
		//flags |= ImGuiWindowFlags_ShowBorders;

        //ImGui::SetStyleColor

		//CRect rcOld = rc ;

		if ( ImGui::Begin ( "Model Viewer", NULL, ImVec2 ( rc.Width(), rc.Height() ), 1.0f, flags ) ) {
			ImGui::SetWindowPos ( ImVec2 ( 0, 0 ) ) ;
			ImGuiStyle& style = ImGui::GetStyle ();
			style.WindowRounding = 0.0f ;
			style.FrameBorderSize = 1.0f ;

			// 		ImGui::Checkbox("Show FPS", &pSettings->bShowFPS);
			// 		ImGui::Checkbox("Show Sky", &pEngSettings->Sky.bShowSky);
			// 		ImGui::Checkbox("Show Terrain", &pEngSettings->Terrain.bShowTerrain);
			// 		ImGui::Checkbox("Show Foliage", &pEngSettings->Foliage.bShowFoliage);
			// 		ImGui::Checkbox("Show Objects", &pEngSettings->Objects.bShowObjects);
			// 		ImGui::Checkbox("Show Physics", &pEngSettings->Physics.bShowPhysicsModels);
			// 		ImGui::Checkbox("Show Scene Nodes", &pEngSettings->Scene.bShowSceneNodes);
			// 		ImGui::Checkbox("Show Object Bounding", &pEngSettings->Objects.bShowBoundings);
			// 		ImGui::Checkbox("Show Object Path", &pEngSettings->Objects.bShowPath);
			bool s_b = false ;
			ImGui::Checkbox ( "Wire-frame", &s_b );
			ImGui::PushItemWidth ( 120.0f );
			//ImGui::InputFloat ( "Transparency", &m_fAlpha, 0.01f, 0.1f, 2 );
			ImGui::PopItemWidth ();
			//VALIDATE_RANGE ( m_fAlpha, 0.1f, 1.0f );
			int w = ImGui::GetWindowSize ().x ;
			int h = ImGui::GetWindowSize ().y ;
			int x = ImGui::GetWindowPos ().x ;
			int y = ImGui::GetWindowPos ().y ;
			ImGui::End ();

			if ( w != rc.Width () || h != rc.Height () ) {
				//MoveWindow ( rc.left, rc.top, w, h ) ;
			}

			if ( 0 )
			if ( x != 0 || y != 0 ) {
				CRect rcWnd ;
				GetWindowRect ( rcWnd ) ;
				MoveWindow ( rcWnd.left + x, rc.top + y, w, h ) ;
			}
		}


	}

	if ( 1 ) {
		if ( ImGui::BeginMainMenuBar () )
		{
			if ( ImGui::BeginMenu ( "File" ) )
			{
				ShowExampleMenuFile ();
				ImGui::EndMenu ();
			}

			if ( ImGui::BeginMenu ( "Edit" ) )
			{
				if ( ImGui::MenuItem ( "Undo", "CTRL+Z" ) ) {}
				if ( ImGui::MenuItem ( "Redo", "CTRL+Y", false, false ) ) {}  // Disabled item
				ImGui::Separator ();
				if ( ImGui::MenuItem ( "Cut", "CTRL+X" ) ) {}
				if ( ImGui::MenuItem ( "Copy", "CTRL+C" ) ) {}
				if ( ImGui::MenuItem ( "Paste", "CTRL+V" ) ) {}
				static bool s_b = true ;
				ImGui::Checkbox ( "Wire-frame", &s_b );
				ImGui::EndMenu ();
			}

			ImGui::EndMainMenuBar ();
		}
	}

	CGuiRenderer::Render () ;
	
	C3DGfx::GetInstance ()->EndFrame ();
	m_pView->ShowFrame( ) ;
}


void CModelViewerDlg::OnSize ( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize ( nType, cx, cy );

	// TODO: Add your message handler code here
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
// 		if ( !m_pMesh )
// 			D3DXCreateTeapot ( C3DGfx::GetInstance ()->GetDevice (), &m_pMesh, NULL ) ;
	}
}

using namespace ImGui ;

//extern struct ImGuiState*             GImGui ;

LRESULT CModelViewerDlg::WindowProc ( UINT message, WPARAM wParam, LPARAM lParam )
{
	// TODO: Add your specialized code here and/or call the base class
	if ( CGuiRenderer::s_pDevice )
		CGuiRenderer::WndProc ( GetSafeHwnd(), message, wParam, lParam ) ;

// 	if ( 0 )
// 	if ( CGuiRenderer::s_pDevice ) {
// 		
// 		struct ImGuiState* st = ( struct ImGuiState*)ImGui::GetState () ;
// 		struct ImGuiWindow* window = st->MovedWindow ;
// 		if ( window ) {
// 			int x = window->Pos.x ;
// 			int y = window->Pos.y ;
// 			if ( x != 0 || y != 0 ) {
// 				CRect rcWnd ;
// 				GetWindowRect ( rcWnd ) ;
// 				MoveWindow ( rcWnd.left + x, rcWnd.top + y, rcWnd.Width (), rcWnd.Height () ) ;
// 			}
// 		}
// 
// 	}


	return CDialogEx::WindowProc ( message, wParam, lParam );
}


BOOL CModelViewerDlg::OnEraseBkgnd ( CDC* pDC )
{
	// TODO: Add your message handler code here and/or call default
	//return TRUE ;
	pDC->SelectObject ( GetStockObject ( NULL_BRUSH ) ) ;
	return TRUE ;

	return CDialogEx::OnEraseBkgnd ( pDC );
}
