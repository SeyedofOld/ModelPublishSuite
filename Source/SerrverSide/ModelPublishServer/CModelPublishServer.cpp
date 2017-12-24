#include "stdafx.h"

#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <windows.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
 
#include "GlobalDefines.h"

#include <map>
#include <set>
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

using namespace std;
using namespace web; 
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;


using namespace sql ;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"


/*
Request:
[GET]: http://testapi.ariogames.ir/purchase/key?pid=1&hash=4234253214323234325

Response:
Status code: 401
{
  "message": "Invalid token. Token is expired.",
  "error": true,
  "result_number": 9999
}

Status code: 404
{
  "error": true,
  "message": "this user do not have permission to access this product",
  "result_number": 2053
}

Status code: 200
{
  "error": false,
  "result_number": 1,
  "message": {
    "key": "123123"
  }
}
*/

class CModelPublishServer
{
public:
    CModelPublishServer() { }
    CModelPublishServer(utility::string_t url);

	pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

 	//static void OnValidatePurchase ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result ) ;
 	static void OnGetModel ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result ) ;
// 	static void OnAnalyticsData ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result ) ;

private:
    void HandleGet ( http_request message ) ;
    void HandlePost ( http_request message ) ;
    
	http_listener m_listener;   
};

CModelPublishServer::CModelPublishServer(utility::string_t url) : m_listener(url)
{
	m_listener.support ( methods::GET,  std::bind(&CModelPublishServer::HandleGet,  this, std::placeholders::_1) ) ;
	m_listener.support ( methods::POST, std::bind(&CModelPublishServer::HandlePost, this, std::placeholders::_1) ) ;
}

