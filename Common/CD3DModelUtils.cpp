#include "CD3DModelUtils.h"
#include <assert.h>
#include "C3DScanFileUtils.h"

class CMyEffectInclude : public ID3DXInclude
{
public:

	CMyEffectInclude() : ID3DXInclude ()
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

/*
bool CD3DModelUtils::CreateFromObj ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, MY_OBJ& Obj, D3D_MODEL& d3dModel )
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

			D3D_SUBSET d3dSubset ;

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
						D3DXVECTOR3 ( -VertList [ face.VertIndex [ 0 ] ].x,
						VertList [ face.VertIndex [ 0 ] ].y,
						VertList [ face.VertIndex [ 0 ] ].z ) ;

					d3dSubset.pVB [ iVbIndex + 1 ].ptPos =
						D3DXVECTOR3 ( -VertList [ face.VertIndex [ iVert - 1 ] ].x,
							VertList [ face.VertIndex [ iVert - 1 ] ].y,
							VertList [ face.VertIndex [ iVert - 1 ] ].z ) ;

					d3dSubset.pVB [ iVbIndex + 2 ].ptPos =
						D3DXVECTOR3 ( -VertList [ face.VertIndex [ iVert - 0 ] ].x,
							VertList [ face.VertIndex [ iVert - 0 ] ].y,
							VertList [ face.VertIndex [ iVert - 0 ] ].z ) ;

					if ( db.bHasNormal ) {
						d3dSubset.pVB [ iVbIndex ].vNormal =
							D3DXVECTOR3 ( -NormalList [ face.NormalIndex [ 0 ] ].x,
								NormalList [ face.NormalIndex [ 0 ] ].y,
								NormalList [ face.NormalIndex [ 0 ] ].z ) ;

						d3dSubset.pVB [ iVbIndex + 1 ].vNormal =
							D3DXVECTOR3 ( -NormalList [ face.NormalIndex [ iVert - 1 ] ].x,
								NormalList [ face.NormalIndex [ iVert - 1 ] ].y,
								NormalList [ face.NormalIndex [ iVert - 1 ] ].z ) ;

						d3dSubset.pVB [ iVbIndex + 2 ].vNormal =
							D3DXVECTOR3 ( -NormalList [ face.NormalIndex [ iVert - 0 ] ].x,
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
			d3dSubset.sMatName = db.sMatName ;

			d3dPart.Batches.push_back ( d3dSubset ) ;
		}

		if ( d3dPart.Batches.size() )
			d3dModel.Parts.push_back ( d3dPart ) ;
	}

	// Materials
	for ( unsigned int iMtrl = 0 ; iMtrl < Obj.Materials.size () ; iMtrl++ ) {
		MY_MTL& mtrl = Obj.Materials [ iMtrl ] ;

		D3D_MATERIAL d3dMtl ;
		d3dMtl.sName = mtrl.sName ;
		d3dMtl.clrAmbient = D3DXCOLOR ( mtrl.Ka [ 0 ], mtrl.Ka [ 1 ], mtrl.Ka [ 2 ], 1.0f ) ;
		d3dMtl.clrDiffuse = D3DXCOLOR ( mtrl.Kd [ 0 ], mtrl.Kd [ 1 ], mtrl.Kd [ 2 ], 1.0f ) ;
		d3dMtl.clrSpecular = D3DXCOLOR ( mtrl.Ks [ 0 ], mtrl.Ks [ 1 ], mtrl.Ks [ 2 ], 1.0f ) ;
		d3dMtl.fTransparency = 1.0f - mtrl.fTr ;
		d3dMtl.fGlossiness = mtrl.fNs ;

		hr = D3DXCreateTextureFromFileA ( pDevice, mtrl.sMapKd.c_str (), &d3dMtl.pTexDiffuse ) ;

		char szTempPath [ MAX_PATH ] ;
		GetCurrentDirectoryA ( MAX_PATH, szTempPath ) ;

		CString str ;
		AfxGetModuleFileName ( AfxGetInstanceHandle (), str ) ;

		str.ReverseFind ( '\\' ) ;
		str = str.Left ( str.ReverseFind ( '\\' ) ) ;
		str = str + L"\\" ;

		SetCurrentDirectory ( str.GetBuffer () ) ;

		//str = str + L"\\DiffuseMapSpec_trans.fx" ;

		hr = D3DXCreateEffectFromFileA ( pDevice,
			"DiffuseMapSpec_trans.fx",
			NULL,
			&EffectInclude,
			0,
			pEffectPool,
			&d3dMtl.pShader,
			NULL ) ;

		SetCurrentDirectoryA ( szTempPath ) ;

		d3dModel.Materials [ mtrl.sName ] = d3dMtl ;
	}

	d3dModel.ptMin = D3DXVECTOR3 ( Obj.ptMin.x, Obj.ptMin.y, Obj.ptMin.z ) ;
	d3dModel.ptMax = D3DXVECTOR3 ( Obj.ptMax.x, Obj.ptMax.y, Obj.ptMax.z ) ;

	return true ;
}
*/

bool CD3DModelUtils::CreateFromTDModel ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, TD_SCAN_MODEL& model, D3D_MODEL& d3dModel )
{
	if ( ! pDevice )
		return false ;

	HRESULT hr ;
	CMyEffectInclude EffectInclude ;

	for ( uint32_t iPart = 0 ; iPart < model.Parts.size() ; iPart++ ) {

		D3DMODEL_PART part ;
		part.sName = model.Parts [ iPart ].sName ;

		for ( uint32_t iSubset = 0 ; iSubset < model.Parts [ iPart ].Subsets.size() ; iSubset++ ) {
			TD_MODEL_SUBSET& mdlsub = model.Parts [ iPart ].Subsets [ iSubset ] ;
			D3DMODEL_SUBSET subset ;
			subset.iTriCount = mdlsub.iTriCount ;
			if ( mdlsub.pIB ) {
				subset.pIB = new uint32_t [ mdlsub.iTriCount * 3 ] ;
				memcpy ( subset.pIB, mdlsub.pIB, mdlsub.iTriCount * 3 * sizeof ( uint32_t ) ) ;
			}
			if ( mdlsub.pVB ) {
				subset.pVB = new uint8_t [ mdlsub.iTriCount * 3 * C3DScanFileUtils::GetVertexSize ( mdlsub.uiVertexFmt ) ] ;
				memcpy ( subset.pVB, mdlsub.pVB, mdlsub.iTriCount * 3 * C3DScanFileUtils::GetVertexSize ( mdlsub.uiVertexFmt ) ) ;
			}
			subset.sMatName = mdlsub.sMatName ;
			subset.uiVertexFmt = mdlsub.uiVertexFmt ;
			subset.uiFVF = VertexFormatToFvf ( subset.uiVertexFmt ) ;

			part.Subsets.push_back ( subset ) ;
		}

		d3dModel.Parts.push_back ( part ) ;
	}

	for ( uint32_t iTex = 0 ; iTex < model.Textures.size() ; iTex++ ) {
		auto i = model.Textures.begin () ;
		advance ( i, iTex ) ;

		D3DMODEL_TEXTURE_SLOT slot ;
		slot.sName = i->second.sName ;
		slot.eFormat = i->second.eFormat ;
		slot.uiSize = i->second.uiSize ;
		slot.pData = new uint8_t [ slot.uiSize ] ;
		memcpy ( slot.pData, i->second.pData, slot.uiSize ) ;

		D3DXCreateTextureFromFileInMemory ( pDevice, slot.pData, slot.uiSize, &slot.pTexture ) ;

		d3dModel.Textures [ slot.sName ] = slot ;
	}

	for ( uint32_t iMat = 0 ; iMat < model.Materials.size() ; iMat++ ) {
		auto i = model.Materials.begin() ;
		advance ( i, iMat ) ;

		TD_MODEL_MATERIAL& mdlmtrl = i->second ;
		D3DMODEL_MATERIAL mtrl ;
		
		mtrl.sName = mdlmtrl.sName ;
		mtrl.clrAmbient = mdlmtrl.clrAmbient ;
		mtrl.clrDiffuse = mdlmtrl.clrDiffuse ;
		mtrl.fSpecIntensity = mdlmtrl.fSpecIntensity ;
		mtrl.fGlossiness = mdlmtrl.fGlossiness ;
		mtrl.fTransparency = mdlmtrl.fTransparency ;

		mtrl.sDiffuseTextureName = mdlmtrl.sDiffuseTextureName ;
		mtrl.sAlphaTextureName = mdlmtrl.sAlphaTextureName ;
		mtrl.sNormalTextureName = mdlmtrl.sNormalTextureName ;
		mtrl.sSpecularTextureName = mdlmtrl.sSpecularTextureName ;
		mtrl.sReflectionTextureName = mdlmtrl.sReflectionTextureName ;

		hr = D3DXCreateEffectFromFileA ( pDevice,
			"DiffuseMapSpec_trans.fx",
			NULL,
			&EffectInclude,
			0,
			pEffectPool,
			&mtrl.pShader,
			NULL ) ;

		d3dModel.Materials [ mtrl.sName ] = mtrl ;
	}

	d3dModel.ptMin = model.ptMin ;
	d3dModel.ptMax = model.ptMax ;

	return true ;
}

