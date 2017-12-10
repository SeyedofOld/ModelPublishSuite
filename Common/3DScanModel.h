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

using namespace std ;

struct MODEL_SUBSET {
	void*		pVB ;
	uint32_t*	pIB ;
	int32_t		iTriCount ;
	string		sMatName ;
	int32_t		iMatIndex;
	uint32_t	uiVertexFmt ;
	MODEL_SUBSET() {
		pVB = NULL ;
		pIB = NULL ;
		iTriCount = 0 ;
		iMatIndex = 0 ;
		uiVertexFmt = 0 ;
	}
};

struct MODEL_PART {
	string					sName ;
	vector<MODEL_SUBSET>	Subsets ;
};

struct MODEL_MATERIAL {
	string			sName ;
	float4_rgba		clrAmbient ;
	float4_rgba		clrDiffuse ;
	float4_rgba		clrSpecular ;
	float			fTransparency ;
	float			fGlossiness ;
	string			sTextureName ;
	MODEL_MATERIAL () {
		clrAmbient	= float4_rgba{ 0, 0, 0, 0 };
		clrDiffuse	= float4_rgba{ 1, 1, 1, 1 };
		clrSpecular = float4_rgba{ 1, 1, 1, 1 };
		fTransparency	= 1.0f ;
		fGlossiness		= 0.0f ;
	}
};

struct MODEL_TEXTURE_SLOT {
	string sName ;
};

struct TDSCAN_MODEL {
	vector<MODEL_PART>				Parts ;
	map<string, MODEL_MATERIAL>		Materials;
	map<string, MODEL_TEXTURE_SLOT>	Textures ;
	float3							ptMin ;
	float3							ptMax ;
};

