/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "C3DScanFile.h"
#include "C3DScanFileUtils.h"

TD_SCAN_MODEL* C3DScanFile::Load3DScanModel ( char* pszFilename )
{
	if ( ! pszFilename )
		return NULL;

	FILE * pFile = fopen ( pszFilename, "rb" ) ;
	if ( ! pFile )
		return NULL ;
	
	// Load Header
	TDSCAN_FILE_HEADER hdr;
	fread ( &hdr, sizeof(TDSCAN_FILE_HEADER), 1, pFile ) ;
	if ( memcmp ( hdr.szSign, "SEYEDOF 3D FORMAT", sizeof(hdr.szSign) ) != 0 ) 
		goto load_error ;

	if ( hdr.uiVersion != (1 << 16) + 0 )
		goto load_error ;

	// Load Geometry
	TD_SCAN_MODEL* pModel = new TD_SCAN_MODEL ;
	if ( ! pModel )
		goto load_error ;

	int32_t iNextPartOfs = hdr.iFirstPartOfs ;

	while ( iNextPartOfs > 0 ) {

		fseek ( pFile, iNextPartOfs, SEEK_SET ) ;

		TDSCAN_FILE_PART part_hdr;
		fread ( &part_hdr, sizeof(TDSCAN_FILE_PART), 1, pFile ) ;
		if ( memcmp ( part_hdr.szSign, "PART", sizeof(part_hdr.szSign) ) != 0 )
			goto load_error ;

		{
			TD_MODEL_PART part ;
			pModel->Parts.push_back ( part ) ;
		}
		TD_MODEL_PART& part = pModel->Parts [ pModel->Parts.size() - 1 ] ;

		part.sName = (char*)part_hdr.szName ;
		
		int32_t iNextSubsetOfs = part_hdr.iFirstSubset ;

		while ( iNextSubsetOfs > 0 ) {

			{
				TD_MODEL_SUBSET subset;
				part.Subsets.push_back ( subset ) ;
			}
			TD_MODEL_SUBSET& subset = part.Subsets [ part.Subsets.size () - 1 ] ;

			fseek ( pFile, iNextSubsetOfs, SEEK_SET ) ;

			TDSCAN_FILE_SUBSET sub_hdr;
			fread ( &sub_hdr, sizeof(TDSCAN_FILE_SUBSET), 1, pFile ) ;
			if ( memcmp ( sub_hdr.szSign, "SUBSET", sizeof(sub_hdr.szSign) ) != 0 )
				goto load_error ;

			subset.uiVertexFmt = sub_hdr.uiVertexFormat ;

			uint32_t uiVertexSize = C3DScanFileUtils::GetVertexSize ( sub_hdr.uiVertexFormat ) ;
			if ( sub_hdr.uiDataSize != uiVertexSize * sub_hdr.uiVertexCount + sub_hdr.uiTriCount * 3 * sizeof(uint32_t) )
				goto load_error ;

			subset.pVB = new int8_t [ uiVertexSize * sub_hdr.uiVertexCount ] ;
			if ( ! subset.pVB )
				goto load_error ;

			subset.pIB = new uint32_t [ sizeof(uint32_t) * sub_hdr.uiTriCount * 3 ] ;
			if ( ! subset.pIB ) 
				goto load_error;

			subset.uiTriCount = sub_hdr.uiTriCount ;
			subset.uiVertCount = sub_hdr.uiVertexCount ;

			fread ( subset.pVB, uiVertexSize, sub_hdr.uiVertexCount, pFile ) ;
			fread ( subset.pIB, sizeof(uint32_t), sub_hdr.uiTriCount * 3, pFile ) ;

			subset.sMatName = (char*)sub_hdr.szMatName ;

			iNextSubsetOfs = sub_hdr.iNextSubsetOfs ;
		}

		iNextPartOfs = part_hdr.iNextPartOfs ;
	}

	// Load Materials
	int32_t iNextMatOfs = hdr.iMaterialLibOfs ;
	while ( iNextMatOfs > 0 ) {

		fseek ( pFile, iNextMatOfs, SEEK_SET ) ;

		TDSCAN_FILE_MATERIAL mat_hdr ;
		fread ( &mat_hdr, sizeof ( TDSCAN_FILE_MATERIAL ), 1, pFile ) ;
		if ( memcmp ( mat_hdr.szSign, "MATERIAL", sizeof ( mat_hdr.szSign ) ) != 0 )
			goto load_error ;

		{
			TD_MODEL_MATERIAL mtrl ;
			pModel->Materials [ (char*)mat_hdr.szName ] = mtrl ;
		}

		TD_MODEL_MATERIAL& mtrl = pModel->Materials [ (char*)mat_hdr.szName ] ;

		mtrl.sName			= (char*)mat_hdr.szName ;
		mtrl.clrDiffuse		= mat_hdr.f4Diffuse ;
		mtrl.clrAmbient		= mat_hdr.f4Ambient ;
		mtrl.fSpecIntensity = mat_hdr.fSpecIntensity ;
		mtrl.fTransparency	= mat_hdr.fAlpha ;
		mtrl.fGlossiness	= mat_hdr.fGlossiness ;
		mtrl.fReflectionFactor = mat_hdr.fReflectionFactor ;
		
		mtrl.sDiffuseTextureName	= (char*)mat_hdr.szDiffuseTex ;
		mtrl.sNormalTextureName		= (char*)mat_hdr.szNormalTex ;
		mtrl.sAlphaTextureName		= (char*)mat_hdr.szAlphaTex ;
		mtrl.sSpecularTextureName	= (char*)mat_hdr.szSpecularTex ;
		mtrl.sReflectionTextureName = (char*)mat_hdr.szReflectionTex ;

		iNextMatOfs = mat_hdr.iNextMaterialOfs ;
	}

	// Load Textures
	int32_t iNextTexOfs = hdr.iTexLibOfs ;
	while ( iNextTexOfs > 0 ) {

		fseek ( pFile, iNextTexOfs, SEEK_SET ) ;

		TDSCAN_FILE_TEXTURE tex_hdr ;
		fread ( &tex_hdr, sizeof ( TDSCAN_FILE_TEXTURE ), 1, pFile ) ;
		if ( memcmp ( tex_hdr.szSign, "TEX", sizeof ( tex_hdr.szSign ) ) != 0 )
			goto load_error ;

		{
			TD_MODEL_TEXTURE_SLOT tex ;
			tex.sName = (char*)tex_hdr.szName ;
			pModel->Textures [ tex.sName ] = tex ;
		}
		
		TD_MODEL_TEXTURE_SLOT& tex = pModel->Textures [ (char*)tex_hdr.szName ] ;

		tex.clrAvgColor = tex_hdr.clrAvgColor ;
		tex.uiSize = tex_hdr.uiDataSize ;
		tex.eFormat = tex_hdr.eTexFormat ;
		tex.pData = new int8_t [ tex_hdr.uiDataSize ] ;

		if ( tex_hdr.uiFlags == TDS_FLAG_COMPRESSED ) {
			int8_t* pData = new int8_t [ tex_hdr.uiCompressedSize ] ;
			fread ( pData, 1, tex_hdr.uiCompressedSize, pFile ) ;

			// TODO: Decompression code 
		} 
		else {
			fread ( tex.pData, 1, tex_hdr.uiDataSize, pFile ) ;
		}

		iNextTexOfs = tex_hdr.iNextTexOfs ;
	}

	fclose  ( pFile ) ;
	
	return pModel ;

load_error:
	if ( pFile )
		fclose ( pFile ) ;
// 	if ( pModel )
// 		CD3DModelUtils::FreeD3DModel ( pModel ) ;

	return NULL ;
}

