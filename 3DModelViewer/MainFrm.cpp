
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "3DModelViewer.h"

#include "MainFrm.h"
#include "tlC3DGfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND ( ID_OPEN_SECOND, &CMainFrame::OnOpenSecond )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

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

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// set the visual manager used to draw all user interface elements
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	C3DGfx::CreateInstance() ;
	C3DGfx::GetInstance ()->Initialize ( GetSafeHwnd (), 100, 100, D3DFMT_A8R8G8B8, FALSE, 0, FALSE );

	CMyEffectInclude2 EffectInclude ;

	HRESULT hr = D3DXCreateEffectFromFileA ( C3DGfx::GetInstance()->GetDevice(),
		"DiffuseMapSpec_trans.fx",
		NULL,
		&EffectInclude,
		0,
		C3DGfx::GetInstance()->GetEffectPool(),
		&m_pShader,
		NULL ) ;

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnOpenSecond ()
{
	// TODO: Add your command handler code here
	int mm = 1 ;
}
