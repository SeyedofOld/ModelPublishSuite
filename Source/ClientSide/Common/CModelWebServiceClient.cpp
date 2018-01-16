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

HWND CModelServiceWebClient::m_hCallbackWnd = NULL ;

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

pplx::task<http_response> make_task_request_get ( http_client & client, method mtd, uri const &uri, web::http::progress_handler progress )
{
	http_request request ( mtd ) ;
	request.set_request_uri ( uri ) ;

	///////////////////////////////////////
	request.set_progress_handler ( progress ) ;
// 			[ & ]( message_direction::direction direction, utility::size64_t so_far )
// 	{
// 		calls += 1;
// 		if ( direction == message_direction::upload )
// 			upsize = so_far;
// 		else
// 			downsize = so_far;
// 	} );
// 
// 	auto response = client.request ( request ).get ();
// 	http_asserts::assert_response_equals ( response, status_codes::OK );
// 
// 	response.content_ready ().wait ();

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
 
void make_request_get ( http_client & client, method mtd, uri &uri, json::value& result, status_code& http_result, web::http::progress_handler progress )
{
	make_task_request_get(client, mtd, uri, progress)
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

bool CModelServiceWebClient::GetModel ( wchar_t* pszUrl, char* pszClientId, char** ppData, int* piSize, int iInstanceId )
{
	if ( ! pszUrl || ! ppData )
		return false ;
	if ( ! pszClientId )
		return false ;


	wchar_t szClientID [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszClientId, strlen(pszClientId), szClientID, 1000 ) ;
	szClientID [ iLen ] = 0 ;

	web::uri inuri ( pszUrl ) ;

	uri_builder builder ( inuri ) ;

    // Append the query parameters: ?method=flickr.test.echo&name=value
	builder.append_query ( U ( "client" ), szClientID ) ;
 
	http_client_config config;
    config.set_validate_certificates ( false ) ;

	//MessageBox ( NULL, "0", "", MB_OK ) ;

	http_client client ( inuri, config ) ;

	json::value answer ;
	status_code http_result ;

	if ( 0 )
		make_request_get ( client, methods::GET, inuri, answer, http_result, &CModelServiceWebClient::ProgressCallback ) ;

	{
		http_request request ( methods::GET ) ;
		request.set_request_uri ( inuri ) ;

		request.set_progress_handler ( &CModelServiceWebClient::ProgressCallback ) ;

		try {
			MessageBoxW ( NULL, inuri.to_string ().c_str (), L"2", MB_OK ) ;
			http_response response = client.request ( request ).get () ;

			http_result = response.status_code () ;
			if ( http_result == status_codes::OK )
				answer = response.extract_json ( true ).get ();
		}
		catch ( http_exception const & e )
		{
			MessageBox ( NULL, e.what(), "", MB_OK ) ;
			wcout << e.what () << endl;
		}
	}

	//wcout << answer << endl ;
	wcout << L"HTTP Status Code:" << http_result << endl ;

//	rbInvalidSessionId = (http_result == status_codes::Unauthorized) ;

	if ( http_result != status_codes::OK ) {

		*ppData = NULL ;
		return false ;
	}
	else if ( answer.is_null() ) {
		*ppData = NULL ;
		return false ;
	}
	else if ( ! answer.has_field(L"message") ) {
		*ppData = NULL ;
		return false ;
	}
	else {
		//json::value imessage = answer[L"message"];
		if (!answer.has_field(L"model"))
		{
			*ppData = NULL;
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

			*ppData = new char [ iLen + 1 ] ;


			int iDecSize = iLen ;
			Base64Decode ( pszAnsi, iLen, (BYTE*)*ppData, &iDecSize ) ;

			if ( piSize )
				*piSize = iDecSize ;

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

/*bool CModelServiceWebClient::ValidatePurchase ( char* pszSessionId, char* pszProductId, char** ppszOrderId, bool& rbInvalidSessionId  )
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
	builder.set_path ( U(MODEL_URI) ) ;
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
}*/

/*bool CModelServiceWebClient::SendAnalyticsData ( char* pszSessionId, char* pszProductId, int iAnalyticType, int iParam1, int iParam2, bool& rbInvalidSessionId )
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

void CModelServiceWebClient::ProgressCallback ( web::http::message_direction::direction direction, utility::size64_t so_far )
{
	static int64_t s = 0 ;
	s += so_far ;

	if ( m_hCallbackWnd ) {
		::PostMessage ( m_hCallbackWnd, WM_USER_HTTP_PROGRESS, 0, (LPARAM)so_far ) ;
	}

	Sleep ( 10 ) ;
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


bool CModelServiceWebClient::GetModelInfo ( wchar_t* pszUrl, char* pszClientId, char** ppData, int* piSize, int* piFileSize, int iInstanceId )
{
	if ( !pszUrl || !ppData )
		return false ;
	if ( !pszClientId )
		return false ;

	// 	wchar_t szUrl [ 1000 ] ;
	// 	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszUrl, strlen(pszUrl), szUrl, 1000 ) ;
	// 	szUrl [ iLen ] = 0 ;

	wchar_t szClientID [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszClientId, strlen ( pszClientId ), szClientID, 1000 ) ;
	szClientID [ iLen ] = 0 ;

	// 	wchar_t szCustId [ 1000 ] ;
	// 	iLen = MultiByteToWideChar ( CP_ACP, 0, pszCustId, strlen ( pszCustId ), szCustId, 1000 ) ;
	// 	szCustId [ iLen ] = 0 ;


	//     json::value json_obj;
	//     json_obj[L"method"] = json::value::string(U("getregkey"));
	//     json_obj[L"package_name"] = json::value::string(szPid);
	//     json_obj[L"hash"] = json::value::string(U(""));


	web::uri inuri ( pszUrl ) ;

	uri_builder builder ( inuri ) ;
	// 	builder.set_scheme ( U(STORE_SCHEME) ) ;
	// 	builder.set_host ( U(STORE_URL) ) ;
	// 	builder.set_port ( STORE_PORT ) ;
	// 	builder.set_path ( U(STORE_URI) ) ;
	// 	builder.append_path ( U(MODEL_API_GET) ) ;

	//     utility::string_t port = std::to_wstring ( STORE_PORT ) ;
	//     utility::string_t address = STORE_URL ;
	//     address.append ( L":" ) ;
	//     address.append ( port ) ;
	// 
	//     address.append ( STORE_URI ) ;
	//     address.append ( STORE_API_GETREGKEY ) ;

	// Append the query parameters: ?method=flickr.test.echo&name=value
	builder.append_query ( U ( "client" ), szClientID ) ;
	//builder.append_query ( U ( "subsid" ), szModelId ) ;
	//	builder.append_query ( U ("custid"), szCustId ) ;
	//builder.append_query ( U("hash"), U("") ) ;

	//auto path_query_fragment = builder.to_string();

	//address.append ( path_query_fragment ) ;
	//    http::uri uri = http::uri(address);

	http_client_config config;
	config.set_validate_certificates ( false ) ;

	//MessageBox ( NULL, "0", "", MB_OK ) ;

	//MessageBoxW ( NULL, pszUrl, L"", MB_OK ) ;
	//MessageBoxW ( NULL, inuri.to_string ().c_str (), L"", MB_OK ) ;

	http_client client ( inuri, config ) ;

	//MessageBox ( NULL, "1", "", MB_OK ) ;

	// Write the current JSON value to a stream with the native platform character width
	//     utility::stringstream_t stream;
	//     json_obj.serialize(stream);
	//  
	//     // Display the string stream
	//     std::wcout << stream.str() << endl;

	//std::wcout << inuri.to_string () << endl;

	//uri my_uri = L"" ;

	json::value answer ;
	status_code http_result ;

	make_request_get ( client, methods::GET, inuri, answer, http_result, &CModelServiceWebClient::ProgressCallbackNull ) ;

	//MessageBoxW ( NULL, L"", L"2", MB_OK ) ;
	//wcout << answer << endl ;
	wcout << L"HTTP Status Code:" << http_result << endl ;

	//	rbInvalidSessionId = (http_result == status_codes::Unauthorized) ;

	if ( http_result != status_codes::OK ) {
		*ppData = NULL ;
		return false ;
	}
	else if ( answer.is_null () ) {
		*ppData = NULL ;
		return false ;
	}
	else if ( !answer.has_field ( L"message" ) ) {
		*ppData = NULL ;
		return false ;
	}
	else {
		//json::value imessage = answer[L"message"];
		if ( !answer.has_field ( L"info" ) )
		{
			*ppData = NULL;
			return false;
		}
		else {
			wstring str1 = answer [ L"info" ].as_string ();
			// 		wstring str2 = L"OK" ;
			// 		if ( str1 != str2 ) {
			// 			*ppszRegKey = NULL ;
			// 			return false ;
			// 		}

			//str1 = answer[L"key"].as_string();

			char* pszAnsi = new char [ str1.length () + 1 ] ;
			int iLen = WideCharToMultiByte ( CP_ACP, 0, str1.c_str (), str1.length (), pszAnsi, str1.length (), pszAnsi, NULL );
			pszAnsi [ iLen ] = 0 ;

			*ppData = new char [ iLen + 1 ] ;


			int iDecSize = iLen ;
			Base64Decode ( pszAnsi, iLen, (BYTE*)*ppData, &iDecSize ) ;

			if ( piSize )
				*piSize = iDecSize ;
			if ( piFileSize )
				*piFileSize = answer [ L"size" ].as_integer () ;

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

bool CModelServiceWebClient::GetAd ( wchar_t* pszUrl, char* pszClientId, char** ppData, int& riSize, std::string& strUrl, int iInstanceId )
{
	if ( !pszUrl || !ppData )
		return false ;
	if ( !pszClientId )
		return false ;

	wchar_t szClientID [ 1000 ] ;
	int iLen = MultiByteToWideChar ( CP_ACP, 0, pszClientId, strlen ( pszClientId ), szClientID, 1000 ) ;
	szClientID [ iLen ] = 0 ;

	// 	wchar_t szCustId [ 1000 ] ;
	// 	iLen = MultiByteToWideChar ( CP_ACP, 0, pszCustId, strlen ( pszCustId ), szCustId, 1000 ) ;
	// 	szCustId [ iLen ] = 0 ;


	//     json::value json_obj;
	//     json_obj[L"method"] = json::value::string(U("getregkey"));
	//     json_obj[L"package_name"] = json::value::string(szPid);
	//     json_obj[L"hash"] = json::value::string(U(""));

	web::uri inuri ( pszUrl ) ;

	uri_builder builder ( inuri ) ;

	http_client_config config;
	config.set_validate_certificates ( false ) ;

	http_client client ( inuri, config ) ;

	//std::wcout << path_query_fragment << endl;

	//uri my_uri = L"" ;

	json::value answer ;
	status_code http_result ;

	make_request_get ( client, methods::GET, inuri, answer, http_result, CModelServiceWebClient::ProgressCallbackNull ) ;

	//wcout << answer << endl ;
	wcout << L"HTTP Status Code:" << http_result << endl ;

	if ( http_result != status_codes::OK ) {
		*ppData = NULL ;
		return false ;
	}
	else if ( answer.is_null () ) {
		*ppData = NULL ;
		return false ;
	}
	else if ( !answer.has_field ( L"message" ) ) {
		*ppData = NULL ;
		return false ;
	}
	else {
		if ( !answer.has_field ( L"ad" ) )
		{
			*ppData = NULL;
			return false;
		}
		else {
			wstring str1 = answer [ L"ad" ].as_string ();

			char* pszAnsi = new char [ str1.length () + 1 ] ;
			int iLen = WideCharToMultiByte ( CP_ACP, 0, str1.c_str (), str1.length (), pszAnsi, str1.length (), pszAnsi, NULL );
			pszAnsi [ iLen ] = 0 ;

			*ppData = new char [ iLen + 1 ] ;

			int iDecSize = iLen ;
			Base64Decode ( pszAnsi, iLen, (BYTE*)*ppData, &iDecSize ) ;

			riSize = iDecSize ;

			if ( pszAnsi )
				delete pszAnsi ;

			{
				wstring sUrl = answer [ L"url" ].as_string ();

				char* pszUrl = new char [ sUrl.length () + 1 ] ;
				int iLen = WideCharToMultiByte ( CP_ACP, 0, sUrl.c_str (), sUrl.length (), pszUrl, sUrl.length (), "", NULL );
				pszUrl [ iLen ] = 0 ;

				if ( pszUrl ) {
					strUrl = pszUrl ;
					delete pszUrl ;
				}
			}

			return true;
		}
	}

	return false ;
}

