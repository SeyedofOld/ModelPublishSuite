/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Convert Obj data structure to 3d scan model structure
*********************************************************************/

#pragma once

#include "Obj2Model.h"
#include "3DScanFmt.h"
#include "C3DScanFileUtils.h"
#include <algorithm>

void ToLower ( string& str )
{
	std::transform ( str.begin (), str.end (), str.begin (), ::tolower );
}

void RemovePath ( string& str )
{
	char szName [ MAX_PATH ] ;
	char szExt [ MAX_PATH ] ;
	_splitpath ( str.c_str (), NULL, NULL, szName, szExt ) ;

	str = szName ;
	str += szExt ;
}

bool LoadAndAddToTextures ( string& sTexName, TD_SCAN_MODEL& model )
{
	if ( sTexName == "" )
		return true ;

	if ( model.Textures.find ( sTexName ) != model.Textures.end() )
		return true ;

	TD_MODEL_TEXTURE_SLOT slot ;
	slot.sName = sTexName ;

	FILE* pFile = fopen ( sTexName.c_str(), "rb" ) ;
	if ( ! pFile )
		return false ;

	fseek ( pFile, 0, SEEK_END ) ;
	slot.uiSize = ftell ( pFile ) ;
	fseek ( pFile, 0, SEEK_SET ) ;

	slot.pData = new uint8_t [ slot.uiSize ] ;
	if ( ! slot.pData ) {
		fclose ( pFile ) ;
		return false ;
	}

	fread ( slot.pData, 1, slot.uiSize, pFile ) ;
	fclose ( pFile ) ;

	model.Textures [ sTexName ] = slot ;

	return true ;
}

