/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	3dscan file format handler
*********************************************************************/

#pragma once

#include "3DScanFmt.h"
#include "3DScanModel.h"
#include "CBaseStream.h"

#define TD_SAVE_FLAG_NO_COMPRESSION 1

class C3DScanFile {

public:
	static TD_SCAN_MODEL* Load3DScanModelFromFile ( char* pszFilename ) ;
	static TD_SCAN_MODEL* Load3DScanModelFromMemory ( void* pMem, uint32_t uiSize ) ;
	static TD_SCAN_MODEL* Load3DScanModel ( CBaseStream& rStream ) ;
	static bool Save3DScanModel ( char* pszFilename, TD_SCAN_MODEL* pModel, uint32_t uiFlags = 0 ) ;

};