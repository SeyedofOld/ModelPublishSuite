
// ModelViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ModelEditor.h"
#include "ModelEditorDlg.h"
#include "afxdialogex.h"
#include "tlC3DGfx.h"
#include "Imgui/imgui_internal.h"
#include "ObjLoader.h"
#include "C3DScanFile.h"
#include "Obj2Model.h"
#include "CD3DModelUtils.h"
#include "C3DModelUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CModelViewerDlg dialog


CModelViewerDlg::CModelViewerDlg(CWnd* pParent /*=NULL*/)
	: CRenderDialog(IDD_MODELEDITOR_DIALOG, pParent) 
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bFileOpened = false ;
	m_bHasFilename = false ;
	m_pd3dModel1 = NULL ;
	m_pModel1 = NULL ;
	D3DXMatrixIdentity ( &m_matWorld ) ;
	m_fYaw = 0.0f ;
	m_fPitch = 0.0f ;
	m_ptPos = vector3 ( 0, 0, 0 ) ;

	m_clrClear = float4_rgba{ 0.2f, 0.2f, 0.2f, 0.0f } ;
	m_clrLight = float4_rgba { 1.0f, 1.0f, 1.0f, 1.0f } ;

	m_ppszTextureNames = NULL ;
	m_iTextureCount = 0 ;
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

	CMyEffectInclude EffectInclude ;

	HRESULT hr = D3DXCreateEffectFromFileA ( C3DGfx::GetInstance ()->GetDevice (),
		"DiffuseMapSpec_trans.fx",
		NULL,
		&EffectInclude,
		0,
		C3DGfx::GetInstance ()->GetEffectPool (),
		&m_pShader,
		NULL ) ;


	CGuiRenderer::Initialize ( C3DGfx::GetInstance()->GetDevice(), rc.Width(), rc.Height() ) ;

	//m_SettingsGui.Initialize() ;
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

