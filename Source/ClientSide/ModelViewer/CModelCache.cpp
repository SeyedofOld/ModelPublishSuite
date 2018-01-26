#include "stdafx.h"

#include "CModelCache.h"

CModelCache::CModelCache()
{
	m_szCacheRoot [ 0 ] = 0 ;
	m_CacheTable.clear() ;

	m_hCacheFile = INVALID_HANDLE_VALUE ;

	m_bEnableCacheRead = true ;
	m_bEnableCacheStore = true ;
}

CModelCache::~CModelCache()
{
	CleanUp() ;
}

void CModelCache::CleanUp()
{
	m_szCacheRoot [ 0 ] = 0 ;
	m_CacheTable.clear() ;

	CloseHandle ( m_hCacheFile ) ;
	m_hCacheFile = INVALID_HANDLE_VALUE ;

	m_bEnableCacheRead = true ;
	m_bEnableCacheStore = true ;
}

bool CModelCache::Initialize ()
{
	if ( m_hCacheFile != INVALID_HANDLE_VALUE )
		return false ;

	wchar_t szFile [ MAX_PATH ] ;
	wcscpy ( szFile, m_szCacheRoot ) ;
	wcscat ( szFile, L"cache.dat" ) ;
	m_hCacheFile = CreateFileW ( szFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) ;
	if ( m_hCacheFile == INVALID_HANDLE_VALUE ) {
		CreateDirectoryW ( L"Cache", NULL ) ;
		char a = 0 ;
		if ( ! SaveMemToFile ( szFile, &a, 0 ) )
			return false ;

		m_hCacheFile = CreateFileW ( szFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) ;
		if ( m_hCacheFile == INVALID_HANDLE_VALUE )
			return false ;
	}

	DWORD dwSize = GetFileSize ( m_hCacheFile, NULL ) ;
	size_t iCount = dwSize / sizeof ( CACHE_ENTRY ) ;

	if ( iCount ) {
		m_CacheTable.resize ( iCount ) ;
		ReadFile ( m_hCacheFile, m_CacheTable.data(), iCount * sizeof(CACHE_ENTRY), NULL, NULL ) ;
	}

	return true ;
}

int CModelCache::SearchInCache ( std::wstring& strSubsid )
{
	if ( m_hCacheFile == INVALID_HANDLE_VALUE )
		return -1 ;

	if ( ! m_bEnableCacheRead )
		return -1 ;

	for ( size_t i = 0 ; i < m_CacheTable.size() ; i++ ) {

		CACHE_ENTRY& entry = m_CacheTable [ i ] ;
		if ( wcscmp ( entry.szSibscriptionId, strSubsid.c_str() ) == 0 )
			return i ;
	}

	return -1 ;
}

bool CModelCache::LoadFileToMem ( wchar_t* pszFilename, void** ppData, int* piSize )
{
	if ( ! pszFilename )
		return false ;
	if ( ! ppData )
		return false ;

	HANDLE hFile = CreateFileW ( pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL ) ;
	if ( hFile == INVALID_HANDLE_VALUE )
		return false ;

	DWORD dwFileSize = GetFileSize ( hFile, NULL ) ;

	BYTE* pBuf = new BYTE [ dwFileSize ] ;
	if ( ! pBuf ) {
		CloseHandle ( hFile ) ;
		return false ;
	}

	DWORD dwRead ;
	ReadFile ( hFile, pBuf, dwFileSize, &dwRead, NULL ) ;
	CloseHandle ( hFile ) ;

	if ( piSize )
		*piSize = dwFileSize ;

	*ppData = pBuf ;

	return true ;
}

bool CModelCache::SaveMemToFile ( wchar_t* pszFilename, void* pData, int iSize )
{
	HANDLE hFile = CreateFileW ( pszFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) 
		return false ;

	DWORD dwBytesWritten = 0 ;
	if ( ! WriteFile ( hFile, pData, iSize, &dwBytesWritten, NULL ) ) {
		CloseHandle ( hFile ) ;
		return false ;
	}

	CloseHandle ( hFile ) ;
}

void CModelCache::EnableCache( bool bEnable )
{
	EnableCacheRead ( bEnable ) ;
	EnableCacheStore ( bEnable ) ;
}

void CModelCache::EnableCacheRead( bool bEnable )
{
	m_bEnableCacheRead = bEnable ;
}

void CModelCache::EnableCacheStore( bool bEnable )
{
	m_bEnableCacheStore = bEnable ;
}

void CModelCache::SetCacheRoot ( wchar_t* pszRoot )
{
	if ( pszRoot )
		wcscpy ( m_szCacheRoot, pszRoot ) ;
}

