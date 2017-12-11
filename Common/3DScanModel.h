/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3D model in-memory structures
*********************************************************************/

#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include "BaseTypes.h"
#include "3DScanFmt.h"

using namespace std ;

struct TD_MODEL_SUBSET {
	void*		pVB ;
	uint32_t*	pIB ;
	int32_t		iTriCount ;
	string		sMatName ;
	//int32_t		iMatIndex;
	uint32_t	uiVertexFmt ;
	TD_MODEL_SUBSET() {
		pVB = NULL ;
		pIB = NULL ;
		iTriCount = 0 ;
		//iMatIndex = 0 ;
		uiVertexFmt = 0 ;
	}
};

struct TD_MODEL_PART {
	string					sName ;
	vector<TD_MODEL_SUBSET>	Subsets ;
};

struct TD_MODEL_MATERIAL {
	string			sName ;
	float4_rgba		clrAmbient ;
	float4_rgba		clrDiffuse ;
	float			fTransparency ;
	float			fSpecIntensity ;
	float			fGlossiness ;
	string			sDiffuseTextureName ;
	string			sNormalTextureName ;
	string			sSpecularTextureName ;
	string			sAlphaTextureName ;
	string			sReflectionTextureName ;
	TD_MODEL_MATERIAL () {
		clrAmbient	= float4_rgba{ 0, 0, 0, 1 };
		clrDiffuse	= float4_rgba{ 1, 1, 1, 1 };
		fSpecIntensity  = 1.0f ;
		fTransparency	= 1.0f ;
		fGlossiness		= 10.0f ;
	}
};

struct TD_MODEL_TEXTURE_SLOT {
	string			sName ;
	uint32_t		uiSize ;
	void*			pData ;
	TEXTURE_FORMAT	eFormat ;
	TD_MODEL_TEXTURE_SLOT()	{
		uiSize = 0 ;
		pData = NULL ;
		eFormat = TEX_UNKNWON ;
	}
};

struct TD_SCAN_MODEL {
	vector<TD_MODEL_PART>				Parts ;
	map<string, TD_MODEL_MATERIAL>		Materials;
	map<string, TD_MODEL_TEXTURE_SLOT>	Textures ;
	float3								ptMin ;
	float3								ptMax ;
};

void FreeModel ( TD_SCAN_MODEL& model ) ;
