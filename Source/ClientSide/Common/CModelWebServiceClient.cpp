#include <windows.h>
#include "CModelWebServiceClient.h"
#include <stringapiset.h>
#include "GlobalDefines.h"

#include <iostream>
#include <streambuf>
#include <sstream>
#include <fstream>
#include "cpprest/json.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"

#include <atlenc.h>

using namespace std;
using namespace web; 
using namespace utility;
using namespace http;
using namespace http::client;
using namespace web::json ;

#ifdef _WIN32
# define iequals(x, y) (_stricmp((x), (y))==0)
#else
# define iequals(x, y) boost::iequals((x), (y))
#endif

void copy_result ( json::value & jvalue, json::value& result )
{
	result = jvalue ;
}

void display_field_map_json(  json::value & jvalue )
{
    if (!jvalue.is_null()) {
        for (auto const & e : jvalue.as_object()) {
            wcout << e.first << L" : " << e.second.as_string() << endl;
        }
    }
}

pplx::task<http_response> make_task_request_get ( http_client & client, method mtd, uri const &uri )
{
	http_request request ( mtd ) ;
	request.set_request_uri ( uri ) ;

// 	wchar_t szSid [ 1000 ] ;
// 	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
// 	szSid [ iLen ] = 0 ;
// 
// 	wstring strValue = L"Token " ;
// 	strValue += szSid ;
// 
// 	request.headers().add ( L"Authorization", strValue.c_str() ) ;

	if ( (mtd == methods::GET || mtd == methods::HEAD) ) {
	}
	else {
		//request.set_body ( jvalue.serialize() ) ;
	}
   
	return client.request ( request ) ;

	//return (mtd == methods::GET || mtd == methods::HEAD) ? client.request(mtd, STORE_URI_U) :  client.request(mtd, STORE_URI_U, jvalue.serialize());
}
 
