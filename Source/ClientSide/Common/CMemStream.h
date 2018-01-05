/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Base stream class
*********************************************************************/

#pragma once

#include "CBaseStream.h"

class CMemStream : public CBaseStream {

public:
	CMemStream() ;
	virtual ~CMemStream() ;

	bool Bind ( void* pMem, uint32_t uiSize ) ;

	virtual uint32_t Seek ( int32_t iOfs, uint32_t uiOrig ) ;
	virtual uint32_t Tell () ;
	virtual void Close () ;
	virtual uint32_t Read ( void* pBuf, uint32_t uiSize ) ;
	virtual uint32_t Write ( void* pBuf, uint32_t uiSize ) ;

protected:
	char*		m_pMem ;
	uint32_t	m_uiSize ;
	uint32_t	m_uiCurPos ;
};