/*
bool CD3DModelUtils::RenderD3DModel ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel )
{
	for ( unsigned int iPart = 0 ; iPart < d3dModel.Parts.size() ; iPart++ ) {
		for ( unsigned int iSubset = 0 ; iSubset < d3dModel.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
			
			D3D_SUBSET& subset = d3dModel.Parts [ iPart ].Subsets [ iSubset ] ;

			pDevice->SetFVF ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 ) ;

			D3D_MATERIAL& d3dMtl = d3dModel.Materials [ subset.sMatName ] ;

			d3dMtl.pShader->SetFloatArray ( "g_f4AmbientColor", (float*)&d3dMtl.clrAmbient, 4 ) ;
			d3dMtl.pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&d3dMtl.clrDiffuse, 4 ) ;
			d3dMtl.pShader->SetFloatArray ( "g_f4SpecularColor", (float*)&d3dMtl.clrSpecular, 4 ) ;
			d3dMtl.pShader->SetFloat ( "g_fTransparency", d3dMtl.fTransparency ) ;
			d3dMtl.pShader->SetFloat ( "g_fGlossiness", d3dMtl.fGlossiness ) ;
			d3dMtl.pShader->SetFloat ( "g_fSpecularIntensity", d3dMtl.clrSpecular.r ) ;

			//d3dMtl.pShader->SetTexture ( "g_txDiffuse", d3dMtl.pTexDiffuse ) ;

			UINT uiPassCount = 0 ;
			d3dMtl.pShader->Begin ( &uiPassCount, 0 ) ;
			for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
				d3dMtl.pShader->BeginPass ( iPass ) ;

				pDevice->DrawPrimitiveUP ( D3DPT_TRIANGLELIST,
					subset.iTriCount,
					subset.pVB,
					sizeof ( D3D_VERTEX ) ) ;

				d3dMtl.pShader->EndPass() ;
			}
			d3dMtl.pShader->End() ;

		}
	}

	return true ;
}
*/
void CD3DModelUtils::FreeD3DModel ( D3D_MODEL & d3dModel )
{
/*	for ( unsigned int iPart = 0 ; iPart < d3dModel.Parts.size () ; iPart++ ) {
		D3DMODEL_PART& part = d3dModel.Parts [ iPart ] ;
		for ( unsigned int iSubset = 0 ; iSubset < d3dModel.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
			D3D_SUBSET& subset = part.Subsets [ iSubset ] ;

			if ( subset.pVB )
				delete subset.pVB ;
			subset.pVB = NULL ;
			subset.iTriCount = 0 ;
		}
		part.Subsets.clear () ;
	}

	d3dModel.Parts.clear () ;

	for ( auto iMtrl = d3dModel.Materials.begin () ; iMtrl != d3dModel.Materials.end () ; iMtrl++ ) {
		D3D_MATERIAL& mtrl = iMtrl->second ;
// 		if ( mtrl.pTexDiffuse )
// 			mtrl.pTexDiffuse->Release() ;
		if ( mtrl.pShader )
			mtrl.pShader->Release() ;
	}

	d3dModel.Materials.clear() ;*/
}

uint32_t CD3DModelUtils::VertexFormatToFvf ( uint32_t uiVertFmt )
{
	uint32_t uiFvf = 0 ;
	if ( uiVertFmt & VF_POSITIION )
		uiFvf |= D3DFVF_XYZ ;
	if ( uiVertFmt & VF_NORMAL )
		uiFvf |= D3DFVF_NORMAL ;
	if ( uiVertFmt & VF_UV )
		uiFvf |= D3DFVF_TEX2 ;

	return uiFvf ;
}

