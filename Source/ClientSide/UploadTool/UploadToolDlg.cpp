
// UploadToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UploadTool.h"
#include "UploadToolDlg.h"
#include "afxdialogex.h"
#include "CModelWebServiceClient.h"
#include "GlobalDefines.h"
#include "tlMacros.h"
#include <string>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std ;

#ifndef SAFE_DELETE
/// For pointers allocated with new.
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif

CUploadToolDlg* g_pDlg = NULL ;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUploadToolDlg dialog



CUploadToolDlg::CUploadToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPLOADTOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_pDlg = this ;
}

void CUploadToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUploadToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED ( IDOK, &CUploadToolDlg::OnBnClickedOk )
	ON_BN_CLICKED ( IDC_BTN_MODEL_UPLOAD, &CUploadToolDlg::OnBnClickedBtnModelUpload )
	ON_BN_CLICKED ( IDC_BTN_AD_UPLOAD, &CUploadToolDlg::OnBnClickedBtnAdUpload )
	ON_BN_CLICKED ( IDC_BTN_MODEL_FILE, &CUploadToolDlg::OnBnClickedBtnModelFile )
	ON_BN_CLICKED ( IDC_BTN_CREATE_SUBS, &CUploadToolDlg::OnBnClickedBtnCreateSubs )
	ON_BN_CLICKED ( IDC_BTN_AD_FILE, &CUploadToolDlg::OnBnClickedBtnAdFile )
END_MESSAGE_MAP()


// CUploadToolDlg message handlers

BOOL CUploadToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	FILE* pFile = fopen ( "config.txt", "rt" ) ;
	if ( pFile ) {
		char szServer [ 1000 ] = { 0 } ;
		fscanf ( pFile, "%s", szServer ) ;
		fclose ( pFile ) ;
		wchar_t wszServer [ 1000 ] = { 0 };
		MultiByteToWideChar ( CP_ACP, 0, szServer, strlen ( szServer ), wszServer, 1000 ) ;
		SetDlgItemText ( IDC_EDIT_SERVER, wszServer ) ;
	}

	CModelServiceWebClient::m_hCallbackWnd = GetSafeHwnd() ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUploadToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUploadToolDlg::OnPaint()
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
HCURSOR CUploadToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUploadToolDlg::OnBnClickedOk ()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK ();
}


void MyModelCallback ( bool bRes, int iModelId )
{
	CString str ;
	str.Format ( L"%d", iModelId ) ;
	g_pDlg->SetDlgItemText ( IDC_EDIT_MODEL_ID, str ) ;
// 	g_pDlg->m_PointerPass [ 1 ].pData = pData ;
// 	g_pDlg->m_PointerPass [ 1 ].iSize = iSize ;
// 
// 	g_pDlg->PostMessage ( WM_USER_MODEL_DOWNLOADED, 1, 0 ) ;
}

std::thread UploadModelThread ;

void CUploadToolDlg::OnBnClickedBtnModelUpload ()
{
	char* szUser = new char [ 1000 ] ;
	char* szPass = new char [ 1000 ] ;
	char* szName = new char [ 1000 ] ;
	char* szDesc = new char [ 1000 ] ;

	GetDlgItemAsChar ( IDC_EDIT_USER, szUser ) ;
	GetDlgItemAsChar ( IDC_EDIT_PASS, szPass ) ;
	GetDlgItemAsChar ( IDC_EDIT_MODEL_NAME, szName ) ;
	GetDlgItemAsChar ( IDC_EDIT_MODEL_DESC, szDesc ) ;

	CString strFile ;
	GetDlgItemText ( IDC_EDIT_MODEL_FILE, strFile ) ;

	FILE* pFile = _wfopen ( strFile, L"rb" ) ;
	if ( !pFile ) {
		AfxMessageBox ( L"Failed to open model file!", MB_ICONERROR ) ;
		return ;
	}

	fseek ( pFile, 0, SEEK_END ) ;
	int iSize = ftell ( pFile ) ;
	fseek ( pFile, 0, SEEK_SET ) ;
	char* pData = new char [ iSize ] ;
	fread ( pData, iSize, 1, pFile ) ;
	fclose ( pFile ) ;

	char szClientId[] = { MODEL_CLIENT_ID_PCWIN } ;

	GetDlgItem ( IDC_PROGRESS_MODEL )->ShowWindow ( SW_SHOW ) ;

	CString strServer ;
	GetDlgItemText ( IDC_EDIT_SERVER, strServer ) ;

	web::uri_builder builder_mdl ;
	builder_mdl.set_scheme ( U ( MODEL_SERVICE_SCHEME ) ) ;
	builder_mdl.set_host ( strServer.GetBuffer () ) ;
	builder_mdl.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
	builder_mdl.append_path ( U ( MODEL_API_UPLOAD_MODEL ) ) ;
	builder_mdl.append_query ( L"magic", U ( MODEL_API_MAGIC ) ) ;

	wstring strUrl = builder_mdl.to_string () ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;
	wcscpy ( szUrl, strUrl.c_str () ) ;

	std::function<void ( bool bRes, int iModelId)> myCallback = MyModelCallback ;

	UploadModelThread = std::thread ( [ = ]( wchar_t* pUrl, char* pClientId, char* pszUser, char* pszPass, char* pszName, char* pszDesc, char* pData, int iSize ) {

		int iModelId = -1 ;

		CModelServiceWebClient client ;
		bool bRes = client.UploadModel ( pUrl, pClientId, pszUser, pszPass, pData, iSize, pszName, pszDesc, &iModelId ) ;
		if ( myCallback )
			myCallback ( bRes, iModelId ) ;
 		SAFE_DELETE ( pUrl ) ;
// 		SAFE_DELETE ( pClientId ) ;
 		SAFE_DELETE ( pszUser ) ;
 		SAFE_DELETE ( pszPass ) ;
 		SAFE_DELETE ( pszName ) ;
 		SAFE_DELETE ( pszDesc ) ;
		SAFE_DELETE ( pData ) ;

	}, szUrl, szClientId, szUser, szPass, szName, szDesc, pData, iSize ) ;
	UploadModelThread.detach () ;
}