bool C3DScanFile::Save3DScanModel ( char* pszFilename, TD_SCAN_MODEL* pModel )
{
	if ( ! pszFilename || ! pModel )
		return false ;

	FILE * pFile = fopen ( pszFilename, "wb" ) ;
	if ( ! pFile )
		return NULL ;

	uint32_t uiFileOfs = 0 ;

	// Save Header
	TDSCAN_FILE_HEADER hdr ;
	memset ( &hdr, 0, sizeof(TDSCAN_FILE_HEADER) ) ;
	memcpy ( hdr.szSign, "SEYEDOF 3D FORMAT", sizeof("SEYEDOF 3D FORMAT") ) ;
	hdr.uiVersion = ( 1 << 16 ) + 0 ;
	hdr.iFirstPartOfs = sizeof(TDSCAN_FILE_HEADER) ;

	fwrite ( &hdr, sizeof ( TDSCAN_FILE_HEADER ), 1, pFile ) ;
	uiFileOfs = sizeof ( TDSCAN_FILE_HEADER ) ;

	// Save Geometry
	for ( uint32_t iPart = 0 ; iPart < pModel->Parts.size () ; iPart++ ) {
		TD_MODEL_PART& part = pModel->Parts [ iPart ] ;
		
		TDSCAN_FILE_PART part_hdr ;
		memset ( &part_hdr, 0, sizeof ( TDSCAN_FILE_PART ) ) ;
		memcpy ( part_hdr.szSign, "PART", sizeof ( "PART" ) ) ;
		strncpy ( (char*)part_hdr.szName, part.sName.c_str (), sizeof(part_hdr.szName) ) ;
		part_hdr.iFirstSubset = uiFileOfs + sizeof ( TD_MODEL_PART ) ;
		part_hdr.iNextPartOfs = -1 ;

		uint32_t uiPartOfs = uiFileOfs ;

		fwrite ( &part_hdr, sizeof ( TDSCAN_FILE_PART ), 1, pFile ) ;
		uiFileOfs += sizeof ( TDSCAN_FILE_PART ) ;

		for ( uint32_t iSubset = 0 ; iSubset < part.Subsets.size () ; iSubset++ ) {
			TD_MODEL_SUBSET& subset = part.Subsets [ iSubset ] ;
			TDSCAN_FILE_SUBSET sub_hdr ;
			memset ( &sub_hdr, 0, sizeof ( TDSCAN_FILE_SUBSET ) ) ;
			memcpy ( sub_hdr.szSign, "SUBSET", sizeof ( "SUBSET" ) ) ;
			strncpy ( (char*)sub_hdr.szMatName, subset.sMatName.c_str (), sizeof ( sub_hdr.szMatName ) ) ;
			sub_hdr.uiVertexFormat = subset.uiVertexFmt ;
			sub_hdr.uiVertexCount = subset.uiVertCount ;
			sub_hdr.uiTriCount = subset.uiTriCount ;

			uint32_t uiVertexSize = C3DScanFileUtils::GetVertexSize ( subset.uiVertexFmt ) ;
			sub_hdr.uiDataSize = subset.uiVertCount * uiVertexSize + subset.uiTriCount * 3 * sizeof(uint32_t) ;

			// TODO: Compress Data
			sub_hdr.uiCompressedSize = sub_hdr.uiDataSize ;

			if ( iSubset < part.Subsets.size() - 1 )
				sub_hdr.iNextSubsetOfs = uiFileOfs + sub_hdr.uiCompressedSize + sizeof ( TDSCAN_FILE_SUBSET ) ;
			else
				sub_hdr.iNextSubsetOfs = -1 ;

			fwrite ( &sub_hdr, sizeof ( TDSCAN_FILE_SUBSET ), 1, pFile ) ;

			fwrite ( subset.pVB, uiVertexSize, subset.uiVertCount, pFile ) ;
			fwrite ( subset.pIB, sizeof(uint32_t), subset.uiTriCount * 3, pFile ) ;

			uiFileOfs += sub_hdr.uiCompressedSize + sizeof(TDSCAN_FILE_SUBSET) ;

			hdr.uiTotalTriCount += subset.uiTriCount ;
		}

		if ( iPart < pModel->Parts.size () - 1 )
			part_hdr.iNextPartOfs = uiFileOfs ;
		else
			part_hdr.iNextPartOfs= -1 ;

		fseek ( pFile, uiPartOfs, SEEK_SET ) ;
		fwrite ( &part_hdr, sizeof ( TDSCAN_FILE_PART ), 1, pFile ) ;
		fseek ( pFile, uiFileOfs, SEEK_SET ) ;
	}

	if ( pModel->Materials.size() )
		hdr.iMaterialLibOfs = uiFileOfs ;

	// Save Materials
	for ( uint32_t iMat = 0 ; iMat < pModel->Materials.size() ; iMat++ ) {
		auto i = pModel->Materials.begin () ;
		advance ( i, iMat ) ;
		
		TD_MODEL_MATERIAL& mtrl = i->second ;
		
		TDSCAN_FILE_MATERIAL mtl_hdr ;
		memset ( &mtl_hdr, 0, sizeof ( TDSCAN_FILE_MATERIAL ) ) ;
		memcpy ( mtl_hdr.szSign, "MATERIAL", sizeof ( "MATERIAL" ) ) ;
		strncpy ( (char*)mtl_hdr.szName, mtrl.sName.c_str (), sizeof ( mtl_hdr.szName ) ) ;
		mtl_hdr.f4Diffuse = mtrl.clrDiffuse ;
		mtl_hdr.f4Ambient = mtrl.clrAmbient ;
		mtl_hdr.fAlpha = mtrl.fTransparency ;
		mtl_hdr.fGlossiness = mtrl.fGlossiness ;
		mtl_hdr.fSpecIntensity = mtrl.fSpecIntensity ;
		mtl_hdr.fReflectionFactor = mtrl.fReflectionFactor ;

		strncpy ( (char*)mtl_hdr.szDiffuseTex, mtrl.sDiffuseTextureName.c_str (), sizeof ( mtl_hdr.szDiffuseTex ) ) ;
		strncpy ( (char*)mtl_hdr.szAlphaTex, mtrl.sAlphaTextureName.c_str (), sizeof ( mtl_hdr.szAlphaTex ) ) ;
		strncpy ( (char*)mtl_hdr.szNormalTex, mtrl.sNormalTextureName.c_str (), sizeof ( mtl_hdr.szNormalTex) ) ;
		strncpy ( (char*)mtl_hdr.szSpecularTex, mtrl.sSpecularTextureName.c_str (), sizeof ( mtl_hdr.szSpecularTex) ) ;
		strncpy ( (char*)mtl_hdr.szReflectionTex, mtrl.sReflectionTextureName.c_str (), sizeof ( mtl_hdr.szReflectionTex ) ) ;

		if ( iMat < pModel->Materials.size () - 1 )
			mtl_hdr.iNextMaterialOfs = uiFileOfs + sizeof ( TDSCAN_FILE_MATERIAL ) ;
		else
			mtl_hdr.iNextMaterialOfs = -1 ;

		fwrite ( &mtl_hdr, sizeof ( TDSCAN_FILE_MATERIAL ), 1, pFile ) ;

		uiFileOfs += sizeof ( TDSCAN_FILE_MATERIAL ) ;
	}

	hdr.iTexLibOfs = uiFileOfs ;

	// Save Textures
	for ( uint32_t iTex = 0 ; iTex < pModel->Textures.size () ; iTex++ ) {
		auto i = pModel->Textures.begin () ;
		advance ( i, iTex ) ;

		TD_MODEL_TEXTURE_SLOT& tex = i->second ;

		TDSCAN_FILE_TEXTURE tex_hdr ;
		memset ( &tex_hdr, 0, sizeof ( TDSCAN_FILE_TEXTURE ) ) ;
		memcpy ( tex_hdr.szSign, "TEX", sizeof ( "TEX" ) ) ;
		strncpy ( (char*)tex_hdr.szName, tex.sName.c_str (), sizeof ( tex_hdr.szName ) ) ;
		tex_hdr.clrAvgColor = tex.clrAvgColor ;
		tex_hdr.eTexFormat = tex.eFormat ;
		
		tex_hdr.uiDataSize = tex.uiSize ;

		// TODO: Handle Compression
		tex_hdr.uiCompressedSize = tex.uiSize ;

		if ( iTex < pModel->Textures.size () - 1 )
			tex_hdr.iNextTexOfs = uiFileOfs + sizeof ( TDSCAN_FILE_TEXTURE ) + tex_hdr.uiCompressedSize ;
		else
			tex_hdr.iNextTexOfs = -1 ;

		fwrite ( &tex_hdr, sizeof ( TDSCAN_FILE_TEXTURE ), 1, pFile ) ;
		uiFileOfs += sizeof ( TDSCAN_FILE_TEXTURE ) ;
		
		fwrite ( tex.pData, tex_hdr.uiCompressedSize, 1, pFile ) ;
		uiFileOfs += tex_hdr.uiCompressedSize ;
	}

	fseek ( pFile, 0, SEEK_SET ) ;
	fwrite ( &hdr, sizeof(TDSCAN_FILE_HEADER), 1, pFile ) ;

	fclose ( pFile ) ;

	return pModel ;

load_error:
	if ( pFile )
		fclose ( pFile ) ;
	// 	if ( pModel )
	// 		CD3DModelUtils::FreeD3DModel ( pModel ) ;



	fclose ( pFile ) ;

	return true ;
}
