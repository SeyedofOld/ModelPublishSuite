
// ModelViewerDlg.h : header file
//

#pragma once
#include <d3dx9.h>
#include "tlCCamera.h"
#include "tlCGuiRenderer.h"
#include "tlCSettingsGui.h"
#include "tlC3DViewContext.h"


// CModelViewerDlg dialog
class CModelViewerDlg : public CDialogEx
{
// Construction
public:
	CModelViewerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODELVIEWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel ();
	afx_msg void OnBnClickedOk ();

public:
	ID3DXEffect*	m_pShader ;
	CCamera			m_Camera ;
	CGuiRenderer	m_GuiRenderer ;
	CSettingsGui	m_SettingsGui ;
	C3DViewContext*  m_pView ;

public:
	void Update() ;
	void Render() ;
	afx_msg void OnSize ( UINT nType, int cx, int cy );
	virtual LRESULT WindowProc ( UINT message, WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd ( CDC* pDC );
};
