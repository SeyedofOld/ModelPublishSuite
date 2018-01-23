#pragma once

#include <stdint.h>
#include <vector>

class CModelCache
{
#include <pshpack8.h>
	struct CACHE_ENTRY {
		wchar_t		szSibscriptionId [ MAX_PATH ] ;
		SYSTEMTIME	DateTime ;
		uint32_t	dwHash ;
		wchar_t		szModelFile [ MAX_PATH ] ;
		wchar_t		szAdFile [ MAX_PATH ] ;
		wchar_t		szThumbFile [ MAX_PATH ] ;
	} ;
#include <poppack.h>

public:
	CModelCache();
	~CModelCache();

	void CleanUp() ;

	bool Initialize() ;

	bool AddModelToCache ( std::wstring& strSubsid, void* pModelFileData, int iModelFileSize, void* pAdFileData, int iAdFileSize, void* pThumbFileData, int iThumbFileSize ) ;
	bool LoadModel ( std::wstring& strSubsid, void** ppModelFileData, int* piModelFileSize, void** ppAdFileData, int* piAdFileSize, void** ppThumbFileData, int* piThumFileSize ) ;

	void EnableCache ( bool bEnable ) ;
	void EnableCacheRead ( bool bEnable ) ;
	void EnableCacheStore ( bool bEnable ) ;
	void SetCacheRoot ( wchar_t* pszRoot ) ;

private:
	int SearchInCache ( std::wstring& strSubsid ) ;
	bool LoadFileToMem ( wchar_t* pszFilename, void** ppData, int* piSize ) ;
	bool SaveMemToFile ( wchar_t* pszFilename, void* pData, int iSize ) ;

private:
	wchar_t m_szCacheRoot [ MAX_PATH ] ;
	HANDLE m_hCacheFile ;

	bool m_bEnableCacheStore ;
	bool m_bEnableCacheRead ;

	std::vector<CACHE_ENTRY> m_CacheTable ;
};
