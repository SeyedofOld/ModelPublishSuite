/********************************************************************
*	Created:	2017/12/05
*	Author:		Ali Seyedof (seyedof@gmail.com)
*	Purpose:	Base stream class
*********************************************************************/

#include "CFileStream.h"

CFileStream::CFileStream()
{
	m_pFile = NULL ;
}

CFileStream::~CFileStream()
{
	Close() ;
}

bool CFileStream::Open ( char* pszFilename )
{
	if ( ! pszFilename )
		return false ;

	m_pFile = fopen ( pszFilename, "rb" ) ;
	if ( ! m_pFile )
		return false ;

	return true ;
}

uint32_t CFileStream::Seek ( int32_t iOfs, uint32_t uiOrig )
{
	if ( ! m_pFile )
		return 0 ;

	return fseek ( m_pFile, iOfs, uiOrig ) ;
}

uint32_t CFileStream::Tell()
{
	return ftell ( m_pFile ) ;
}

void CFileStream::Close ()
{
	fclose ( m_pFile ) ;
}

uint32_t CFileStream::Read ( void* pBuf, uint32_t uiSize )
{
	if ( ! m_pFile || ! pBuf )
		return 0 ;

	return (uint32_t)fread ( pBuf, uiSize, 1, m_pFile ) ;
}

uint32_t CFileStream::Write ( void* pBuf, uint32_t uiSize )
{
	if ( ! m_pFile || ! pBuf )
		return 0 ;

	return (uint32_t)fwrite ( pBuf, uiSize, 1, m_pFile ) ;
}