/*void CModelPublishServer::OnValidatePurchase ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result )
{
	int iUserId = -1 ;
	int iProductId = -1 ;
	int iPurchaseId = -1 ;

	if ( ! params.has_field(L"package_name") ) {
		answer [ L"message" ] = json::value::string(L"No product id specified!") ;
		http_result = status_codes::InternalError ;
		return ;
	}

	sql::Driver *driver;
	sql::Connection *con;

	try {
		// Create a connection
		driver = get_driver_instance();
		con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;

		// Connect to the MySQL test database 
		con->setSchema ( STORE_DATABASE_NAME ) ;
	}
	catch ( sql::SQLException &e ) {
		cout << e.what() << endl ;
		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
		http_result = status_codes::InternalError ;
		return ;
	}

	{ // Fetch session id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		char szSessionId [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, sessionId.c_str(), sessionId.length(), szSessionId, 256, "", NULL ) ;
		szSessionId [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT UserId FROM tblDummySessionIds WHERE SessionId='%s'", szSessionId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"Invalid token. Token is expired.") ;
			answer [ L"result_number" ] = json::value::number(9999) ;
			http_result = status_codes::Unauthorized ;
		}
	  
		while ( res->next() ) {
			iUserId = res->getInt ( "UserId" ) ;
		//		cout << "\t... MySQL replies: ";
		// Access column data by alias or column name 
		//cout << res->getString("_message") << endl;
		//cout << "\t... MySQL says it again: ";
		// Access column data by numeric offset, 1 is the first column 
		//cout << res->getString(1) << endl;
		}
		delete res;
		delete stmt;

	} 

	if ( iUserId != -1 ) { // Fetch product id from database

		sql::Statement *stmt ;
		sql::ResultSet *res ;

		// Make query string
		char szQuery [ 5000 ] ;

		wstring strPname = params.at(L"package_name").as_string() ;
 		char szPaclageName [ 256 ] ;
 		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str(), strPname.length(), szPaclageName, 256, "", NULL ) ;
 		szPaclageName [ iLen ] = 0 ;


		sprintf_s ( szQuery, 5000, "SELECT ProductId FROM tblDummyProductIds WHERE PackageName='%s'", szPaclageName ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"Product token not found!") ;
			http_result = status_codes::InternalError ;
		}
	  
		while ( res->next() ) {
			iProductId = res->getInt("ProductId") ;
		}
		delete res;
		delete stmt;
	}

	if ( iProductId != -1 ) { // Fetch purchase id from database

		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT PurchaseId FROM tblPurchaseKeys WHERE UserId=%d AND ProductId=%d", iUserId, iProductId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"This user does not have permission to access this product!") ;
			answer [ L"result_number" ] = json::value::number(2053) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next() ) {
			iPurchaseId = res->getInt("PurchaseId") ;
		}

		delete res ;
		delete stmt ;

	} //catch (sql::SQLException &e) {


	if ( iPurchaseId != -1 ) {
		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT OrderId FROM tblDummyOrderIds WHERE PurchaseId=%d", iPurchaseId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"This user does not have permission to access this product!") ;
			answer [ L"result_number" ] = json::value::number(2053) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next() ) {

			string strOrderId = "" ;
			std::istream* blob_stream = res->getBlob("OrderId");

			char* pOid = NULL;

			if (blob_stream) {
				try {
					blob_stream->seekg(0, std::ios::end);
					int blobSize = blob_stream->tellg();
					blob_stream->seekg(0, std::ios::beg);
					pOid = new char[blobSize+1];
					blob_stream->read(pOid, blobSize);
					pOid[blobSize] = 0;
					//blob_stream->getline(key, 600);
				}
				catch (...) {

				}
				//std::string retrievedPassword(pws); // also, should handle case where Password length > PASSWORD_LENGTH
				if (pOid) {
					strOrderId = pOid;
					delete pOid;
				}
			}

// 			if ( blob_stream ) {
// 				char oid[1000]; // PASSWORD_LENGTH defined elsewhere; or use other functions to retrieve it
// 				blob_stream->getline(oid, 1000);
// 				strOrderId = oid ;
// 			}

			wchar_t szOrderId [ 1000 ] ;
			int iLen = MultiByteToWideChar ( CP_ACP, 0, strOrderId.c_str(), strOrderId.length(), szOrderId, 1000 ) ;
			szOrderId [ iLen ] = 0 ;

			// Changed to nested (Ugly!)
			//answer [ L"key" ] = json::value::string(szPubKey) ;
			//answer [ L"message" ] = json::value::string(L"No error") ;
			json::value msg ;
			msg [ L"order_id" ] = json::value::string(szOrderId) ;
			answer [ L"message" ] = msg ;

			answer [ L"error" ] = json::value::boolean(false) ;
			answer [ L"result_number" ] = json::value::number(1) ;
			http_result = status_codes::OK ;
		}
		delete res ;
		delete stmt ;

	} //catch (sql::SQLException &e) {
//  	  cout << "# ERR: SQLException in " << __FILE__ ;
//  	  cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
//  	  cout << "# ERR: " << e.what();
//  	  cout << " (MySQL error code: " << e.getErrorCode();
//  	  cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	//}
	delete con ;
}*/

