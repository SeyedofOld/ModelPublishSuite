/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanFmt.h"
#include "D3DModel.h"

class C3DScanFile {

public:
	static D3D_MODEL* Load3DScanModel ( char* pszFilename) ;
	static bool Save3DScanModel ( char* pszFilename ) ;

};