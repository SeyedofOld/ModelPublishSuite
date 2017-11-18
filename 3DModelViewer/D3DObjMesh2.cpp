#include "stdafx.h"

# include "D3DObjMesh2.h"
# include <assert.h>
# include <vector>

class CMyEffectInclude : public ID3DXInclude
{
public:

	CMyEffectInclude() : ID3DXInclude ()
	{
	}
	~CMyEffectInclude ()
	{
	}
	STDMETHOD ( Open )( THIS_ D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes )
	{
		char szFile [ MAX_PATH ] ;
		sprintf_s ( szFile, MAX_PATH, "%s/%s", m_szIncludePath, pFileName ) ;

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

bool CD3DMesh2::CreateFromObj ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, MY_OBJ& Obj, D3D_MODEL& d3dModel )
{
	HRESULT hr ;

	vector<TFloat3>& VertList = Obj.Vertices ;
	vector<TFloat3>& NormalList = Obj.Normals ;
	vector<UV>& UvList = Obj.UVs ;

	CMyEffectInclude EffectInclude ;

	for ( unsigned int iPart = 0 ; iPart < Obj.subParts.size () ; iPart++ ) {

		D3D_MODEL_PART d3dPart ;
		d3dPart.sName = Obj.subParts [ iPart ].sName ;

		for ( unsigned int iSubset = 0 ; iSubset < Obj.subParts [ iPart ].subSets.size () ; iSubset++ ) {

			MY_DRAW_BATCH& db = Obj.subParts [ iPart ].subSets [ iSubset ] ;
			if ( db.Faces.size() == 0 )
				continue ;

			D3D_DRAW_BATCH d3dSubset ;

			for ( unsigned int iFace = 0 ; iFace < db.Faces.size () ; iFace++ )
				d3dSubset.iTriCount += db.Faces [ iFace ].VertIndex.size() - 2 ;

			d3dSubset.pVB = new MY_VERTEX [ d3dSubset.iTriCount * 3 ] ;
			ZeroMemory ( d3dSubset.pVB, d3dSubset.iTriCount * 3 * sizeof(MY_VERTEX) ) ;

			d3dSubset.bNormal = db.bHasNormal ;
			d3dSubset.bUv = db.bHasUv ;

			int iVbIndex = 0 ;
			for ( unsigned int iFace = 0 ; iFace < db.Faces.size() ; iFace++ ) {
				MY_FACE& face = db.Faces [ iFace ] ;
				for ( unsigned int iVert = 2 ; iVert < face.VertIndex.size() ; iVert++ ) {

					d3dSubset.pVB [ iVbIndex ].ptPos = 
						D3DXVECTOR3 ( VertList [ face.VertIndex [ 0 ] ].x,
						VertList [ face.VertIndex [ 0 ] ].y,
						VertList [ face.VertIndex [ 0 ] ].z ) ;

					d3dSubset.pVB [ iVbIndex + 1 ].ptPos =
						D3DXVECTOR3 ( VertList [ face.VertIndex [ iVert - 1 ] ].x,
							VertList [ face.VertIndex [ iVert - 1 ] ].y,
							VertList [ face.VertIndex [ iVert - 1 ] ].z ) ;

					d3dSubset.pVB [ iVbIndex + 2 ].ptPos =
						D3DXVECTOR3 ( VertList [ face.VertIndex [ iVert - 0 ] ].x,
							VertList [ face.VertIndex [ iVert - 0 ] ].y,
							VertList [ face.VertIndex [ iVert - 0 ] ].z ) ;

					if ( db.bHasNormal ) {
						d3dSubset.pVB [ iVbIndex ].vNormal =
							D3DXVECTOR3 ( NormalList [ face.NormalIndex [ 0 ] ].x,
								NormalList [ face.NormalIndex [ 0 ] ].y,
								NormalList [ face.NormalIndex [ 0 ] ].z ) ;

						d3dSubset.pVB [ iVbIndex + 1 ].vNormal =
							D3DXVECTOR3 ( NormalList [ face.NormalIndex [ iVert - 1 ] ].x,
								NormalList [ face.NormalIndex [ iVert - 1 ] ].y,
								NormalList [ face.NormalIndex [ iVert - 1 ] ].z ) ;

						d3dSubset.pVB [ iVbIndex + 2 ].vNormal =
							D3DXVECTOR3 ( NormalList [ face.NormalIndex [ iVert - 0 ] ].x,
								NormalList [ face.NormalIndex [ iVert - 0 ] ].y,
								NormalList [ face.NormalIndex [ iVert - 0 ] ].z ) ;
					}

					if ( db.bHasUv ) {
						d3dSubset.pVB [ iVbIndex ].vUv =
							D3DXVECTOR2 ( UvList [ face.UvIndex [ 0 ] ].u,
								UvList [ face.UvIndex [ 0 ] ].v ) ;

						d3dSubset.pVB [ iVbIndex + 1 ].vUv =
							D3DXVECTOR2 ( UvList [ face.UvIndex [ iVert - 1 ] ].u,
								UvList [ face.UvIndex [ iVert - 1 ] ].v ) ;

						d3dSubset.pVB [ iVbIndex + 2 ].vUv =
							D3DXVECTOR2 ( UvList [ face.UvIndex [ iVert - 0 ] ].u,
								UvList [ face.UvIndex [ iVert - 0 ] ].v ) ;
					}

					iVbIndex += 3 ;
				}
			}

			// Material
			MY_MTL& mtrl = Obj.Materials [ 0 ] ;
			for ( unsigned int iMtrl = 0 ; iMtrl = Obj.Materials.size () ; iMtrl++ ) {
				if ( Obj.Materials [ iMtrl ].sName == db.sMatName ) {
					mtrl = Obj.Materials [ iMtrl ] ;
					break ;
				}
			}

			d3dSubset.clrAmbient = D3DXCOLOR ( mtrl.Ka [ 0 ], mtrl.Ka [ 1 ], mtrl.Ka [ 2 ], 1.0f ) ;
			d3dSubset.clrDiffuse = D3DXCOLOR ( mtrl.Kd [ 0 ], mtrl.Kd [ 1 ], mtrl.Kd [ 2 ], 1.0f ) ;
			d3dSubset.clrSpecular = D3DXCOLOR ( mtrl.Ks [ 0 ], mtrl.Ks [ 1 ], mtrl.Ks [ 2 ], 1.0f ) ;
			d3dSubset.fTransparency = mtrl.fTr ;
			d3dSubset.fGlossiness = mtrl.fNs ;
			
			//hr = D3DXCreateEffect ( pd3dDevice, pBuf, dwSize, pDefines, m_pEffectInclude, 0, m_pEffectPool, &pd3dEffect, &pErrBuf ) ;
			hr = D3DXCreateTextureFromFileA ( pDevice, mtrl.sMapKd.c_str (), &d3dSubset.pTexDiffuse ) ;
			
			hr = D3DXCreateEffectFromFileA ( pDevice,
				"DiffuseMapSpec_trans.fx",
				NULL,
				&EffectInclude,
				0,
				pEffectPool,
				&d3dSubset.pShader,
				NULL ) ;

			d3dPart.Batches.push_back ( d3dSubset ) ;
		}

		if ( d3dPart.Batches.size() )
			d3dModel.Parts.push_back ( d3dPart ) ;
	}

	return true ;
}


bool CD3DMesh2::RenderD3DMesh ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel )
{
	for ( unsigned int iPart = 0 ; iPart < d3dModel.Parts.size () ; iPart++ ) {
		for ( unsigned int iSubset = 0 ; iSubset < d3dModel.Parts [ iPart ].Batches.size () ; iSubset++ ) {
			
			D3D_DRAW_BATCH& subset = d3dModel.Parts [ iPart ].Batches [ iSubset ] ;

			pDevice->SetFVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 ) ;

			subset.pShader->SetFloatArray ( "g_f4AmbientColor", subset.clrAmbient, 4 ) ;
			subset.pShader->SetFloatArray ( "g_f4DiffuseColor", subset.clrDiffuse, 4 ) ;
			subset.pShader->SetFloatArray ( "g_f4SpecularColor", subset.clrSpecular, 4 ) ;
			subset.pShader->SetFloat ( "g_fTransparency", subset.fTransparency ) ;
			subset.pShader->SetTexture ( "g_txDiffuse", subset.pTexDiffuse ) ;
			subset.pShader->SetFloat ( "g_fGlossiness", subset.fGlossiness ) ;

			UINT uiPassCount = 0 ;
			subset.pShader->Begin ( &uiPassCount, 0 ) ;
			for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
				subset.pShader->BeginPass ( iPass ) ;

				pDevice->DrawPrimitiveUP ( D3DPT_TRIANGLELIST,
					subset.iTriCount,
					subset.pVB,
					sizeof ( MY_VERTEX ) ) ;

				subset.pShader->EndPass() ;
			}
			subset.pShader->End() ;

		}
	}

	return true ;
}

