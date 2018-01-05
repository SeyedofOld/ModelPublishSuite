/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Base stream class
*********************************************************************/

#include "CMemStream.h"
#include <memory.h>

CMemStream::CMemStream()
{
	m_pMem = NULL ;
	m_uiSize = 0 ;
	m_uiCurPos = 0 ;
}

CMemStream::~CMemStream()
{
	Close() ;
}

bool CMemStream::Bind ( void* pMem, uint32_t uiSize )
{
	if ( ! pMem )
		return false ;

	m_pMem = (char*)pMem ;
	m_uiSize = uiSize ;
	m_uiCurPos = 0 ;

	return true ;
}

uint32_t CMemStream::Seek ( int32_t iOfs, uint32_t uiOrig )
{
	if ( ! m_pMem )
		return 0 ;

	int32_t iNewPos = 0 ;
	if ( uiOrig == SEEK_SET )
		iNewPos = iOfs ;
	else if ( uiOrig == SEEK_END )
		iNewPos = m_uiSize + iOfs ;
	else if ( uiOrig == SEEK_CUR )
		iNewPos = m_uiCurPos + iOfs ;

	if ( iNewPos < 0 )
		iNewPos = 0 ;
	if ( iNewPos >= m_uiSize )
		iNewPos = m_uiSize - 1 ;

	m_uiCurPos = iNewPos ;

	return m_uiCurPos ;
}

uint32_t CMemStream::Tell()
{
	return m_uiCurPos ;
}

void CMemStream::Close ()
{
	m_pMem = NULL ;
	m_uiSize = 0 ;
	m_uiCurPos = 0 ;
}

uint32_t CMemStream::Read ( void* pBuf, uint32_t uiSize )
{
	if ( ! m_pMem || ! pBuf )
		return 0 ;

	uint32_t uiEnd = m_uiCurPos + uiSize ;
	if ( uiEnd >= m_uiSize )
		uiSize = m_uiSize - m_uiCurPos ;

	memcpy ( pBuf, m_pMem + m_uiCurPos, uiSize ) ;
	
	m_uiCurPos += uiSize ;

	return uiSize ;
}

uint32_t CMemStream::Write ( void* pBuf, uint32_t uiSize )
{
	if ( !m_pMem || !pBuf )
		return 0 ;

	uint32_t uiEnd = m_uiCurPos + uiSize ;
	if ( uiEnd >= m_uiSize )
		uiSize = m_uiSize - m_uiCurPos ;

	memcpy ( m_pMem + m_uiCurPos, pBuf, uiSize ) ;

	m_uiCurPos += uiSize ;

	return uiSize ;
}

