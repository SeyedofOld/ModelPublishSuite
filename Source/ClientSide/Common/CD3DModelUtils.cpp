#include "CD3DModelUtils.h"
#include <assert.h>
#include "C3DScanFileUtils.h"


bool CD3DModelUtils::CreateFromTDModel ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, TD_SCAN_MODEL& model, D3D_MODEL& d3dModel )
{
	if ( ! pDevice )
		return false ;

	HRESULT hr ;
	CMyEffectInclude EffectInclude ;

	for ( uint32_t iMat = 0 ; iMat < model.Materials.size () ; iMat++ ) {
		auto i = model.Materials.begin () ;
		advance ( i, iMat ) ;

		TD_MODEL_MATERIAL& mdlmtrl = i->second ;
		D3DMODEL_MATERIAL mtrl ;

// 		mtrl.sName = mdlmtrl.sName ;
		mtrl.clrAmbient = mdlmtrl.clrAmbient ;
		mtrl.clrDiffuse = mdlmtrl.clrDiffuse ;
		mtrl.fSpecIntensity = mdlmtrl.fSpecIntensity ;
		mtrl.fGlossiness = mdlmtrl.fGlossiness ;
		mtrl.fTransparency = mdlmtrl.fTransparency ;
		mtrl.fReflectionFactor = mdlmtrl.fReflectionFactor ;

		mtrl.sDiffuseTextureName = mdlmtrl.sDiffuseTextureName ;
		mtrl.sAlphaTextureName = mdlmtrl.sAlphaTextureName ;
		mtrl.sNormalTextureName = mdlmtrl.sNormalTextureName ;
		mtrl.sSpecularTextureName = mdlmtrl.sSpecularTextureName ;
		mtrl.sReflectionTextureName = mdlmtrl.sReflectionTextureName ;

		mtrl.pBase = &mdlmtrl ;

		hr = D3DXCreateEffectFromFileA ( pDevice,
			"UberShader.fx",
			NULL,
			&EffectInclude,
			0,
			pEffectPool,
			&mtrl.pShader,
			NULL ) ;

		d3dModel.Materials [ mdlmtrl.sName ] = mtrl ;
		//d3dModel.Materials [ mdlmtrl.sName ].pBase = &mdlmtrl ;
	}

	for ( uint32_t iTex = 0 ; iTex < model.Textures.size () ; iTex++ ) {
		auto i = model.Textures.begin () ;
		advance ( i, iTex ) ;

		D3DMODEL_TEXTURE_SLOT slot ;
		slot.pBase = &i->second ;

		D3DXCreateTextureFromFileInMemory ( pDevice, slot.pBase->pData, slot.pBase->uiSize, &slot.pTexture ) ;

		d3dModel.Textures [ i->second.sName ] = slot ;
	}

	for ( uint32_t iPart = 0 ; iPart < model.Parts.size() ; iPart++ ) {

		D3DMODEL_PART part ;
		part.pBase = &model.Parts [ iPart ] ;
		//part.sName = model.Parts [ iPart ].sName ;

		for ( uint32_t iSubset = 0 ; iSubset < model.Parts [ iPart ].Subsets.size() ; iSubset++ ) {
			TD_MODEL_SUBSET& mdlsub = model.Parts [ iPart ].Subsets [ iSubset ] ;
			D3DMODEL_SUBSET subset ;
			subset.pBase = &mdlsub ;

			subset.uiFVF = VertexFormatToFvf ( subset.pBase->uiVertexFmt ) ;

			subset.Material = d3dModel.Materials [ subset.pBase->sMatName ] ;

			SetTextureNameIndex ( d3dModel, subset.Material.sDiffuseTextureName, subset.Material.iDiffuseTex ) ;
			SetTextureNameIndex ( d3dModel, subset.Material.sAlphaTextureName, subset.Material.iAlphaTex ) ;
			SetTextureNameIndex ( d3dModel, subset.Material.sNormalTextureName, subset.Material.iNormalTex ) ;
			SetTextureNameIndex ( d3dModel, subset.Material.sSpecularTextureName, subset.Material.iSpecTex ) ;
			SetTextureNameIndex ( d3dModel, subset.Material.sReflectionTextureName, subset.Material.iReflTex ) ;

			part.Subsets.push_back ( subset ) ;
		}

		d3dModel.Parts.push_back ( part ) ;
	}

	d3dModel.ptMin = model.ptMin ;
	d3dModel.ptMax = model.ptMax ;

	return true ;
}