void CModelPublishServer::OnGetModel ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result )
{
	int iUserId = -1 ;
	int iProductId = -1 ;

	if ( ! params.has_field(L"package_name") ) {
		answer [ L"message" ] = json::value::string(L"No product id specified!") ;
		http_result = status_codes::InternalError ;
		return ;
	}

	sql::Driver *driver;
	sql::Connection *con;

	try {
		// Create a connection
		driver = get_driver_instance();
		con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;

		// Connect to the MySQL test database
		con->setSchema ( STORE_DATABASE_NAME ) ;
	}
	catch ( sql::SQLException &e ) {
		cout << e.what() << endl ;
		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
		http_result = status_codes::InternalError ;
		return ;
	}

	{ // Fetch session id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		char szSessionId [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, sessionId.c_str(), sessionId.length(), szSessionId, 256, "", NULL ) ;
		szSessionId [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT UserId FROM tblDummySessionIds WHERE SessionId='%s'", szSessionId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"Invalid token. Token is expired.") ;
			answer [ L"result_number" ] = json::value::number(9999) ;
			http_result = status_codes::Unauthorized ;
		}
	  
		while ( res->next() ) {
			iUserId = res->getInt ( "UserId" ) ;
		//		cout << "\t... MySQL replies: ";
		// Access column data by alias or column name 
		//cout << res->getString("_message") << endl;
		//cout << "\t... MySQL says it again: ";
		// Access column data by numeric offset, 1 is the first column 
		//cout << res->getString(1) << endl;
		}
		delete res;
		delete stmt;

	} 

	if ( iUserId != -1 ) { // Fetch product id from database

		sql::Statement *stmt ;
		sql::ResultSet *res ;

		// Make query string
		char szQuery [ 5000 ] ;

		wstring strPname = params.at(L"package_name").as_string() ;
 		char szPaclageName [ 256 ] ;
 		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str(), strPname.length(), szPaclageName, 256, "", NULL ) ;
 		szPaclageName [ iLen ] = 0 ;


		sprintf_s ( szQuery, 5000, "SELECT ProductId FROM tblDummyProductIds WHERE PackageName='%s'", szPaclageName ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"Product token not found!") ;
			http_result = status_codes::InternalError ;
		}
	  
		while ( res->next() ) {
			iProductId = res->getInt("ProductId") ;
		}
		delete res;
		delete stmt;
	}

	if ( iProductId != -1 ) {
		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT ScrambleKey FROM tblPurchaseKeys WHERE UserId=%d AND ProductId=%d", iUserId, iProductId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"message" ] = json::value::string(L"This user does not have permission to access this product!") ;
			answer [ L"result_number" ] = json::value::number(2053) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next() ) {

			string strScrKey = "" ;//= res->getString ( "UserId" ) ;

			std::istream* blob_stream = res->getBlob("ScrambleKey");
			
			char* pKey = NULL;

			if (blob_stream) {
				try {
					blob_stream->seekg(0, std::ios::end);
					int blobSize = blob_stream->tellg();
					blob_stream->seekg(0, std::ios::beg);
					pKey = new char[blobSize+1];
					blob_stream->read(pKey, blobSize);
					pKey[blobSize] = 0;
					//blob_stream->getline(key, 600);
				}
				catch (...) {

				}
				//std::string retrievedPassword(pws); // also, should handle case where Password length > PASSWORD_LENGTH
				if (pKey) {
					strScrKey = pKey;
					delete pKey;
				}
			}

			wchar_t szScrKey [ 1000 ] ;
			int iLen = MultiByteToWideChar ( CP_ACP, 0, strScrKey.c_str(), strScrKey.length(), szScrKey, 1000 ) ;
			szScrKey [ iLen ] = 0 ;

			// Changed to nested (Ugly!)
			//answer [ L"key" ] = json::value::string(szPubKey) ;
			//answer [ L"message" ] = json::value::string(L"No error") ;
			json::value msg ;
			msg [ L"key" ] = json::value::string(szScrKey) ;
			answer [ L"message" ] = msg ;

			answer [ L"error" ] = json::value::boolean(false) ;
			answer [ L"result_number" ] = json::value::number(1) ;
			http_result = status_codes::OK ;
		}
		delete res ;
		delete stmt ;

	} //catch (sql::SQLException &e) {
//  	  cout << "# ERR: SQLException in " << __FILE__ ;
//  	  cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
//  	  cout << "# ERR: " << e.what();
//  	  cout << " (MySQL error code: " << e.getErrorCode();
//  	  cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	//}
	delete con ;
}



// void CStoreRegServer::OnAnalyticsData ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result )
// {
// }

