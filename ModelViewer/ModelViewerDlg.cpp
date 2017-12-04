
// ModelViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ModelViewer.h"
#include "ModelViewerDlg.h"
#include "afxdialogex.h"
#include "tlC3DGfx.h"

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


	//uiRenderer::Initialize ( C3DGfx::GetInstance()->GetDevice(), rc.Width(), rc.Height() ) ;

	//SettingsGui.Initialize() ;
	//uiRenderer::Update ( 0.01f ) ;

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

void CModelViewerDlg::Render()
{
	bool bDualView = false ;
	bool bSingleView = true ;

	C3DGfx::GetInstance ()->BeginFrame ();
	C3DGfx::GetInstance ()->Clear ( 0xff200040 ) ;

/*
	m_Camera.SetD3DCamera ( C3DGfx::GetInstance ()->GetDevice () ) ;

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
	//CGuiRenderer::Update ( 0.01f ) ;
	
	//m_SettingsGui.Update () ;
	//CGuiRenderer::Render () ;
	
	C3DGfx::GetInstance ()->EndFrame ();
	C3DGfx::GetInstance ()->ShowFrame ( NULL, NULL, GetSafeHwnd () );

	//C3DGfx::GetInstance()->Resize ( 400, 300 ) ;
}


void CModelViewerDlg::OnSize ( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize ( nType, cx, cy );

	// TODO: Add your message handler code here
	if ( C3DGfx::GetInstance () && C3DGfx::GetInstance ()->IsInitialized () ) {
		if ( cx && cy ) {
			C3DGfx::GetInstance ()->Resize ( cx, cy ) ;
			m_Camera.SetAspect ( (float)cx / cy ) ;

			ImGui::GetIO ().DisplaySize.x = (float)cx ;
			ImGui::GetIO ().DisplaySize.y = (float)cy ;

		}
// 		if ( !m_pMesh )
// 			D3DXCreateTeapot ( C3DGfx::GetInstance ()->GetDevice (), &m_pMesh, NULL ) ;
	}
}

LRESULT CModelViewerDlg::WindowProc ( UINT message, WPARAM wParam, LPARAM lParam )
{
	// TODO: Add your specialized code here and/or call the base class
	if ( CGuiRenderer::s_pDevice )
		CGuiRenderer::WndProc ( GetSafeHwnd(), message, wParam, lParam ) ;

	return CDialogEx::WindowProc ( message, wParam, lParam );
}
