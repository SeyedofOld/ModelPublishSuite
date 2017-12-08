#pragma once


#include <windows.h>
#include "D3DModel.h"

class CD3DModelUtils {
public:
	//static bool CreateFromObj ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, MY_OBJ& Obj, D3D_MODEL& d3dModel ) ;
	static bool RenderD3DModel ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel ) ;
	static void FreeD3DModel ( D3D_MODEL& d3dModel ) ;
};