bool CModelCache::GetEntry ( int iEntry, CACHE_ENTRY& rEntry ) 
{
	if ( iEntry < 0 || iEntry >= m_CacheTable.size () )
		return false ;

	rEntry = m_CacheTable [ iEntry ] ;

	return true ;
}

bool CModelCache::AddModelToCache ( std::wstring& strSubsid, void* pModelFileData, int iModelFileSize, void* pAdFileData, int iAdFileSize, void* pThumbFileData, int iThumbFileSize )
{
	if ( m_hCacheFile == INVALID_HANDLE_VALUE )
		return false ;
	if ( ! pModelFileData )
		return false ;

	if ( ! m_bEnableCacheStore )
		return false ;

	int iSearchResult = SearchInCache ( strSubsid ) ;
	if ( iSearchResult != -1 )
		return false ;

	wchar_t szModelFile [ MAX_PATH ] ;
	if ( 0 == GetTempFileNameW ( m_szCacheRoot, L"", 0, szModelFile ) )
		return false ;
	wchar_t szAdFile [ MAX_PATH ] ;
	if ( 0 == GetTempFileNameW ( m_szCacheRoot, L"", 0, szAdFile ) )
		return false ;
	wchar_t szThumbFile [ MAX_PATH ] ;
	if ( 0 == GetTempFileNameW ( m_szCacheRoot, L"", 0, szThumbFile ) )
		return false ;

	if ( ! SaveMemToFile ( szModelFile, pModelFileData, iModelFileSize ) )
		return false ;
		
	if ( pAdFileData )
		if ( ! SaveMemToFile ( szAdFile, pAdFileData, iAdFileSize ) )
			return false ;

	if ( pThumbFileData )
		if ( ! SaveMemToFile ( szThumbFile, pThumbFileData, iThumbFileSize ) )
			return false ;

	CACHE_ENTRY entry ;
	ZeroMemory ( &entry, sizeof(CACHE_ENTRY) ) ;
	wcscpy ( entry.szSibscriptionId, strSubsid.c_str() ) ;
	wcscpy ( entry.szModelFile, szModelFile ) ;
	wcscpy ( entry.szAdFile, szAdFile ) ;
	wcscpy ( entry.szThumbFile, szThumbFile ) ;
	GetLocalTime ( &entry.DateTime ) ;

	m_CacheTable.push_back ( entry ) ;

	DWORD dwBytesWritten ;
	SetFilePointer ( m_hCacheFile, 0, NULL, FILE_END ) ;
	WriteFile ( m_hCacheFile, &entry, sizeof ( CACHE_ENTRY ), &dwBytesWritten, NULL ) ;

	return true ;
}

bool CModelCache::LoadModel ( std::wstring& strSubsid, void** ppModelFileData, int* piModelFileSize, void** ppAdFileData, int* piAdFileSize, void** ppThumbFileData, int* piThumFileSize )
{
	if ( m_hCacheFile == INVALID_HANDLE_VALUE )
		return false ;

	if ( ! ppAdFileData || ! ppModelFileData || ! ppThumbFileData )
		return false ;

	if ( ! m_bEnableCacheRead )
		return false ;

	int iSearchResult = SearchInCache ( strSubsid ) ;

	if ( iSearchResult != -1 ) {
		CACHE_ENTRY& entry = m_CacheTable [ iSearchResult ] ;

		if ( ! LoadFileToMem ( entry.szModelFile, ppModelFileData, piModelFileSize ) )
			return false ;

		if ( ! LoadFileToMem ( entry.szAdFile, ppAdFileData, piAdFileSize ) ) {
			*ppAdFileData = NULL ;
			if ( piAdFileSize )
				*piAdFileSize = 0 ;
		}

		if ( ! LoadFileToMem ( entry.szThumbFile, ppThumbFileData, piThumFileSize ) ) {
			if ( *ppThumbFileData )
				delete *ppThumbFileData ;
			if ( piThumFileSize )
				*piThumFileSize = 0 ;
		}

		return true ;
	}

	return false ;
}

bool CModelCache::AddAdToCache ( std::wstring& strSubsid, void* pAdFileData, int iAdFileSize )
{
	if ( m_hCacheFile == INVALID_HANDLE_VALUE )
		return false ;
	if ( ! pAdFileData )
		return false ;

	if ( ! m_bEnableCacheStore )
		return false ;

	int iSearchResult = SearchInCache ( strSubsid ) ;
	if ( iSearchResult == -1 )
		return false ;

	CACHE_ENTRY& entry = m_CacheTable [ iSearchResult ] ;

	if ( ! SaveMemToFile ( entry.szAdFile, pAdFileData, iAdFileSize ) )
		return false ;

	return true ;
}
