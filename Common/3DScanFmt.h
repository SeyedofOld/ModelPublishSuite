/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format structures
*********************************************************************/

#pragma once

#include <stdint.h>
#include <d3dx9.h>

#include <pshpack1.h>

enum TDSCAN_FLAGS {
	TDS_FLAG_NONE = 0,
	TDS_FLAG_COMPRESSED = 1,
	TDS_FLAG_ENCRYPTED = 2
} ;

enum TEXTURE_FORMAT {
	TEX_JPG = 0,
	TEX_PNG = 1,
	TEX_DDS = 2,
	TEX_BMP = 3,
	TEX_TIFF = 4,
	TEX_JP2 = 5,
	TEX_TGA = 6,
	TEX_ETC1 = 7,
	TEX_ETC2 = 8,
	TEX_EAC = 9
} ;

enum VERTEX_FORMAT {
	VF_POSITIION = 1,
	VF_NORMAL = 2,
	VF_UV = 4
} ;

struct float3 {
	float x, y, z ;
} ;

struct float2 {
	float x, y ;
} ;

struct float2_uv {
	float u, v ;
} ;

struct float4_rgba {
	float r, g, b, a ;
} ;

struct TDSCAN_FILE_HEADER {
	uint8_t		szSign [ 17 ] ; // = SEYEDOF 3D FORMAT
	uint16_t	uiVersion ;
	uint32_t	uiTotalTriCount ;
	int32_t		iFirstPartOfs ;
	int32_t		iMaterialLibOfs ;
	int32_t		iTexLibOfs ;
	uint8_t		uiPadding [ 5 ] ;
} ;

struct TDSCAN_PART {
	uint8_t		szSign [ 4 ] ; // = PART
	int8_t		szName [ 32 ] ;
	int32_t		iNextPartOfs ;
	int32_t		iFirstSubset ;
} ;

struct TDSCAN_MATERIAL {
	uint8_t		szSign [ 8 ] ; // = MATERIAL
	int32_t		iNextMaterialOfs ;
	float4_rgba f4Diffuse ;
	float4_rgba f4Ambient ;
	float		fGlossiness ;
	float		fSpecIntensity ;
	float		fAlpha ;
	float		fReflectionFactor ;
	uint32_t	uiFlags ;
	int8_t		szDiffuseTex [ 32 ] ;
	int8_t		szAlphaTex [ 32 ] ;
	int8_t		szReflectionTex [ 32 ] ;
	int8_t		szSpecularTex [ 32 ] ;
	int8_t		szNormalTex [ 32 ] ;
	uint8_t		uiPadding [ 64 ] ;
} ;

struct TDSCAN_TEXTURE {
	uint8_t			szSign [ 3 ] ; // = TEX
	int8_t			szName [ 32 ] ;
	int32_t			iNextTexOfs ;
	uint32_t		iSize ;
	TEXTURE_FORMAT	eTexFormat ;
	uint32_t		uiDataSize ;
	uint32_t		uiCompressedSize ;
	uint32_t		uiFlags ;
	uint8_t			uiPadding [ 5 ] ;
} ;

struct TDSCAN_SUBSET {
	uint8_t		szSign [ 6 ] ; // = SUBSET
	int32_t		iNextSubsetOfs ;
	uint32_t	iMaterialIndex ;
	uint32_t	iVertexCount ;
	uint32_t	iFaceCount ;
	uint32_t	uiVertexFormat ;
	uint32_t	uiDataSize ;
	uint32_t	uiCompressedSize ;
	uint32_t	uiFlags ;
	uint8_t		uiPadding [ 10 ] ;
} ;

#include <poppack.h>

