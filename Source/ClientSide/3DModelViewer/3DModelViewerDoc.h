
// 3DModelViewerDoc.h : interface of the CMy3DModelViewerDoc class
//


#pragma once

//#include "D3DObjMesh.h"
#include "D3DObjMesh2.h"

class CMy3DModelViewerDoc : public CDocument
{
protected: // create from serialization only
	CMy3DModelViewerDoc();
	DECLARE_DYNCREATE(CMy3DModelViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMy3DModelViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument ( LPCTSTR lpszPathName );
	bool LoadModelFromMemory ( void * pData, DWORD dwDataSize, D3D_MODEL& d3dModel );
	D3D_MODEL m_d3dMesh1 ;
	D3D_MODEL m_d3dMesh2 ;
	afx_msg void OnOpenSecond ();
};
