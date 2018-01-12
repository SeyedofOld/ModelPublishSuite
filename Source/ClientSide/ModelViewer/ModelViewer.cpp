
// ModelViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ModelViewer.h"
#include "ModelViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CModelViewerApp

BEGIN_MESSAGE_MAP(CModelViewerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CModelViewerApp construction

CModelViewerApp::CModelViewerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CModelViewerApp object

CModelViewerApp theApp;


// CModelViewerApp initialization

BOOL CModelViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	////////////////////////////////
	// Parse command line for standard shell commands, DDE, file open
// 	CCommandLineInfo cmdInfo;
// 	ParseCommandLine ( cmdInfo );

	// Enable DDE Execute open
	EnableShellOpen ();
	//RegisterShellFileTypes ( TRUE );


	CStringW strCmdLine = GetCommandLineW() ;
	int iParamCount = 0 ;
	LPWSTR* pStr = CommandLineToArgvW ( strCmdLine, &iParamCount ) ;

	bool bIsUrl = false ;
	bool bIsFile = false ;

	int iUrlParamIndex = 0 ;

	for ( int i = 0 ; i < iParamCount ; i++ ) {
		if ( wcscmp ( pStr [ i ], L"-url" ) == 0 ) {
			iUrlParamIndex = i ;
			bIsUrl = true ;
		}
	}

	if ( ! bIsUrl && iParamCount > 1 )
		bIsFile = true ;

// 	if ( bIsUrl ) {
// 		MessageBoxW ( NULL, L"IsUrl", L"A", MB_ICONEXCLAMATION ) ;
// 		MessageBoxW ( NULL, (wchar_t*)(pStr [ 1 ]), L"A", MB_ICONEXCLAMATION ) ;
// 	}

	{ // Set current directory to where the exe exists
		wchar_t g_szExeName [ MAX_PATH ] ;
		wcscpy ( g_szExeName, pStr [ 0 ] ) ;

		wchar_t szDrive [ MAX_PATH ], szDir [ MAX_PATH ]  ;
		_wsplitpath ( g_szExeName, szDrive, szDir, NULL, NULL ) ;

		wchar_t szFolder [ MAX_PATH ] ;
		wcscpy ( szFolder, szDrive ) ;
		wcscat ( szFolder, szDir ) ;

		SetCurrentDirectoryW ( szFolder ) ;
	}

	//AfxMessageBox ( "1" ) ;

// 	if ( pStr )
// 		delete ( pStr ) ;

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
// 	if ( !ProcessShellCommand ( cmdInfo ) )
// 		return FALSE;
	////////////////////////////////


	//CModelViewerDlg dlg;
	m_pDlg = new CModelViewerDlg ;
	m_pDlg->Create ( IDD_MODELVIEWER_DIALOG ) ;
	m_pMainWnd = m_pDlg;
	m_pMainWnd->ShowWindow ( SW_SHOW ) ;

	if ( bIsFile ) {
		char* pszAnsi = new char [ 1000 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, pStr [ 1 ], wcslen ( pStr [ 1 ] ), pszAnsi, 1000, pszAnsi, NULL );
		pszAnsi [ iLen ] = 0 ;

		//MessageBox ( NULL, pszAnsi, "File", MB_OK ) ;

		m_pDlg->Load3DScanFile ( (CString)pszAnsi ) ;
		if ( pszAnsi )
			delete pszAnsi ;
	}
	else if ( bIsUrl ) {
		char* pszAnsi = new char [ 1000 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, pStr [ 1 ], wcslen ( pStr [ 1 ] ), pszAnsi, 1000, pszAnsi, NULL );
		pszAnsi [ iLen ] = 0 ;

		//MessageBox ( NULL, pszAnsi, "Url", MB_OK ) ;

		m_pDlg->Load3DScanFromUrl ( (CString)pszAnsi ) ;
		if ( pszAnsi )
			delete pszAnsi ;
	}

	//AfxMessageBox ( "3" ) ;

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return TRUE;
}

BOOL CModelViewerApp::OnIdle ( LONG lCount )
{
	// TODO: Add your specialized code here and/or call the base class
	m_pDlg->Update() ;
	m_pDlg->Render() ;

	return TRUE ;
	//return CWinApp::OnIdle ( lCount );
}