void CModelPublishServer::HandleGet ( http_request request )
{
	TRACE("\nhandle GET\n");

	ucout << request.to_string() << endl ;

	json::value answer ;

	status_code http_result = status_codes::NotFound ;

	answer [ L"message" ] = json::value::string(L"Unknown error") ;
	answer [ L"error" ] = json::value::boolean(true) ;
	answer [ L"result_number" ] = json::value::number(0) ;

	//wcout << answer.as_string() << endl ;

	auto itAuth = request.headers().find (L"Authorization") ;

	auto itHash = request.headers ().find ( L"Hash" ) ;

	try {
		if ( itAuth != request.headers().end() ) {

			wstring strSessionId = itAuth->second ;

			auto uri = request.relative_uri();
			auto path_comps  = uri::split_path ( web::uri::decode(uri.path()) ) ;
			auto query_comps = uri::split_query ( web::uri::decode(uri.query()) ) ;

			json::value jsonParams2 ;//= request.extract_json(true).get() ;
			//ucout << jsonParams.serialize() << endl ;

			if ( path_comps.size() > 0 ) {
				if ( true ) {

					//jsonParams2 [ L"hash" ] = json::value::string ( query_comps [ L"hash" ] ) ;

					wstring strMethod = path_comps [ path_comps.size() - 1 ] ;

					if ( strMethod == U(MODEL_API_GET) ) {

						if ( query_comps.find(L"package_name") != query_comps.end() )
							jsonParams2 [ L"package_name" ] = json::value::string ( query_comps [ L"package_name" ] ) ;

						OnGetModel ( strSessionId, jsonParams2, answer, http_result ) ;
					}
					else if ( strMethod == L"analyticdata" ) {

						if ( query_comps.find(L"package_name") != query_comps.end() )
							jsonParams2 [ L"package_name" ] = json::value::string ( query_comps [ L"package_name" ] ) ;
						if ( query_comps.find(L"analytictype") != query_comps.end() )
							jsonParams2 [ L"analytictype" ] = json::value::number ( stoi(query_comps [ L"analytictype" ]) ) ;
						if ( query_comps.find(L"param1") != query_comps.end() )
							jsonParams2 [ L"param1" ] = json::value::number ( stoi(query_comps [ L"param1" ]) ) ;
						if ( query_comps.find(L"param2") != query_comps.end() )
							jsonParams2 [ L"param2" ] = json::value::number ( stoi(query_comps [ L"param2" ]) ) ;

						//OnAnalyticsData ( strSessionId, jsonParams2, answer, http_result ) ;
					}
					else if ( strMethod == L"validate" ) {

						if ( query_comps.find(L"package_name") != query_comps.end() )
							jsonParams2 [ L"package_name" ] = json::value::string ( query_comps [ L"package_name" ] ) ;

						//OnValidatePurchase ( strSessionId, jsonParams2, answer, http_result ) ;
					}
					else {
						answer [ L"message" ] = json::value::string(L"Unrecognized method name!") ;
					}
				}
				else {
					// No hash (No amphetamine :)
					answer [ L"message" ] = json::value::string(L"No message hash present!") ;
				}
			}
			else {
				// Invalid request
				answer [ L"message" ] = json::value::string(L"Invalid request!") ;
			}

		}
		else {
			// Error no session id in header
			answer [ L"message" ] = json::value::string(L"No session id specified!") ;
		}
	}
	catch ( http_exception const & e ) {
		wcout << e.what() << endl;
	}

	wcout << http_result << endl ;
	wcout << answer << endl ;

	request.reply ( http_result, answer ) ;
}

void CModelPublishServer::HandlePost ( http_request request )
{
	TRACE("\nhandle POST\n");

	ucout << request.to_string() << endl ;

	status_code http_result = status_codes::NotFound ;

	json::value answer ;
	answer [ L"message" ] = json::value::string(L"Unknown error") ;
	answer [ L"result" ] = json::value::string(L"FAILED") ;

	auto itAuth = request.headers().find (L"Authorization") ;

	try {
		if ( itAuth != request.headers().end() ) {

			wstring strSessionId = itAuth->second ;

			auto & jsonParams = request.extract_json(true).get() ;
			ucout << jsonParams.serialize() << endl ;

			if ( jsonParams.has_field(L"hash") ) {
				if ( jsonParams.has_field(L"method") ) {
				
					wstring strMethod = jsonParams.at(L"method").as_string() ;

					if ( strMethod == L"getregkey" ) {
						//OnRequestRegKey ( strSessionId, jsonParams, answer, http_result ) ;
					}
					else if ( strMethod == L"analyticdata" ) {
						//OnAnalyticsData ( strSessionId, jsonParams, answer, http_result ) ;
					}
					else {
						answer [ L"message" ] = json::value::string(L"Unrecognized method name!") ;
					}
				}
				else {
					// Error no method name
					answer [ L"message" ] = json::value::string(L"No method name specified!") ;
				}
			}
			else {
				// No hash (No amphetamine :)
				answer [ L"message" ] = json::value::string(L"No message hash present!") ;
			}

		}
		else {
			// Error no session id in header
			answer [ L"message" ] = json::value::string(L"No session id specified!") ;
		}
	}
	catch ( http_exception const & e ) {
		wcout << e.what() << endl;
	}

	request.reply ( status_codes::OK, answer ) ;
}
 
