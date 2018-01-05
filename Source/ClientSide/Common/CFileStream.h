/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Base stream class
*********************************************************************/

#pragma once

#include "CBaseStream.h"

class CFileStream : public CBaseStream {

public:
	CFileStream () ;
	virtual ~CFileStream () ;

	bool Open ( char* pszFilename ) ;
	bool Open ( wchar_t* pszFilename ) ;

	virtual uint32_t Seek ( int32_t iOfs, uint32_t uiOrig ) ;
	virtual uint32_t Tell () ;
	virtual void Close () ;
	virtual uint32_t Read ( void* pBuf, uint32_t uiSize ) ;
	virtual uint32_t Write ( void* pBuf, uint32_t uiSize ) ;

protected:
	FILE* m_pFile ;
};

