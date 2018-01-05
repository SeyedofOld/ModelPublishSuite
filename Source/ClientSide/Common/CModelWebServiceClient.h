#pragma once

#include "cpprest/http_msg.h"
#include "cpprest/http_client.h"

class CModelServiceWebClient 
{
public:
	CModelServiceWebClient() {} ;
	virtual ~CModelServiceWebClient() {} ;

	static bool ValidatePurchase ( char* pszSessionId, char* pszProductId, char** ppszOrderId, bool& rbInvalidSessionId ) ;
	static bool GetModel ( char* pszUrl, char* pszClientId, char** ppData ) ;
	static bool GetModelInfo ( char* pszUrl, char* pszClientId, char** ppData ) ;
	static bool GetAd ( char* pszUrl, char* pszClientId, char** ppData ) ;
	//static bool GetModel ( char* pszUrl, char* pszClientId, char** ppszRegKey ) ;
	static bool SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2, bool& rbInvalidSessionId ) ;
	static void ProgressCallback ( web::http::message_direction::direction direction, utility::size64_t so_far ) ;

protected:

private:

};