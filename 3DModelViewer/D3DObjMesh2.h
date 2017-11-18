// This header contains stuff to facilitate drawing obj meshes using direct3d.

#pragma once


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "ObjLoader.h"
//# include "Win32Util.h"


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

	ID3DXEffect* pShader ;

	D3DXCOLOR clrAmbient ;
	D3DXCOLOR clrDiffuse ;
	D3DXCOLOR clrSpecular ;
	float     fTransparency ;
	float	  fGlossiness ;
	IDirect3DTexture9* pTexDiffuse ;

	D3D_DRAW_BATCH () {
		pVB = NULL ;
		iTriCount = 0 ;
		bNormal = false ;
		bUv = false ;
		pShader = NULL ;
		pTexDiffuse = NULL ;
		clrAmbient = D3DXCOLOR ( 0, 0, 0, 0 ) ;
		clrDiffuse = D3DXCOLOR ( 1, 1, 1, 1 ) ;
		clrSpecular = D3DXCOLOR ( 1, 1, 1, 1 ) ;
		fTransparency = 1.0f ;
		fGlossiness = 0.0f ;
	}
};

struct D3D_MODEL_PART {
	std::string sName ;
	vector<D3D_DRAW_BATCH> Batches ;
};

struct D3D_MODEL {
	vector<D3D_MODEL_PART> Parts ;
	D3DXVECTOR3 ptMin ;
	D3DXVECTOR3 ptMax ;
};



class CD3DMesh2
{
public:
	CD3DMesh2()
	{
// 		vertexSize = 0;
// 		FVF = 0;
// 		pVB = NULL;
// 		pTex = NULL;
	}

	~CD3DMesh2() { /*SAFE_RELEASE( pVB );*/ }

	static bool CreateFromObj ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, MY_OBJ& Obj, D3D_MODEL& d3dModel ) ;
	static bool RenderD3DMesh ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel ) ;
// 	HRESULT Create ( LPDIRECT3DDEVICE9 pD3DDevice, const CObjMesh& objMesh, BOOL flipTriangles, BOOL flipUVs );
// 
// 
// 	UINT triCount;
// 	UINT vertexSize;
// 	DWORD FVF;
// 	LPDIRECT3DVERTEXBUFFER9 pVB;
// 	D3DXVECTOR3 bbmin, bbmax; // bounding box.
// 	IDirect3DTexture9* pTex ;

protected:
	// This method is probably the most important in the sample after the obj loader.
	// This methdo is called by the Create() method to construct the D3D vertex buffer and fill it
	// with the triangles of the obj mesh. Each triangle has its own copy of the vertices, so it's not
	// really very efficient. Instead, in a real world application, the vertex buffer should only contain
	// unique vertices, and an index buffer is neaded to create the triangles. That is, if many face
	// vertices are identical (in ALL their components), only one corresponding vertex needs to be copied
	// to the vertex buffer.
	// This optimization requires sorting and/or searching which can be quite slow for heavy meshes.
	//HRESULT InitVB( LPDIRECT3DDEVICE9 pD3DDevice, const CObjMesh& objMesh, BOOL flipTriangles, BOOL flipUVs );
};



