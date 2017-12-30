#pragma once


class CModelServiceWebClient 
{
public:
	CModelServiceWebClient() {} ;
	virtual ~CModelServiceWebClient() {} ;

	static bool ValidatePurchase ( char* pszSessionId, char* pszProductId, char** ppszOrderId, bool& rbInvalidSessionId ) ;
	static bool GetModel ( char* pszModelId, char* pszClientId, char** ppszRegKey ) ;
	//static bool GetModel ( char* pszUrl, char* pszClientId, char** ppszRegKey ) ;
	static bool SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2, bool& rbInvalidSessionId ) ;

protected:

private:

};