pplx::task<http_response> make_task_request_post ( http_client & client, method mtd, json::value const & jvalue, char* pszSessionId )
{
	http_request request ( mtd ) ;
	request.set_request_uri ( U(MODEL_API_GET) ) ;

	wchar_t szSid [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
	szSid [ iLen ] = 0 ;

	wstring strValue = L"Token " ;
	strValue += szSid ;

	request.headers().add ( L"Authorization", strValue.c_str() ) ;

	if ( (mtd == methods::GET || mtd == methods::HEAD) ) {
	}
	else {
		request.set_body ( jvalue.serialize() ) ;
	}
   
	return client.request ( request ) ;

	//return (mtd == methods::GET || mtd == methods::HEAD) ? client.request(mtd, STORE_URI_U) :  client.request(mtd, STORE_URI_U, jvalue.serialize());
}
 
void make_request_get ( http_client & client, method mtd, uri &uri, json::value& result, status_code& http_result )
{
	make_task_request_get(client, mtd, uri)
	.then([&](http_response response)
	{
		//wcout << response.extract_json(true).get () << endl ;

		http_result = response.status_code() ;
		if (response.status_code() == status_codes::OK) {
			return response.extract_json(true);
		}
		
		return pplx::task_from_result(json::value());
	})
	.then([&result](pplx::task<json::value> previousTask) 
	{
		try {
			//display_field_map_json(previousTask.get());
			//auto ali = previousTask.get() ;
			//display_field_map_json(ali);
			result = previousTask.get() ;
			//display_field_map_json(result);
			//copy_result ( previousTask.get(), result ) ;
			//cout<<endl;
        }
        catch (http_exception const & e)
        {
			wcout << e.what() << endl;
        }
	})
	.wait();
}

void make_request_post ( http_client & client, method mtd, json::value const & jvalue, char* pszSessionId, json::value& result, status_code& http_result )
{
	make_task_request_post(client, mtd, jvalue, pszSessionId)
      .then([&](http_response response)
      {
         if (response.status_code() == status_codes::OK)
         {
            return response.extract_json();
         }
         return pplx::task_from_result(json::value());
      })
      .then([&result](pplx::task<json::value> previousTask)
      {
         try {
			 display_field_map_json(previousTask.get());
			 copy_result ( previousTask.get(), result ) ;
			cout<<endl;
         }
         catch (http_exception const & e)
         {
            wcout << e.what() << endl;
         }
      })
      .wait();
}

bool CModelServiceWebClient::GetModel ( char* pszModelId, char* pszClientId, char** ppszRegKey )
{
	if ( ! pszModelId || ! ppszRegKey )
		return false ;
	if ( ! pszClientId )
		return false ;

	wchar_t szModelId [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszModelId, strlen(pszModelId), szModelId, 1000 ) ;
	szModelId [ iLen ] = 0 ;

	wchar_t szClientID [ 1000 ] ;
	iLen = MultiByteToWideChar ( CP_ACP, 0, pszClientId, strlen(pszClientId), szClientID, 1000 ) ;
	szClientID [ iLen ] = 0 ;

// 	wchar_t szCustId [ 1000 ] ;
// 	iLen = MultiByteToWideChar ( CP_ACP, 0, pszCustId, strlen ( pszCustId ), szCustId, 1000 ) ;
// 	szCustId [ iLen ] = 0 ;


//     json::value json_obj;
//     json_obj[L"method"] = json::value::string(U("getregkey"));
//     json_obj[L"package_name"] = json::value::string(szPid);
//     json_obj[L"hash"] = json::value::string(U(""));

    uri_builder builder;
	builder.set_scheme ( U(STORE_SCHEME) ) ;
	builder.set_host ( U(STORE_URL) ) ;
	builder.set_port ( STORE_PORT ) ;
	builder.set_path ( U(STORE_URI) ) ;
	builder.append_path ( U(MODEL_API_GET) ) ;

//     utility::string_t port = std::to_wstring ( STORE_PORT ) ;
//     utility::string_t address = STORE_URL ;
//     address.append ( L":" ) ;
//     address.append ( port ) ;
// 
//     address.append ( STORE_URI ) ;
//     address.append ( STORE_API_GETREGKEY ) ;

    // Append the query parameters: ?method=flickr.test.echo&name=value
	builder.append_query ( U ( "client" ), szClientID ) ;
	builder.append_query ( U ( "subsid" ), szModelId ) ;
//	builder.append_query ( U ("custid"), szCustId ) ;
    builder.append_query ( U("hash"), U("") ) ;
 
    auto path_query_fragment = builder.to_string();

	//address.append ( path_query_fragment ) ;
//    http::uri uri = http::uri(address);

	http_client_config config;
    config.set_validate_certificates ( false ) ;

	http_client client ( path_query_fragment, config ) ;

    // Write the current JSON value to a stream with the native platform character width
//     utility::stringstream_t stream;
//     json_obj.serialize(stream);
//  
//     // Display the string stream
//     std::wcout << stream.str() << endl;

	std::wcout << path_query_fragment << endl;

	uri my_uri = L"" ;

	json::value answer ;
	status_code http_result ;

	make_request_get ( client, methods::GET, my_uri, answer, http_result ) ;

	wcout << answer << endl ;
	wcout << L"HTTP Stutus Code:" << http_result << endl ;

//	rbInvalidSessionId = (http_result == status_codes::Unauthorized) ;

	if ( http_result != status_codes::OK ) {
		*ppszRegKey = NULL ;
		return false ;
	}
	else if ( answer.is_null() ) {
		*ppszRegKey = NULL ;
		return false ;
	}
	else if ( ! answer.has_field(L"message") ) {
		*ppszRegKey = NULL ;
		return false ;
	}
	else {
		//json::value imessage = answer[L"message"];
		if (!answer.has_field(L"model"))
		{
			*ppszRegKey = NULL;
			return false;
		}
		else{
			wstring str1 = answer[L"model"].as_string();
			// 		wstring str2 = L"OK" ;
			// 		if ( str1 != str2 ) {
			// 			*ppszRegKey = NULL ;
			// 			return false ;
			// 		}

			//str1 = answer[L"key"].as_string();

			char* pszAnsi = new char [ str1.length () + 1 ] ;
			int iLen = WideCharToMultiByte ( CP_ACP, 0, str1.c_str (), str1.length (), pszAnsi, str1.length (), pszAnsi, NULL );
			pszAnsi [ iLen ] = 0 ;

			*ppszRegKey = new char [ iLen + 1 ] ;


			int iDecSize = iLen ;
			Base64Decode ( pszAnsi, iLen, (BYTE*)*ppszRegKey, &iDecSize ) ;

// 			int iLen = WideCharToMultiByte(CP_ACP, 0, str1.c_str(), str1.length(), NULL, 0, "", NULL);
// 			*ppszRegKey = new char[iLen + 1];
// 			WideCharToMultiByte(CP_ACP, 0, str1.c_str(), str1.length(), *ppszRegKey, iLen, "", NULL);
// 			char *p = ppszRegKey ;
// 			p[iLen] = 0;
			if ( pszAnsi )
				delete pszAnsi ;

			return true;
		}
	}

	return false ;
}

bool CModelServiceWebClient::ValidatePurchase ( char* pszSessionId, char* pszProductId, char** ppszOrderId, bool& rbInvalidSessionId  )
{
	if ( ! pszSessionId || ! ppszOrderId )
		return false ;
	if ( ! pszProductId )
		return false ;

	wchar_t szSid [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
	szSid [ iLen ] = 0 ;

	wchar_t szPid [ 1000 ] ;
	iLen = MultiByteToWideChar ( CP_ACP, 0, pszProductId, strlen(pszProductId), szPid, 1000 ) ;
	szPid [ iLen ] = 0 ;


//     json::value json_obj;
//     json_obj[L"method"] = json::value::string(U("getregkey"));
//     json_obj[L"package_name"] = json::value::string(szPid);
//     json_obj[L"hash"] = json::value::string(U(""));

    uri_builder builder;
	builder.set_scheme ( U(STORE_SCHEME) ) ;
	builder.set_host ( U(STORE_URL) ) ;
	builder.set_port ( STORE_PORT ) ;
	builder.set_path ( U(STORE_URI) ) ;
	builder.append_path ( U(STORE_API_VALIDATE) ) ;

//     utility::string_t port = std::to_wstring ( STORE_PORT ) ;
//     utility::string_t address = STORE_URL ;
//     address.append ( L":" ) ;
//     address.append ( port ) ;
// 
//     address.append ( STORE_URI ) ;
//     address.append ( STORE_API_GETREGKEY ) ;

    // Append the query parameters: ?method=flickr.test.echo&name=value
    builder.append_query ( U("package_name"), szPid ) ;
    builder.append_query ( U("hash"), U("") ) ;
 
    auto path_query_fragment = builder.to_string();

	//address.append ( path_query_fragment ) ;
//    http::uri uri = http::uri(address);

	http_client_config config;
    config.set_validate_certificates ( false ) ;

	http_client client ( path_query_fragment, config ) ;

    // Write the current JSON value to a stream with the native platform character width
//     utility::stringstream_t stream;
//     json_obj.serialize(stream);
//  
//     // Display the string stream
//     std::wcout << stream.str() << endl;

	std::wcout << path_query_fragment << endl;

	uri my_uri = L"" ;

	json::value answer ;
	status_code http_result ;

	make_request_get ( client, methods::GET, my_uri, answer, http_result ) ;

	wcout << answer << endl ;
	wcout << L"HTTP Status Code:" << http_result << endl ;

	rbInvalidSessionId = (http_result == status_codes::Unauthorized) ;

	if ( http_result != status_codes::OK ) {
		*ppszOrderId = NULL ;
		return false ;
	}
	else if ( answer.is_null() ) {
		*ppszOrderId = NULL ;
		return false ;
	}
	else if ( ! answer.has_field(L"message") ) {
		*ppszOrderId = NULL ;
		return false ;
	}
	else {
		json::value imessage = answer[L"message"];
		if (!imessage.has_field(L"order_id"))
		{
			*ppszOrderId = NULL;
			return false;
		}
		else{
			wstring str1 = imessage[L"order_id"].as_string();
			// 		wstring str2 = L"OK" ;
			// 		if ( str1 != str2 ) {
			// 			*ppszRegKey = NULL ;
			// 			return false ;
			// 		}

			//str1 = answer[L"key"].as_string();

			int iLen = WideCharToMultiByte(CP_ACP, 0, str1.c_str(), str1.length(), NULL, 0, "", NULL);
			*ppszOrderId = new char[iLen + 1];
			WideCharToMultiByte(CP_ACP, 0, str1.c_str(), str1.length(), *ppszOrderId, iLen, "", NULL);
			char *p = *ppszOrderId;
			p[iLen] = 0;
			return true;
		}
	}

	return false ;
}

bool CModelServiceWebClient::SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2, bool& rbInvalidSessionId )
{
	if ( ! pszSessionId )
		return false ;
	if ( ! pszProductId )
		return false ;
/*
	wchar_t szSid [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
	szSid [ iLen ] = 0 ;

	wchar_t szPid [ 1000 ] ;
	iLen = MultiByteToWideChar ( CP_ACP, 0, pszProductId, strlen(pszProductId), szPid, 1000 ) ;
	szPid [ iLen ] = 0 ;

    json::value json_obj;
    json_obj[L"method"] = json::value::string(U("analyticdata"));
    json_obj[L"package_name"] = json::value::string(szPid);
    json_obj[L"analytictype"] = json::value::number(iAnalyticType);
    json_obj[L"param1"] = json::value::number(iParam1);
    json_obj[L"param2"] = json::value::number(iParam2);
    json_obj[L"hash"] = json::value::string(U(""));

    utility::string_t port = std::to_wstring ( STORE_PORT ) ;
    utility::string_t address = STORE_URL ;
    address.append ( L":" ) ;
    address.append ( port ) ;

//    http::uri uri = http::uri(address);

    http_client client ( address ) ;

    // Write the current JSON value to a stream with the native platform character width
    utility::stringstream_t stream;
    json_obj.serialize(stream);
 
    // Display the string stream
    std::wcout << stream.str() << endl ;

	json::value result ;
	make_request ( client, methods::POST, json_obj, pszSessionId, result ) ;

	if ( result.is_null() ) {
		return false ;
	}
	else if ( ! result.has_field(L"result") ) {
		return false ;
	}
	else {
		wstring str1 = result [ L"result" ].as_string() ;
		wstring str2 = L"OK" ;
		if ( str1 != str2 ) {
			return false ;
		}

		return true ;
	}
	*/
	return true ;
}


/*
bool CStoreWebServiceClientOld::GetRegKey ( char* pszSessionId, char* pszProductId, char** ppszRegKey )
{
	if ( ! pszSessionId || ! ppszRegKey )
		return false ;
	if ( ! pszProductId )
		return false ;

	wchar_t szSid [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
	szSid [ iLen ] = 0 ;

	wchar_t szPid [ 1000 ] ;
	iLen = MultiByteToWideChar ( CP_ACP, 0, pszProductId, strlen(pszProductId), szPid, 1000 ) ;
	szPid [ iLen ] = 0 ;

    json::value json_obj;
    json_obj[L"method"] = json::value::string(U("getregkey"));
    json_obj[L"package_name"] = json::value::string(szPid);
    json_obj[L"hash"] = json::value::string(U(""));

    utility::string_t port = std::to_wstring ( STORE_PORT ) ;
    utility::string_t address = STORE_URL ;
    address.append ( L":" ) ;
    address.append ( port ) ;

//    http::uri uri = http::uri(address);

    http_client client ( address ) ;

    // Write the current JSON value to a stream with the native platform character width
    utility::stringstream_t stream;
    json_obj.serialize(stream);
 
    // Display the string stream
    std::wcout << stream.str() << endl;

	json::value result ;
	make_request ( client, methods::POST, json_obj, pszSessionId, result ) ;
	
	if ( result.is_null() ) {
		*ppszRegKey = NULL ;
		return false ;
	}
	else if ( ! result.has_field(L"result") ) {
		*ppszRegKey = NULL ;
		return false ;
	}
	else {
		wstring str1 = result [ L"result" ].as_string() ;
		wstring str2 = L"OK" ;
		if ( str1 != str2 ) {
			*ppszRegKey = NULL ;
			return false ;
		}

		str1 = result [ L"key" ].as_string() ;

		int iLen = WideCharToMultiByte ( CP_ACP, 0, str1.c_str(), str1.length(), NULL, 0, "", NULL ) ;
		*ppszRegKey = new char [ iLen + 1 ] ;
		WideCharToMultiByte ( CP_ACP, 0, str1.c_str(), str1.length(), *ppszRegKey, iLen, "", NULL ) ;
		char *p = *ppszRegKey ;
		p [ iLen ] = 0 ;
		return true ;

	}

	return false ;
}

bool CStoreWebServiceClientOld::SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2 )
{
	if ( ! pszSessionId )
		return false ;
	if ( ! pszProductId )
		return false ;

	wchar_t szSid [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszSessionId, strlen(pszSessionId), szSid, 1000 ) ;
	szSid [ iLen ] = 0 ;

	wchar_t szPid [ 1000 ] ;
	iLen = MultiByteToWideChar ( CP_ACP, 0, pszProductId, strlen(pszProductId), szPid, 1000 ) ;
	szPid [ iLen ] = 0 ;

    json::value json_obj;
    json_obj[L"method"] = json::value::string(U("analyticdata"));
    json_obj[L"package_name"] = json::value::string(szPid);
    json_obj[L"analytictype"] = json::value::number(iAnalyticType);
    json_obj[L"param1"] = json::value::number(iParam1);
    json_obj[L"param2"] = json::value::number(iParam2);
    json_obj[L"hash"] = json::value::string(U(""));

    utility::string_t port = std::to_wstring ( STORE_PORT ) ;
    utility::string_t address = STORE_URL ;
    address.append ( L":" ) ;
    address.append ( port ) ;

//    http::uri uri = http::uri(address);

    http_client client ( address ) ;

    // Write the current JSON value to a stream with the native platform character width
    utility::stringstream_t stream;
    json_obj.serialize(stream);
 
    // Display the string stream
    std::wcout << stream.str() << endl ;

	json::value result ;
	make_request ( client, methods::POST, json_obj, pszSessionId, result ) ;

	if ( result.is_null() ) {
		return false ;
	}
	else if ( ! result.has_field(L"result") ) {
		return false ;
	}
	else {
		wstring str1 = result [ L"result" ].as_string() ;
		wstring str2 = L"OK" ;
		if ( str1 != str2 ) {
			return false ;
		}

		return true ;
	}

	return true ;
}

*/