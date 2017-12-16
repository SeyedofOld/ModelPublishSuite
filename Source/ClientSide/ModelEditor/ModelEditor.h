
// ModelViewer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// CModelViewerApp:
// See ModelViewer.cpp for the implementation of this class
//
class CModelViewerDlg ;

class CModelViewerApp : public CWinApp
{
public:
	CModelViewerApp();

// Overrides
public:
	virtual BOOL InitInstance();
	CModelViewerDlg* m_pDlg ;

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle ( LONG lCount );
};

extern CModelViewerApp theApp;