bool ConvertObjTo3DModel ( MY_OBJ& obj, TD_SCAN_MODEL& model )
{
	for ( uint32_t i = 0 ; i < obj.subParts.size() ; i++ ) {
		MY_OBJ_PART& objpart = obj.subParts [ i ] ;

		if ( objpart.subSets.size() == 0 )
			continue ; 

		TD_MODEL_PART part ;

		for ( uint32_t j = 0 ; j < objpart.subSets.size() ; j++ ) {
			MY_DRAW_BATCH& objsub = objpart.subSets [ j ] ;

			if ( objsub.Faces.size() == 0 )
				continue ;

			TD_MODEL_SUBSET subset ;
			subset.uiVertexFmt |= VF_POSITIION ;
			if ( objsub.bHasNormal )
				subset.uiVertexFmt |= VF_NORMAL ;
			if ( objsub.bHasUv )
				subset.uiVertexFmt |= VF_UV ;

			subset.sMatName = objsub.sMatName ;

			for ( uint32_t iFace = 0 ; iFace < objsub.Faces.size () ; iFace++ )
				subset.uiTriCount += objsub.Faces [ iFace ].VertIndex.size () - 2 ;

			subset.uiVertCount = subset.uiTriCount * 3 ;

			uint32_t uiVertSize = C3DScanFileUtils::GetVertexSize ( subset.uiVertexFmt ) ;

			subset.pVB = new uint8_t [ subset.uiTriCount * 3 * uiVertSize ];
			memset ( subset.pVB, 0, subset.uiTriCount * 3 * uiVertSize );

			float* pVB = (float*)subset.pVB ;

			for ( uint32_t iFace = 0; iFace < objsub.Faces.size(); iFace++ ) {
				MY_FACE& face = objsub.Faces [ iFace ];
				for ( uint32_t iVert = 2; iVert < face.VertIndex.size(); iVert++ ) {

					// v1
					*((float3*)pVB) = float3 { -obj.Vertices [ face.VertIndex [ 0 ] ].x,
															 obj.Vertices [ face.VertIndex [ 0 ] ].y,
															 obj.Vertices [ face.VertIndex [ 0 ] ].z };
					pVB += 3 ;

					if ( objsub.bHasNormal ) {
						*( (float3*)pVB ) = float3{ -obj.Normals [ face.NormalIndex [ 0 ] ].x,
																 obj.Normals [ face.NormalIndex [ 0 ] ].y,
																 obj.Normals [ face.NormalIndex [ 0 ] ].z } ;
						pVB += 3 ;
					}

					if ( objsub.bHasUv ) {
						*( (float2*)pVB ) = float2{ obj.UVs [ face.UvIndex [ 0 ] ].u,
																 obj.UVs [ face.UvIndex [ 0 ] ].v };
						pVB += 2 ;
					}

					// v2
					*((float3*)pVB) = float3 { -obj.Vertices [ face.VertIndex [ iVert - 1 ] ].x,
																obj.Vertices [ face.VertIndex [ iVert - 1 ] ].y,
																obj.Vertices [ face.VertIndex [ iVert - 1 ] ].z };
					pVB += 3 ;

					if ( objsub.bHasNormal ) {
						*( (float3*)pVB ) = float3{ -obj.Normals [ face.NormalIndex [ iVert - 1 ] ].x,
																	obj.Normals [ face.NormalIndex [ iVert - 1 ] ].y,
																	obj.Normals [ face.NormalIndex [ iVert - 1 ] ].z } ;
						pVB += 3 ;
					}

					if ( objsub.bHasUv ) {
						*( (float2*)pVB ) = float2{ obj.UVs [ face.UvIndex [ iVert - 1 ] ].u,
																	obj.UVs [ face.UvIndex [ iVert - 1 ] ].v };
						pVB += 2 ;
					}

					// v3
					*( (float3*)pVB ) = float3 { -obj.Vertices [ face.VertIndex [ iVert - 0 ] ].x,
  																obj.Vertices [ face.VertIndex [ iVert - 0 ] ].y,
																obj.Vertices [ face.VertIndex [ iVert - 0 ] ].z };
					pVB += 3 ;

					if ( objsub.bHasNormal ) {
						*( (float3*)pVB ) = float3{ -obj.Normals [ face.NormalIndex [ iVert - 0 ] ].x,
																	obj.Normals [ face.NormalIndex [ iVert - 0 ] ].y,
																	obj.Normals [ face.NormalIndex [ iVert - 0 ] ].z } ;
						pVB += 3 ;
					}

					if ( objsub.bHasUv ) {
						*( (float2*)pVB ) = float2{ obj.UVs [ face.UvIndex [ iVert - 0 ] ].u,
																   obj.UVs [ face.UvIndex [ iVert - 0 ] ].v };
						pVB += 2 ;
					}

				} // Vert
			} // Poly

			subset.pIB = new uint32_t [ subset.uiTriCount * 3 ];
			for ( uint32_t iIndex = 0 ; iIndex < subset.uiTriCount ; iIndex++ )
				subset.pIB [ iIndex ] = iIndex ;


			part.Subsets.push_back ( subset ) ;
		}
		
		part.sName = objpart.sName ;
		if ( part.Subsets.size() != 0 )
			model.Parts.push_back ( part ) ;
	}

	for ( uint32_t iMat = 0 ; iMat < obj.Materials.size () ; iMat++ ) {
		
		TD_MODEL_MATERIAL mtrl ;

		MY_MTL& objmtl = obj.Materials [ iMat ] ;

		mtrl.sName		= objmtl.sName ;

		mtrl.clrAmbient = float4_rgba { objmtl.Ka [ 0 ], objmtl.Ka [ 1 ], objmtl.Ka [ 2 ], 1.0f } ;
		mtrl.clrDiffuse = float4_rgba { objmtl.Kd [ 0 ], objmtl.Kd [ 1 ], objmtl.Kd [ 2 ], 1.0f } ;
		
		mtrl.fSpecIntensity	= objmtl.Ks [ 0 ] ;
		mtrl.fGlossiness	= objmtl.fNs ;
		mtrl.fTransparency	= 1.0f - objmtl.fTr ;

		mtrl.sDiffuseTextureName	= objmtl.sMapKd ;
		mtrl.sNormalTextureName		= objmtl.sMapBump ;
		mtrl.sSpecularTextureName	= objmtl.sMapNs ;
		mtrl.sAlphaTextureName		= objmtl.sMapTr ;
		mtrl.sReflectionTextureName = objmtl.sMapRefl ;

		ToLower ( mtrl.sDiffuseTextureName ) ;
		ToLower ( mtrl.sNormalTextureName ) ;
		ToLower ( mtrl.sSpecularTextureName ) ;
		ToLower ( mtrl.sAlphaTextureName ) ;
		ToLower ( mtrl.sReflectionTextureName ) ;

		RemovePath ( mtrl.sDiffuseTextureName ) ;
		RemovePath ( mtrl.sNormalTextureName ) ;
		RemovePath ( mtrl.sSpecularTextureName ) ;
		RemovePath ( mtrl.sAlphaTextureName ) ;
		RemovePath ( mtrl.sReflectionTextureName ) ;

		bool bFound = false ;
		for ( uint32_t iPart = 0 ; iPart < model.Parts.size () ; iPart++ ) {
			for ( uint32_t iSubset = 0 ; iSubset < model.Parts [ iPart ].Subsets.size () ; iSubset++ ) {
				if ( mtrl.sName == model.Parts [ iPart ].Subsets [ iSubset ].sMatName ) {
					bFound = true ;
					break ;
				}
			}
			if ( bFound )
				break ;
		}
		if ( bFound ) 
			model.Materials [ mtrl.sName ] = mtrl ;
	}

	for ( uint32_t iMat = 0 ; iMat < model.Materials.size() ; iMat++ ) {
		auto i = model.Materials.begin() ;
		advance ( i, iMat ) ;
		
		TD_MODEL_MATERIAL& mtrl = i->second ;

		LoadAndAddToTextures ( mtrl.sDiffuseTextureName, model ) ;
		LoadAndAddToTextures ( mtrl.sAlphaTextureName, model ) ;
		LoadAndAddToTextures ( mtrl.sNormalTextureName, model ) ;
		LoadAndAddToTextures ( mtrl.sSpecularTextureName, model ) ;
		LoadAndAddToTextures ( mtrl.sReflectionTextureName, model ) ;
	}


	return true ;
}




