
// 3DModelViewer.h : main header file for the 3DModelViewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMy3DModelViewerApp:
// See 3DModelViewer.cpp for the implementation of this class
//

class CMy3DModelViewerApp : public CWinAppEx
{
public:
	CMy3DModelViewerApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMy3DModelViewerApp theApp;
