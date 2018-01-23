
// ModelViewerDlg.h : header file
//

#pragma once
#include "tlCCamera.h"
#include "tlCGuiRenderer.h"
#include "tlCSettingsGui.h"
#include "tlC3DViewContext.h"
#include "D3DModel.h"
#include "CRenderDialog.h"
#include "CModelCache.h"


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

	struct POINTER_PASS {
		void* pData ;
		int32_t iSize ;
	};

public:
	ID3DXEffect*	m_pShader ;
	CCamera			m_Camera ;
	CGuiRenderer	m_GuiRenderer ;
	CSettingsGui	m_SettingsGui ;
	C3DViewContext*  m_pView ;
	
	bool			m_bFileOpened ;
	bool			m_bHasFilename ;
	CString			m_strFilename ;

	D3D_MODEL*		m_pd3dModel1 ;
	TD_SCAN_MODEL*	m_pModel1 ;
	D3DXMATRIX		m_matWorld ;

	vector3			m_ptPos ;
	float			m_fYaw ;
	float			m_fPitch ;

	float4_rgba		m_clrClear ;
	float4_rgba		m_clrLight ; 

	IDirect3DTexture9*	m_pAdTex ;
	std::string			m_strAdUrl ;

	bool			m_bDownloading ;
	float			m_fDownloadProgress ;

	POINTER_PASS	m_PointerPass [ 3 ] ;
	CModelCache		m_Cache ;

public:
	void Update() ;
	void ShowExampleMenuFile();
	void Render();
	void UpdateWorldMatrix() ;
	void FillTextureList() ;
	void CalcTextureAverages() ;
	void ResetView ();

	bool Load3DScanFile ( CString& strPathName ) ;
	bool Load3DScanFromUrl ( CString& strUrl ) ;

	void DownloadInfo ( wstring& strUrl ) ;
	void DownloadModel ( wstring& strUrl ) ;
	void DownloadAd ( wstring& strUrl ) ;

	char**			m_ppszTextureNames ;
	int				m_iTextureCount ;
	int				m_iFileSize ;

	std::wstring	m_strSubsid ;

	std::wstring	m_strModel ;
	std::wstring	m_strAd ;

	bool			m_bShowRecent ;
	IDirect3DTexture9* m_pThumbTex ;

	bool			m_bInitialized ;

	afx_msg void OnSize ( UINT nType, int cx, int cy );
	virtual LRESULT WindowProc ( UINT message, WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd ( CDC* pDC );
	void UpdateGui() ;

};
