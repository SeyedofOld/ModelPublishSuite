#pragma once

#include "cpprest/http_msg.h"
#include "cpprest/http_client.h"

#define WM_USER_HTTP_PROGRESS	WM_USER+100

class CModelServiceWebClient 
{
public:
	CModelServiceWebClient() {} ;
	virtual ~CModelServiceWebClient() {} ;

	//static bool ValidatePurchase ( char* pszSessionId, char* pszProductId, char** ppszOrderId, bool& rbInvalidSessionId ) ;
	static bool GetModel ( wchar_t* pszUrl, char* pszClientId, char** ppData, int iInstanceId = 0 ) ;
	static bool GetModelInfo ( wchar_t* pszUrl, char* pszClientId, char** ppData, int iInstanceId = 0 ) ;
	static bool GetAd ( wchar_t* pszUrl, char* pszClientId, char** ppData, int& riSize, std::string& strUrl, int iInstanceId = 0 ) ;
	static bool SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2, bool& rbInvalidSessionId ) ;
	static void ProgressCallback ( web::http::message_direction::direction direction, utility::size64_t so_far ) ;
	static void ProgressCallbackNull ( web::http::message_direction::direction direction, utility::size64_t so_far ) {} ;

	static HWND m_hCallbackWnd ;

protected:
private:

};