void MyAdCallback ( bool bRes, int iAdId )
{
	CString str ;
	str.Format ( L"%d", iAdId ) ;
	g_pDlg->SetDlgItemText ( IDC_EDIT_AD_ID, str ) ;
	// 	g_pDlg->m_PointerPass [ 1 ].pData = pData ;
	// 	g_pDlg->m_PointerPass [ 1 ].iSize = iSize ;
	// 
	// 	g_pDlg->PostMessage ( WM_USER_MODEL_DOWNLOADED, 1, 0 ) ;
}

std::thread UploadAdThread ;

void CUploadToolDlg::OnBnClickedBtnAdUpload ()
{
	char* szUser = new char [ 1000 ] ;
	char* szPass = new char [ 1000 ] ;
	char* szAdUrl = new char [ 1000 ] ;

	GetDlgItemAsChar ( IDC_EDIT_USER, szUser ) ;
	GetDlgItemAsChar ( IDC_EDIT_PASS, szPass ) ;
	GetDlgItemAsChar ( IDC_EDIT_AD_URL, szAdUrl ) ;

	CString strFile ;
	GetDlgItemText ( IDC_EDIT_AD_FILE, strFile ) ;

	FILE* pFile = _wfopen ( strFile, L"rb" ) ;
	if ( !pFile ) {
		AfxMessageBox ( L"Failed to open ad file!", MB_ICONERROR ) ;
		return ;
	}

	fseek ( pFile, 0, SEEK_END ) ;
	int iSize = ftell ( pFile ) ;
	fseek ( pFile, 0, SEEK_SET ) ;
	char* pData = new char [ iSize ] ;
	fread ( pData, iSize, 1, pFile ) ;
	fclose ( pFile ) ;

	char szClientId[] = { MODEL_CLIENT_ID_PCWIN } ;

	GetDlgItem ( IDC_PROGRESS_MODEL )->ShowWindow ( SW_SHOW ) ;

	CString strServer ;
	GetDlgItemText ( IDC_EDIT_SERVER, strServer ) ;

	web::uri_builder builder_mdl ;
	builder_mdl.set_scheme ( U ( MODEL_SERVICE_SCHEME ) ) ;
	builder_mdl.set_host ( strServer.GetBuffer () ) ;
	builder_mdl.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
	builder_mdl.append_path ( U ( MODEL_API_UPLOAD_AD ) ) ;
	builder_mdl.append_query ( L"magic", U ( MODEL_API_MAGIC ) ) ;

	wstring strUrl = builder_mdl.to_string () ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;
	wcscpy ( szUrl, strUrl.c_str () ) ;

	std::function<void ( bool bRes, int iModelId )> myCallback = MyAdCallback ;

	UploadAdThread = std::thread ( [ = ]( wchar_t* pUrl, char* pClientId, char* pszUser, char* pszPass, char* pszAdUrl, char* pData, int iSize ) {

		int iAdId = -1 ;

		CModelServiceWebClient client ;
		bool bRes = client.UploadAd ( pUrl, pClientId, pszUser, pszPass, pData, iSize, pszAdUrl, &iAdId ) ;
		if ( myCallback )
			myCallback ( bRes, iAdId ) ;
		SAFE_DELETE ( pUrl ) ;
		// 		SAFE_DELETE ( pClientId ) ;
		SAFE_DELETE ( pszUser ) ;
		SAFE_DELETE ( pszPass ) ;
		SAFE_DELETE ( pszAdUrl ) ;
		SAFE_DELETE ( pData ) ;

	}, szUrl, szClientId, szUser, szPass, szAdUrl, pData, iSize ) ;
	UploadAdThread.detach () ;
}