bool CD3DModelUtils::RenderD3DModel ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel )
{
	for ( unsigned int iPart = 0 ; iPart < d3dModel.Parts.size() ; iPart++ ) {
		if ( ! d3dModel.Parts [ iPart ].bVisible )
			continue ;
		for ( unsigned int iSubset = 0 ; iSubset < d3dModel.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
			
			D3DMODEL_SUBSET& subset = d3dModel.Parts [ iPart ].Subsets [ iSubset ] ;
			if ( ! subset.bVisible )
				continue ;
				//d3dModel.Parts [ iPart ].Subsets [ iSubset ] ;

			//D3DMODEL_MATERIAL& d3dMtl = d3dModel.Materials [ subset.sMatName ] ;
			D3DMODEL_MATERIAL& d3dMtl = subset.Material ;
			
			pDevice->SetFVF ( subset.uiFVF ) ;

			d3dMtl.pShader->SetFloatArray ( "g_f4AmbientColor", (float*)&d3dMtl.clrAmbient, 4 ) ;
			d3dMtl.pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&d3dMtl.clrDiffuse, 4 ) ;
			if ( subset.bSelected ) {
				float4_rgba f4 = d3dMtl.clrDiffuse ;
				f4.r *= 2.0f ;
				f4.g *= 2.0f ;
				f4.b *= 2.0f ;
				f4.a = 1.0f ;
				d3dMtl.pShader->SetFloatArray ( "g_f4DiffuseColor", (float*)&f4, 4 ) ;
			}
			d3dMtl.pShader->SetFloat ( "g_fTransparency", d3dMtl.fTransparency ) ;
			d3dMtl.pShader->SetFloat ( "g_fGlossiness", d3dMtl.fGlossiness ) ;
			d3dMtl.pShader->SetFloat ( "g_fSpecularIntensity", d3dMtl.fSpecIntensity ) ;

			d3dMtl.pShader->SetBool ( "g_bHasNormal", ( subset.pBase->uiVertexFmt & VF_NORMAL ) != 0 ) ;
			d3dMtl.pShader->SetBool ( "g_bHasUv", (subset.pBase->uiVertexFmt & VF_UV) != 0 ) ;

			if ( d3dModel.Textures.find ( d3dMtl.sDiffuseTextureName ) != d3dModel.Textures.end () ) {
				d3dMtl.pShader->SetTexture ( "g_txDiffuse", d3dModel.Textures [ d3dMtl.sDiffuseTextureName ].pTexture ) ;
				d3dMtl.pShader->SetBool ( "g_bHasDiffTex", true ) ;
			}
			else {
				d3dMtl.pShader->SetBool ( "g_bHasDiffTex", false ) ;
			}

			if ( d3dModel.Textures.find ( d3dMtl.sAlphaTextureName ) != d3dModel.Textures.end () ) {
				d3dMtl.pShader->SetTexture ( "g_txAlpha", d3dModel.Textures [ d3dMtl.sAlphaTextureName ].pTexture ) ;
				d3dMtl.pShader->SetBool ( "g_bHasAlphaTex", true ) ;
			}
			else {
				d3dMtl.pShader->SetBool ( "g_bHasAlphaTex", false ) ;
			}

			if ( d3dModel.Textures.find ( d3dMtl.sNormalTextureName ) != d3dModel.Textures.end () ) {
				d3dMtl.pShader->SetTexture ( "g_txNormal", d3dModel.Textures [ d3dMtl.sNormalTextureName ].pTexture ) ;
				d3dMtl.pShader->SetBool ( "g_bHasNormTex", true ) ;
			}
			else {
				d3dMtl.pShader->SetBool ( "g_bHasNormTex", false ) ;
			}

			if ( d3dModel.Textures.find ( d3dMtl.sSpecularTextureName ) != d3dModel.Textures.end () ) {
				d3dMtl.pShader->SetTexture ( "g_txSpecular", d3dModel.Textures [ d3dMtl.sSpecularTextureName ].pTexture ) ;
				d3dMtl.pShader->SetBool ( "g_bHasSpecTex", true ) ;
			}
			else {
				d3dMtl.pShader->SetBool ( "g_bHasSpecTex", false ) ;
			}

			if ( d3dModel.Textures.find ( d3dMtl.sReflectionTextureName ) != d3dModel.Textures.end () ) {
				d3dMtl.pShader->SetTexture ( "g_txReflection", d3dModel.Textures [ d3dMtl.sReflectionTextureName ].pTexture ) ;
				d3dMtl.pShader->SetBool ( "g_bHasReflTex", true ) ;
			}
			else {
				d3dMtl.pShader->SetBool ( "g_bHasReflTex", false ) ;
			}

			UINT uiPassCount = 0 ;
			d3dMtl.pShader->Begin ( &uiPassCount, 0 ) ;
			for ( UINT iPass = 0 ; iPass < uiPassCount ; iPass++ ) {
				d3dMtl.pShader->BeginPass ( iPass ) ;

// 				pDevice->DrawPrimitiveUP ( D3DPT_TRIANGLELIST,
// 					subset.pBase->uiTriCount,
// 					subset.pBase->pVB,
// 					C3DScanFileUtils::GetVertexSize (subset.pBase->uiVertexFmt) ) ;

				pDevice->DrawIndexedPrimitiveUP ( D3DPT_TRIANGLELIST,
					0,
					subset.pBase->uiVertCount,
					subset.pBase->uiTriCount,
					subset.pBase->pIB,
					D3DFMT_INDEX32,
					subset.pBase->pVB,
					C3DScanFileUtils::GetVertexSize ( subset.pBase->uiVertexFmt ) ) ;

				d3dMtl.pShader->EndPass() ;
			}
			d3dMtl.pShader->End() ;

		}
	}

	return true ;
}

