
// ModelViewerDlg.h : header file
//

#pragma once
#include "tlCCamera.h"
#include "tlCGuiRenderer.h"
#include "tlCSettingsGui.h"
#include "tlC3DViewContext.h"
#include "D3DModel.h"
#include "CRenderDialog.h"

// CModelViewerDlg dialog
class CModelViewerDlg : public CRenderDialog
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
	bool			m_bFileOpened ;
	D3D_MODEL*		m_pd3dModel1 ;
	TD_SCAN_MODEL*	m_pModel1 ;
	D3DXMATRIX		m_matWorld ;
	float			m_fYaw ;
	float			m_fPitch ;

public:
	void Update() ;
	void ShowExampleMenuFile();
	void Render();
	void UpdateWorldMatrix() ;

	afx_msg void OnSize ( UINT nType, int cx, int cy );
	virtual LRESULT WindowProc ( UINT message, WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd ( CDC* pDC );
	void UpdateGui() ;
};
