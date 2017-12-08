/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanFmt.h"



class C3DScanModel {

public:
	static bool Load3DScanModel ( char* pszFilename ) ;
	static bool Save3DScanModel ( char* pszFilename ) ;

};