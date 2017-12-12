#pragma once


#include <windows.h>
#include "D3DModel.h"

class CD3DModelUtils {
public:
	static bool CreateFromTDModel ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, TD_SCAN_MODEL& model, D3D_MODEL& d3dModel ) ;
	static bool RenderD3DModel ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel ) ;
	static void FreeD3DModel ( D3D_MODEL& d3dModel ) ;
	static uint32_t VertexFormatToFvf ( uint32_t uiVertFmt ) ;
};



class CMyEffectInclude : public ID3DXInclude
{
public:

	CMyEffectInclude () : ID3DXInclude ()
	{
		m_szIncludePath [ 0 ] = 0 ;
	}
	~CMyEffectInclude ()
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