void CD3DModelUtils::FreeD3DModel ( D3D_MODEL& d3dModel )
{
	for ( unsigned int iPart = 0 ; iPart < d3dModel.Parts.size () ; iPart++ ) {
		D3DMODEL_PART& part = d3dModel.Parts [ iPart ] ;
		for ( unsigned int iSubset = 0 ; iSubset < d3dModel.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
			D3DMODEL_SUBSET& subset = part.Subsets [ iSubset ] ;
		}
		part.Subsets.clear () ;
	}

	d3dModel.Parts.clear () ;

	for ( auto iMtrl = d3dModel.Materials.begin () ; iMtrl != d3dModel.Materials.end () ; iMtrl++ ) {
		D3DMODEL_MATERIAL& mtrl = iMtrl->second ;
		if ( mtrl.pShader )
			mtrl.pShader->Release() ;
	}

	d3dModel.Materials.clear() ;

	for ( auto iTex = d3dModel.Textures.begin () ; iTex != d3dModel.Textures.end () ; iTex++ ) {
		D3DMODEL_TEXTURE_SLOT& tex = iTex->second ;
		if ( tex.pTexture )
			tex.pTexture->Release () ;
	}

	d3dModel.Textures.clear () ;
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

bool CD3DModelUtils::IntersectRay ( float3 ptStart, float3 vDir, D3D_MODEL& model, float3* pptHit, D3DMODEL_SUBSET** ppSubset )
{
	D3DXVECTOR3 ptRayPos = D3DXVECTOR3 ( ptStart.x, ptStart.y, ptStart.z ) ;
	D3DXVECTOR3 vRayDir = D3DXVECTOR3 ( vDir.x, vDir.y, vDir.z ) ;

	std::vector<pair<D3DMODEL_SUBSET*,float>> hitlist ;

	for ( uint32_t iPart = 0 ; iPart < model.Parts.size () ; iPart++ ) {

		for ( uint32_t iSubset = 0 ; iSubset < model.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
			D3DMODEL_SUBSET& mdlsub = model.Parts [ iPart ].Subsets [ iSubset ] ;

			float* pVB = (float*)mdlsub.pBase->pVB ;
			uint32_t iVertSize = C3DScanFileUtils::GetVertexSize ( mdlsub.pBase->uiVertexFmt ) / sizeof ( float ) ;
			for ( uint32_t iTri = 0 ; iTri < mdlsub.pBase->uiTriCount ; iTri++ ) {
				D3DXVECTOR3 p1 = *( (D3DXVECTOR3*)pVB ) ;
				pVB += iVertSize ;
				D3DXVECTOR3 p2 = *( (D3DXVECTOR3*)pVB ) ;
				pVB += iVertSize ;
				D3DXVECTOR3 p3 = *( (D3DXVECTOR3*)pVB ) ;
				pVB += iVertSize ;

				float fDist = 0.0f ;
				float fU, fV ;
				BOOL bHit = D3DXIntersectTri ( &p1, &p2, &p3, &ptRayPos, &vRayDir, &fU, &fV, &fDist )  ;

				if ( bHit ) {
					hitlist.push_back ( pair<D3DMODEL_SUBSET*,float>(&mdlsub,fDist) ) ;
				}
			}
		}
	}

	if ( hitlist.size () == 0 )
		return false ;

	pair<D3DMODEL_SUBSET*, float> closest = pair<D3DMODEL_SUBSET*, float>(NULL, FLT_MAX) ;
	for ( size_t iHit = 0 ; iHit < hitlist.size () ; iHit++ ) {
		if ( hitlist [ iHit ].second < closest.second ) {
			closest = hitlist [ iHit ] ;
		}
	}
	*ppSubset = closest.first ;

	return true ;
}

void CD3DModelUtils::SetTextureNameIndex ( D3D_MODEL& model, string& sTexName, int32_t& iIndex )
{
	iIndex = model.Textures.size() ;

	for ( uint32_t iTex = 0 ; iTex < model.Textures.size() ; iTex++ ) {
		auto i = model.Textures.begin () ;
		advance ( i, iTex ) ;

		if ( sTexName == i->second.pBase->sName )
			iIndex = iTex ;
	}
}
