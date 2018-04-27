#pragma once

#include "cpprest/http_msg.h"
#include "cpprest/http_client.h"

#define WM_USER_HTTP_PROGRESS		WM_USER+100
#define WM_USER_MODEL_INFO			WM_USER+150
#define WM_USER_MODEL_DOWNLOADED	WM_USER+200
#define WM_USER_AD_DOWNLOADED		WM_USER+300
#define WM_USER_MODEL_UPLOAD		WM_USER+400

class CModelServiceWebClient 
{
public:
	CModelServiceWebClient() {} ;
	virtual ~CModelServiceWebClient() {} ;

	static bool GetModel ( wchar_t* pszUrl, char* pszClientId, char** ppData, int* piSize, int iInstanceId = 0 ) ;
	static bool GetModelInfo ( wchar_t* pszUrl, char* pszClientId, char** ppData, int* piSize, int* piFileSize, int iInstanceId = 0 ) ;
	static bool GetAd ( wchar_t* pszUrl, char* pszClientId, char** ppData, int& riSize, std::string& strUrl, int iInstanceId = 0 ) ;
	static void ProgressCallback ( web::http::message_direction::direction direction, utility::size64_t so_far ) ;
	static void ProgressCallbackNull ( web::http::message_direction::direction direction, utility::size64_t so_far ) {} ;
	static void UploadCallback ( web::http::message_direction::direction direction, utility::size64_t so_far ) {} ;
	static void UploadCallbackNull ( web::http::message_direction::direction direction, utility::size64_t so_far ) {} ;

	static bool UploadModel ( wchar_t* pszUrl, char* pszCliendId, char* pszUser, char* pszPass, char* pData, int iSize, char* pszName, char* pszDesc, int* piModelId ) ;
	static bool UploadAd ( wchar_t* pszUrl, char* pszCliendId, char* pszUser, char* pszPass, char* pData, int iSize, char* pszAdUrl, int* piAdId ) ;

	static HWND m_hCallbackWnd ;

protected:
private:

};

