/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanFmt.h"
#include <vector>
#include <map>

using namespace std ;

struct TDSCAN_VERTEX {
	float3 ptPos;
	float3 vNormal;
	float2 vUv;
};

struct TDSCAN_SUBSET {
	void*		pVB;
	uint32_t*	pIB;
	int32_t		iTriCount;

	//bool		bNormal ;
	//bool		bUv ;
	//string		sMatName ;
	int32_t		iMatIndex;
	uint32_t	uiVertexFmt;
	uint32_t	uiFVF;

	TDSCAN_SUBSET() {
		pVB = NULL;
		pIB = NULL;
		iTriCount = 0;
		iMatIndex = 0;
		// 		bNormal = false ;
		// 		bUv = false ;
		uiVertexFmt = 0;
		uiFVF = 0;
	}
};

struct TDSCAN_MODEL_PART {
	string				sName;
	vector<TDSCAN_SUBSET>	Subsets;
};

struct TDSCAN_MATERIAL {
	string			sName;
	//ID3DXEffect*	pShader;

	float4_rgba		clrAmbient;
	float4_rgba		clrDiffuse;
	float4_rgba		clrSpecular;
	float			fTransparency;
	float			fGlossiness;
	//IDirect3DTexture9* pTexDiffuse ;
	string			sTextureName;
	TDSCAN_MATERIAL() {
		//pShader = NULL;
		//pTexDiffuse = NULL ;
		clrAmbient = float4_rgba{ 0, 0, 0, 0 };
		clrDiffuse = float4_rgba{ 1, 1, 1, 1 };
		clrSpecular = float4_rgba{ 1, 1, 1, 1 };
		fTransparency = 1.0f;
		fGlossiness = 0.0f;
	}
};

struct TDSCAN_TEXTURE_SLOT {
	string sName;
	IDirect3DTexture9* pTexture;
};

struct TDSCAN_MODEL {
	vector<TDSCAN_MODEL_PART>			Parts;
	map<string, TDSCAN_MATERIAL>		Materials;
	map<string, TDSCAN_TEXTURE_SLOT>	Textures;
	float3								ptMin;
	float3								ptMax;
};



class C3DScanModel {

public:
	static bool Load3DScanModel ( char* pszFilename ) ;
	static bool Save3DScanModel ( char* pszFilename ) ;

};