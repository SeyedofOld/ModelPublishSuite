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

struct D3DMODEL_MATERIAL  {
	TD_MODEL_MATERIAL*	pBase ;
	ID3DXEffect*		pShader ;
// 	IDirect3DTexture9* pTexDiffuse ;
// 	IDirect3DTexture9* pTexSpec;
// 	IDirect3DTexture9* pTexAlpha;
// 	IDirect3DTexture9* pTexNormal;
// 	IDirect3DTexture9* pTexReflection ;

	D3DMODEL_MATERIAL() {
		pBase = NULL ;
		pShader = NULL ;
// 		pTexDiffuse = NULL ;
// 		pTexAlpha = NULL ;
// 		pTexSpec = NULL ;
// 		pTexNormal = NULL ;
// 		pTexReflection = NULL ;
	}
};

struct D3DMODEL_TEXTURE_SLOT {
	TD_MODEL_TEXTURE_SLOT* pBase ;
	IDirect3DTexture9* pTexture;

	D3DMODEL_TEXTURE_SLOT () {
		pBase = NULL ;
		pTexture = NULL ;
	}
};

struct D3DMODEL_SUBSET {
	TD_MODEL_SUBSET* pBase ;
	uint32_t	uiFVF ;
	//int32_t		iMatIndex ;
	bool		bOwnMaterial ;
	D3DMODEL_MATERIAL Material ;
	bool		bVisible ;
	bool		bSelected ;

	D3DMODEL_SUBSET () {
		pBase = NULL ;
		uiFVF = 0 ;
		//iMatIndex = -1 ;
		bOwnMaterial = false ;
		bVisible = true ;
		bSelected = false ;
	}
};

struct D3DMODEL_PART {
	TD_MODEL_PART* pBase ;
	bool bVisible ;
	bool bSelected ;
	vector<D3DMODEL_SUBSET> Subsets ;
	D3DMODEL_PART () {
		pBase = NULL ;
		bVisible = true ;
		bSelected = false ;
	}
};

struct D3D_MODEL {
	vector<D3DMODEL_PART>				Parts;
	map<string, D3DMODEL_MATERIAL>		Materials;
	map<string, D3DMODEL_TEXTURE_SLOT>	Textures;
	float3								ptMin;
	float3								ptMax;
};