LRESULT CUploadToolDlg::WindowProc ( UINT message, WPARAM wParam, LPARAM lParam )
{
	

	return CDialogEx::WindowProc ( message, wParam, lParam );
}

void CUploadToolDlg::GetDlgItemAsChar ( UINT id, char* pszValue )
{
	CString str ;
	GetDlgItemText ( id, str ) ;

	int iLen = WideCharToMultiByte ( CP_ACP, 0, str.GetBuffer (), str.GetLength (), pszValue, str.GetLength () + 1, "", NULL ) ;
	pszValue [ iLen ] = 0 ;
}

void CUploadToolDlg::OnBnClickedBtnModelFile ()
{
	wchar_t szFilters[] = L"3D Scan Files (*.3dscan)|*.3dscan||";
	CFileDialog dlg ( TRUE, L"3dscan", L"*.3dscan", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () );
	if ( dlg.DoModal () == IDOK ) {
		SetDlgItemText ( IDC_EDIT_MODEL_FILE, dlg.GetPathName () ) ;
	}
}

void CUploadToolDlg::OnBnClickedBtnCreateSubs()
{
	GetDlgItem ( IDC_EDIT_SUBS_ID )->ShowWindow ( SW_HIDE ) ;
	GetDlgItem ( IDC_STATIC_SUBS )->ShowWindow ( SW_HIDE ) ;

	char szUser [ 1000 ] ;
	char szPass [ 1000 ] ;
	char szOwner [ 1000 ] ;

	GetDlgItemAsChar ( IDC_EDIT_USER, szUser ) ;
	GetDlgItemAsChar ( IDC_EDIT_PASS, szPass ) ;
	GetDlgItemAsChar ( IDC_EDIT_OWNER, szOwner ) ;

	CString s ;

	GetDlgItemText ( IDC_EDIT_MODEL_ID, s ) ;
	int iModelId = _wtol ( s ) ;

	GetDlgItemText ( IDC_EDIT_AD_ID, s ) ;
	int iAdId = _wtol ( s ) ;

	char szClientId[] = { MODEL_CLIENT_ID_PCWIN } ;

	GetDlgItem ( IDC_PROGRESS_MODEL )->ShowWindow ( SW_SHOW ) ;

	CString strServer ;
	GetDlgItemText ( IDC_EDIT_SERVER, strServer ) ;

	web::uri_builder builder_mdl ;
	builder_mdl.set_scheme ( U ( MODEL_SERVICE_SCHEME ) ) ;
	builder_mdl.set_host ( strServer.GetBuffer () ) ;
	builder_mdl.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
	builder_mdl.append_path ( U ( MODEL_API_CREATE_SUBSCRIPTION ) ) ;
	builder_mdl.append_query ( L"magic", U ( MODEL_API_MAGIC ) ) ;

	wstring strUrl = builder_mdl.to_string () ;

	wchar_t* szUrl = new wchar_t [ strUrl.length () + 1 ] ;
	wcscpy ( szUrl, strUrl.c_str () ) ;

	char szHash [ 100 ] ;
	szHash [ 0 ] = 0 ;

	CModelServiceWebClient client ;
	bool bRes = client.CreateSubscription ( szUrl, szClientId, szUser, szPass, szOwner, iModelId, iAdId, szHash ) ;

	if ( bRes ) {
		GetDlgItem ( IDC_EDIT_SUBS_ID )->ShowWindow ( SW_SHOW ) ;
		GetDlgItem ( IDC_STATIC_SUBS )->ShowWindow ( SW_SHOW ) ;
		wchar_t wszHash [ 1000 ] = { 0 };
		MultiByteToWideChar ( CP_ACP, 0, szHash, strlen ( szHash ), wszHash, 1000 ) ;
		SetDlgItemText ( IDC_EDIT_SUBS_ID, wszHash ) ;
	}

}

void CUploadToolDlg::OnBnClickedBtnAdFile ()
{
	wchar_t szFilters[] = L"Image Files (*.*)|*.*||";
	CFileDialog dlg ( TRUE, L"*", L"*.*", OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, AfxGetMainWnd () );
	if ( dlg.DoModal () == IDOK ) {
		SetDlgItemText ( IDC_EDIT_AD_FILE, dlg.GetPathName () ) ;
	}
}