// 	CMyEffectInclude EffectInclude ;
// 
// 	D3DXCreateEffectFromFileA ( C3DGfx::GetInstance ()->GetDevice (), "DiffuseMapSpec_trans.fx", NULL, &EffectInclude, 0, NULL, &m_pShader, NULL ) ;

	//SetWindowPos ( NULL, 0, 0, 1200, 675, SWP_NOMOVE ) ;

	ModifyStyleEx ( 0, SS_NOTIFY ) ;

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
			TD_SCAN_MODEL* pModel = C3DScanFile::Load3DScanModel ( dlg.GetPathName().GetBuffer() ) ;
			if ( pModel ) {
				m_pModel1 = pModel ;
				m_pd3dModel1 = new D3D_MODEL ;
				if ( ! CD3DModelUtils::CreateFromTDModel ( C3DGfx::GetInstance ()->GetDevice (), C3DGfx::GetInstance ()->GetEffectPool (), *m_pModel1, *m_pd3dModel1 ) ) {
					delete m_pd3dModel1 ;
					m_pd3dModel1 = NULL ;
					delete m_pModel1 ;
					m_pModel1 = NULL ;
				}
				else {
					FillTextureList () ;
					m_strFilename = dlg.GetPathName () ;
					m_bFileOpened = true ;
					m_bHasFilename = true ;
				}
			}
		}
	}
	
	if ( ImGui::BeginMenu ( "Open Recent" ) ) {
		ImGui::MenuItem ( "fish_hat.c" );
		ImGui::MenuItem ( "fish_hat.inl" );
		ImGui::MenuItem ( "fish_hat.h" );
		ImGui::EndMenu ();
	}
	
	if ( ImGui::MenuItem ( "Save 3D Scan File", "Ctrl+S", false, m_bFileOpened ) ) {
		if ( m_bFileOpened && m_pModel1 ) {
			if ( ! m_bHasFilename ) {
				char szFilters[] = "3D Scan Files (*.3dscan)|*.3dscan||";
				CFileDialog dlg ( FALSE, "3dscan", "*.3dscan", OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
				if ( dlg.DoModal () == IDOK ) {
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
	
	if ( ImGui::MenuItem ( "Save 3D Scan File As...", "", false, m_bFileOpened ) ) {
		if ( m_bFileOpened && m_pModel1 ) {
			char szFilters[] = "3D Scan Files (*.3dscan)|*.3dscan||";
			CFileDialog dlg ( FALSE, "3dscan", "*.3dscan", OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () ) ;
			if ( dlg.DoModal () == IDOK ) {
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
	bool bDualView = false ;
	bool bSingleView = true ;

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
	

	/*if ( bDualView ) {
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

	//CGuiRenderer::Render () ;

	m_pShader->SetMatrix ( "g_matView", &m_Camera.GetViewMatrix () ) ;
	m_pShader->SetMatrix ( "g_matProj", &m_Camera.GetProjectionMatrix () ) ;
	m_pShader->SetMatrix ( "g_matWorld", &m_matWorld ) ;

	matrix matLight ;

	vector4 vLightDir ( 0.0f, 0.0f, 1.0f, 0.0f ) ;
	D3DXVec4Normalize ( &vLightDir, &vLightDir ) ;
	CopyMemory ( &matLight [ 0 ], &vLightDir, 4 * sizeof ( float ) ) ;

	//vector4 vLightColor ( 1.0f, 1.0f, 1.0f, 0.0f ) ;
	CopyMemory ( &matLight [ 4 ], &m_clrLight, 4 * sizeof ( float ) ) ;

	vector4 vAmbLight ( 0.5f, 0.5f, 0.5f, 0.0f ) ;
	CopyMemory ( &matLight [ 8 ], &vAmbLight, 4 * sizeof ( float ) ) ;

	m_pShader->SetMatrix ( "g_matSunLight", &matLight ) ;

	if ( m_pd3dModel1 ) {
		CD3DModelUtils::RenderD3DModel ( pDevice, *m_pd3dModel1 ) ;
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
// 		if ( !m_pMesh )
// 			D3DXCreateTeapot ( C3DGfx::GetInstance ()->GetDevice (), &m_pMesh, NULL ) ;
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
				m_fYaw -= fDeltaX ;
				m_fPitch -= fDeltaY ;

				UpdateWorldMatrix () ;
			}
			
			if ( imd.ButtonStatus.bRButton ) {
				float fDelta = - fDeltaY * 2.0f ;
				m_Camera.SetPosition ( m_Camera.GetPosition () + m_Camera.GetDirection () * fDelta ) ;
			}

			if ( imd.ButtonStatus.bMButton ) {
				D3DXMATRIX matView = m_Camera.GetViewMatrix() ;
				float fScale = 1.0f ;
				D3DXVECTOR3 vUp = D3DXVECTOR3 ( matView._21, matView._22, matView._23 ) ;
				D3DXVECTOR3 vRight = D3DXVECTOR3 ( matView._11, matView._12, matView._13 ) ;

				m_ptPos += fDeltaX * vRight * fScale - fDeltaY * vUp * fScale ;

				UpdateWorldMatrix () ;
			}

			s_ptStart = imd.ptCursor ;
		}
		else if ( imd.eEvent == MOUSE_DRAG_END ) {

		}
		else if ( imd.eEvent == MOUSE_LBDOWN ) {
			if ( m_pModel1 ) {
				CPoint ptScreen = RenderPortToScreenPixel ( imd.ptCursor ) ;
				D3DXVECTOR3 vDir, ptPos ;

				CRect rc ;
				GetClientRect ( rc ) ;

				m_Camera.GetRayFromScreen ( ptScreen, &vDir, (float)rc.Width (), (float)rc.Height () ) ;

				D3DXMATRIX matInvWorld ;
				D3DXMatrixInverse ( &matInvWorld, NULL, &m_matWorld ) ;

				D3DXVec3TransformNormal ( &vDir, &vDir, &matInvWorld ) ;

				ptPos = m_Camera.GetPosition () ;
				D3DXVec3TransformCoord ( &ptPos, &ptPos, &matInvWorld ) ;

				float3 ptHit ;
				D3DMODEL_SUBSET* pSubset = NULL ;
				if ( CD3DModelUtils::IntersectRay ( float3{ ptPos.x, ptPos.y, ptPos.z }, float3{ vDir.x, vDir.y, vDir.z }, *m_pd3dModel1, &ptHit, &pSubset ) ) {
					pSubset->bSelected = ! pSubset->bSelected ;
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

	if ( ! ImGui::Begin ( "Hierarchy", NULL, flags ) ) {
		ImGui::End ();
	}
	else /*( ! ImGui::Begin ( "Hierarchy", &opn, flags ) )*/ {
		ImGuiStyle& style = ImGui::GetStyle ();
		//style.WindowRounding = 0.0f ;
		style.FrameBorderSize = 1.0f ;

		bool b = false ;
		ImGui::Checkbox("Show FPS", &b);
		//ImGui::PushItemWidth ( 120.0f );
		//ImGui::InputFloat ( "Transparency", &m_fAlpha, 0.01f, 0.1f, 2 );
		//ImGui::PopItemWidth ();
		//VALIDATE_RANGE ( m_fAlpha, 0.1f, 1.0f );
		ImGui::End ();
	}

	//SetNextWindowSize ( ImVec2 ( 300, 400 ), ImGuiCond_FirstUseEver );
	static bool s_b = false ;
	if ( ImGui::Begin ( "Object Inspector", NULL, ImVec2(200,100)) ) {
		
		if ( m_pd3dModel1 )
		for ( uint32_t iPart = 0 ; iPart < m_pd3dModel1->Parts.size () ; iPart++ ) {

			D3D_MODEL& model = *m_pd3dModel1 ;

			D3DMODEL_PART& part = model.Parts [ iPart ] ;
			ImGui::Checkbox( part.pBase->sName.c_str(), &part.bVisible );

			ImGui::Indent ( 10.0f ) ;
			for ( uint32_t iSubset = 0 ; iSubset < model.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
				D3DMODEL_SUBSET& subset = model.Parts [ iPart ].Subsets [ iSubset ] ;

				bool bSelected = subset.bSelected ;
				if ( bSelected ) 
					ImGui::PushStyleColor ( ImGuiCol_Text, ImVec4 ( 1.0f, 1.0f, 0.2f, 1.0f ) ) ;

				int iIndex = 1000 + iPart * 100 + iSubset ;
				ImGui::PushID ( iIndex ) ;

				CString s ;
				s.Format ( "Subset %d", iSubset ) ;
				ImGui::Checkbox ( s.GetBuffer(), &subset.bVisible );

				ImGui::SameLine () ;
				s.Format ( "Selected %d", iSubset ) ;
				ImGui::Checkbox ( s.GetBuffer (), &subset.bSelected );

				ImGui::Indent ( 10.0f ) ;
				ImGui::Text ( "Material" );

				ImGui::Indent ( 5.0f ) ;
				ImGui::ColorEdit3 ( "Diffuse Color", (float*)&subset.Material.clrDiffuse ) ;
				ImGui::ColorEdit3 ( "Ambient Color", (float*)&subset.Material.clrAmbient ) ;
				ImGui::SliderFloat ( "Glossiness", &subset.Material.fGlossiness, 0.0f, 1000.0f ) ;
				ImGui::SliderFloat ( "Specular Intensity", &subset.Material.fSpecIntensity, 0.0f, 2.0f ) ;
				ImGui::SliderFloat ( "Transparency", &subset.Material.fTransparency, 0.0f, 1.0f ) ;
				ImGui::SliderFloat ( "Reflection", &subset.Material.fReflectionFactor, 0.0f, 1.0f ) ;

				if ( ImGui::Combo ( "Diffuse Texture", &subset.Material.iDiffuseTex, m_ppszTextureNames, m_iTextureCount ) ) {
					subset.Material.sDiffuseTextureName = m_ppszTextureNames [ subset.Material.iDiffuseTex ] ;
				}
				if ( ImGui::Combo ( "Alpha Texture", &subset.Material.iAlphaTex, m_ppszTextureNames, m_iTextureCount ) ) {
					subset.Material.sAlphaTextureName = m_ppszTextureNames [ subset.Material.iAlphaTex ] ;
				}
				if ( ImGui::Combo ( "Normal Texture", &subset.Material.iNormalTex, m_ppszTextureNames, m_iTextureCount ) ) {
					subset.Material.sNormalTextureName = m_ppszTextureNames [ subset.Material.iNormalTex ] ;
				}
				if ( ImGui::Combo ( "Specular Texture", &subset.Material.iSpecTex, m_ppszTextureNames, m_iTextureCount ) ) {
					subset.Material.sSpecularTextureName = m_ppszTextureNames [ subset.Material.iSpecTex ] ;
				}
				if ( ImGui::Combo ( "Reflection Texture", &subset.Material.iReflTex, m_ppszTextureNames, m_iTextureCount ) ) {
					subset.Material.sReflectionTextureName = m_ppszTextureNames [ subset.Material.iReflTex ] ;
				}

				ImGui::Unindent ( 5.0f ) ;
				ImGui::PopID () ;

				ImGui::Separator() ;

				ImGui::Unindent ( 10.0f ) ;

				if ( bSelected )
					ImGui::PopStyleColor() ;

			}
			ImGui::Separator () ;
			ImGui::Unindent ( 10.0f ) ;

		}

		ImGui::End ();
	}

	ImGui::EndFrame () ;
}

void CModelViewerDlg::UpdateWorldMatrix ()
{
	D3DXMATRIX matYaw ;
	D3DXMatrixRotationY ( &matYaw, m_fYaw ) ;
	D3DXMATRIX matPitch ;
	D3DXMatrixRotationX ( &matPitch, m_fPitch ) ;

	D3DXMATRIX matTrans ;
	D3DXMatrixTranslation ( &matTrans, m_ptPos.x, m_ptPos.y, m_ptPos.z ) ;

	m_matWorld = matYaw * matPitch * matTrans ;
}

void CModelViewerDlg::FillTextureList ()
{
	if ( m_pd3dModel1 ) {
		if ( m_ppszTextureNames ) {
			for ( int i = 0 ; i < m_iTextureCount ; i++ ) {
				char* p = *m_ppszTextureNames ;
				delete p ;
			}
			delete m_ppszTextureNames ;
			m_ppszTextureNames = NULL ;
		}

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
		io.MousePos.x = (signed short)( lParam );
		io.MousePos.y = (signed short)( lParam >> 16 );
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
