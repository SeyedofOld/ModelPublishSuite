/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Base stream class
*********************************************************************/

#pragma once

#include <stdint.h>
#include <stdio.h>

class CBaseStream {

public:
	CBaseStream() {} ;
	virtual ~CBaseStream() {} ;

	virtual uint32_t Seek ( int32_t iOfs, uint32_t uiOrig ) = 0 ;
	virtual uint32_t Tell() = 0 ;
	virtual void Close() = 0 ;
	virtual uint32_t Read ( void* pBuf, uint32_t uiSize ) = 0 ;
	virtual uint32_t Write ( void* pBuf, uint32_t uiSize ) = 0 ;
};