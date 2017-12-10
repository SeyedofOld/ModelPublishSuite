/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanFmt.h"
#include "3DScanModel.h"

class C3DScanFile {

public:
	static TDSCAN_MODEL* Load3DScanModel ( char* pszFilename ) ;
	static bool Save3DScanModel ( char* pszFilename ) ;

};