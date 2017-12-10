/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "C3DScanFile.h"
#include "C3DScanFileUtils.h"

TDSCAN_MODEL* C3DScanFile::Load3DScanModel ( char* pszFilename )
{
	if ( ! pszFilename )
		return NULL;

	FILE * pFile = fopen ( pszFilename, "rb" ) ;
	if ( ! pFile )
		return NULL ;
	
	TDSCAN_FILE_HEADER hdr;
	fread ( &hdr, sizeof(TDSCAN_FILE_HEADER), 1, pFile ) ;
	if ( ! memcmp ( hdr.szSign, "SEYEDOF 3D FORMAT", sizeof(hdr.szSign) ) != 0 ) 
		goto load_error ;

	if ( hdr.uiVersion != (1 << 16) + 0 )
		goto load_error ;

	TDSCAN_MODEL* pModel = new TDSCAN_MODEL ;
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
			MODEL_PART part ;
			pModel->Parts.push_back ( part ) ;
		}
		MODEL_PART& part = pModel->Parts.front() ;

		part.sName = (char*)part_hdr.szName ;
		
		int32_t iNextSubsetOfs = part_hdr.iFirstSubset ;

		while ( iNextSubsetOfs > 0 ) {

			{
				MODEL_SUBSET subset;
				part.Subsets.push_back ( subset ) ;
			}
			MODEL_SUBSET& subset = part.Subsets.front() ;

			fseek ( pFile, iNextPartOfs, SEEK_SET ) ;

			TDSCAN_FILE_SUBSET sub_hdr;
			fread ( &sub_hdr, sizeof(TDSCAN_FILE_SUBSET), 1, pFile ) ;
			if ( memcmp ( sub_hdr.szSign, "SUBSET", sizeof(sub_hdr.szSign) ) != 0 )
				goto load_error ;

			uint32_t uiVertexSize = C3DScanFileUtils::GetVertexSize ( sub_hdr.uiVertexFormat ) ;
			if ( sub_hdr.uiDataSize != uiVertexSize * sub_hdr.iVertexCount + sub_hdr.iFaceCount * 3 * sizeof(uint32_t) )
				goto load_error ;

			subset.pVB = new int8_t [ uiVertexSize * sub_hdr.iVertexCount ] ;
			if ( ! subset.pVB )
				goto load_error ;

			subset.pIB = new uint32_t [ sizeof(uint32_t) * sub_hdr.iFaceCount * 3 ] ;
			if ( ! subset.pIB ) 
				goto load_error;

			fread ( subset.pVB, uiVertexSize, sub_hdr.iVertexCount, pFile ) ;
			fread ( subset.pIB, sizeof(uint32_t), sub_hdr.iFaceCount * 3, pFile ) ;

			sub_hdr.iMaterialIndex = 

			iNextSubsetOfs = sub_hdr.iNextSubsetOfs ;
		}

		iNextPartOfs = part_hdr.iNextPartOfs ;
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
