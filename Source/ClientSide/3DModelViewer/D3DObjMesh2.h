// This header contains stuff to facilitate drawing obj meshes using direct3d.

#pragma once


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "ObjLoader.h"
#include <map>

#ifndef SAFE_DELETE
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
#endif

#ifndef MIN
#define MIN(x,y)				( (x)<(y)? (x) : (y) )
#endif

#ifndef MAX
#define MAX(x,y)				( (x)>(y)? (x) : (y) )
#endif

using namespace std ;

struct MY_VERTEX {
	D3DVECTOR ptPos ;
	D3DVECTOR vNormal ;
	D3DXVECTOR2 vUv ;
} ;

struct D3D_DRAW_BATCH {
	MY_VERTEX* pVB ;
	int iTriCount ;

	bool bNormal ;
	bool bUv ;
	string sMatName ;

	D3D_DRAW_BATCH() {
		pVB = NULL ;
		iTriCount = 0 ;
		bNormal = false ;
		bUv = false ;
	}
};

struct D3D_MODEL_PART {
	std::string sName ;
	vector<D3D_DRAW_BATCH> Batches ;
};

struct D3D_MATERIAL {
	string sName ;
	ID3DXEffect* pShader ;

	D3DXCOLOR clrAmbient ;
	D3DXCOLOR clrDiffuse ;
	D3DXCOLOR clrSpecular ;
	float     fTransparency ;
	float	  fGlossiness ;
	IDirect3DTexture9* pTexDiffuse ;
	D3D_MATERIAL () {
		pShader = NULL ;
		pTexDiffuse = NULL ;
		clrAmbient = D3DXCOLOR ( 0, 0, 0, 0 ) ;
		clrDiffuse = D3DXCOLOR ( 1, 1, 1, 1 ) ;
		clrSpecular = D3DXCOLOR ( 1, 1, 1, 1 ) ;
		fTransparency = 1.0f ;
		fGlossiness = 0.0f ;
	}
};

struct D3D_MODEL {
	vector<D3D_MODEL_PART>	 Parts ;
	map<string,D3D_MATERIAL> Materials ;
	D3DXVECTOR3 ptMin ;
	D3DXVECTOR3 ptMax ;
};

class CD3DMesh2
{
public:
	CD3DMesh2 (){} ;

	~CD3DMesh2() {}

	static bool CreateFromObj ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, MY_OBJ& Obj, D3D_MODEL& d3dModel ) ;
	static bool RenderD3DMesh ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel ) ;
	static void FreeModel ( D3D_MODEL& d3dModel ) ;
};



