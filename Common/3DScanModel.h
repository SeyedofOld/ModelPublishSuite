#pragma once


#include <windows.h>
#include <vector>
#include <map>
#include <stdint.h>
#include "BaseTypes.h"

#ifndef SAFE_DELETE
/// For pointers allocated with new.
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
/// For arrays allocated with new [].
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#endif

using namespace std ;

struct MODEL_SUBSET {
	void*		pVB ;
	uint32_t*	pIB ;
	int32_t		iTriCount ;

	//bool		bNormal ;
	//bool		bUv ;
	//string		sMatName ;
	int32_t		iMatIndex;
	uint32_t	uiVertexFmt ;
//	uint32_t	uiFVF ;

	MODEL_SUBSET() {
		pVB = NULL ;
		pIB = NULL ;
		iTriCount = 0 ;
		iMatIndex = 0 ;
// 		bNormal = false ;
// 		bUv = false ;
		uiVertexFmt = 0 ;
//		uiFVF = 0 ;
	}
};

struct MODEL_PART {
	string					sName ;
	vector<MODEL_SUBSET>	Subsets ;
};

struct MODEL_MATERIAL {
	string			sName ;
	//ID3DXEffect*	pShader ;

	float4_rgba		clrAmbient ;
	float4_rgba		clrDiffuse ;
	float4_rgba		clrSpecular ;
	float			fTransparency ;
	float			fGlossiness ;
	//IDirect3DTexture9* pTexDiffuse ;
	string			sTextureName ;
	MODEL_MATERIAL () {
		//pShader		= NULL ;
		//pTexDiffuse = NULL ;
		clrAmbient	= float4_rgba{ 0, 0, 0, 0 };
		clrDiffuse	= float4_rgba{ 1, 1, 1, 1 };
		clrSpecular = float4_rgba{ 1, 1, 1, 1 };
		fTransparency	= 1.0f ;
		fGlossiness		= 0.0f ;
	}
};

struct MODEL_TEXTURE_SLOT {
	string sName ;
	//IDirect3DTexture9* pTexture;
};

struct TDSCAN_MODEL {
	vector<MODEL_PART>				Parts ;
	map<string, MODEL_MATERIAL>		Materials;
	map<string, MODEL_TEXTURE_SLOT>	Textures ;
	float3							ptMin ;
	float3							ptMax ;
};

