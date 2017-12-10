/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	D3D model structures
*********************************************************************/

#pragma once


#include <windows.h>
#pragma warning( push )  
#pragma warning(disable:4005)
#include <d3d9.h>
#include <d3dx9.h>
#pragma warning(pop)
#include "3DScanModel.h"


/*#ifndef SAFE_DELETE
/// For pointers allocated with new.
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
/// For arrays allocated with new [].
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
/// For use with COM pointers.
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif*/

using namespace std ;

struct D3D_VERTEX {
	float3 ptPos ;
	float3 vNormal ;
	float2 vUv ;
} ;

struct D3DMODEL_SUBSET : public MODEL_SUBSET {
	uint32_t	uiFVF ;

	D3DMODEL_SUBSET() {
		uiFVF = 0 ;
	}
};

struct D3DMODEL_PART : public MODEL_PART {
};

struct D3DMODEL_MATERIAL {
	ID3DXEffect*	pShader ;
	IDirect3DTexture9* pTexDiffuse ;
	IDirect3DTexture9* pTexSpec;
	IDirect3DTexture9* pTexAlpha;
	IDirect3DTexture9* pTexNormal;
	IDirect3DTexture9* pTexReflection ;

	D3DMODEL_MATERIAL() {
		pShader = NULL ;
		pTexDiffuse = NULL ;
		pTexAlpha = NULL ;
		pTexSpec = NULL ;
		pTexNormal = NULL ;
		pTexReflection = NULL ;
	}
};

struct D3DMODEL_TEXTURE_SLOT : public MODEL_TEXTURE_SLOT {
	IDirect3DTexture9* pTexture;
};

struct D3D_MODEL {
	vector<D3DMODEL_PART>				Parts;
	map<string, D3DMODEL_MATERIAL>		Materials;
	map<string, D3DMODEL_TEXTURE_SLOT>	Textures;
	float3								ptMin;
	float3								ptMax;
};

