#pragma once


#include <windows.h>
#include "D3DModel.h"

class CD3DModelUtils {
public:
	static bool CreateFromTDModel ( IDirect3DDevice9* pDevice, ID3DXEffectPool* pEffectPool, TD_SCAN_MODEL& model, D3D_MODEL& d3dModel ) ;
	static bool RenderD3DModel ( IDirect3DDevice9* pDevice, D3D_MODEL& d3dModel ) ;
	static void FreeD3DModel ( D3D_MODEL& d3dModel ) ;
	static uint32_t VertexFormatToFvf ( uint32_t uiVertFmt ) ;
};



