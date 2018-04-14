#include "CModelPublishServer.h"
#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <windows.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <atlenc.h>
#include <rpcdce.h>

#include "GlobalDefines.h"

#include <map>
#include <set>


using namespace std;

using namespace web ;
using namespace utility ;
using namespace http ;
using namespace web::http::experimental::listener ;

using namespace sql ;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

char CModelPublishServer::m_szServerRootFolder [ MAX_PATH ] = { 0 } ;

sql::Driver* CModelPublishServer::driver = NULL ;
//sql::Connection* CModelPublishServer::con = NULL ;

CModelPublishServer::CModelPublishServer ()
{
	m_szServerRootFolder [ 0 ] = 0 ;
}

CModelPublishServer::CModelPublishServer(utility::string_t url) : m_listener(url)
{
	m_listener.support ( methods::GET,  std::bind(&CModelPublishServer::HandleGet,  this, std::placeholders::_1) ) ;
	m_listener.support ( methods::POST, std::bind(&CModelPublishServer::HandlePost, this, std::placeholders::_1) ) ;

	driver = get_driver_instance ();
// 	con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
// 
// 	// Connect to the MySQL test database
// 	con->setSchema ( STORE_DATABASE_NAME ) ;
}

void CModelPublishServer::OnGetModel ( json::value& params, json::value& answer, status_code& http_result )
{
// 	sql::Driver *driver;
 	sql::Connection *con;


	if ( ! driver ) {
		answer [ L"error_message" ] = json::value::string ( L"Database error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}
// 
// 
// 	try {
// 		// Create a connection
// 		driver = get_driver_instance();
 		con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
// 
// 		// Connect to the MySQL test database
 		con->setSchema ( SERVICE_DATABASE_NAME ) ;
// 	}
// 	catch ( sql::SQLException &e ) {
// 		cout << e.what() << endl ;
// 		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
// 		http_result = status_codes::InternalError ;
// 		return ;
// 	}
	if ( ! con ) {
		answer [ L"error_message" ] = json::value::string ( L"Database connection error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}

	int iModelId = -1 ;

	{ // Fetch session id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		wstring strPname = params.at ( L"subsid" ).as_string () ;
		char szModelId [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szModelId, 256, "", NULL ) ;
		szModelId [ iLen ] = 0 ;

// 		char szSessionId [ 256 ] ;
// 		int iLen = WideCharToMultiByte ( CP_ACP, 0, sessionId.c_str(), sessionId.length(), szSessionId, 256, "", NULL ) ;
// 		szSessionId [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_subscription WHERE HashId='%s'", szModelId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"error_message" ] = json::value::string(L"Subscription id not found!") ;
			answer [ L"error_code" ] = json::value::number(MS_ERROR_SUBSCRIPTION_NOT_FOUND) ;
			http_result = status_codes::NotFound ;
		}
	  
		while ( res->next() ) {
			iModelId = res->getInt ( "ModelId" ) ;
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

	int iFileId = -1 ;
	if ( iModelId != -1 ) { // Fetch product id from database

		sql::Statement *stmt ;
		sql::ResultSet *res ;

		// Make query string
		char szQuery [ 5000 ] ;

		//wstring strPname = params.at(L"package_name").as_string() ;
//  		char szPaclageName [ 256 ] ;
//  		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str(), strPname.length(), szPaclageName, 256, "", NULL ) ;
//  		szPaclageName [ iLen ] = 0 ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_model_desc WHERE id=%d", iModelId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Model id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_MODEL_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}
	  
		while ( res->next() ) {
			iFileId = res->getInt("PCFileId") ;
		}
		delete res;
		delete stmt;
	}

	if ( iFileId != -1 ) {
		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_file_address WHERE id=%d", iFileId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement() ;
		res = stmt->executeQuery ( szQuery ) ;
	  
		if ( res->rowsCount() == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Model file address not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_FILE_ADDRESS_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next() ) {

			string strPathName = "" ;//= res->getString ( "UserId" ) ;

			std::istream* blob_stream = res->getBlob("FilePathName");
			
			char* pPathName = NULL;

			if (blob_stream) {
				try {
					blob_stream->seekg(0, std::ios::end);
					uint32_t blobSize = (uint32_t)blob_stream->tellg();
					blob_stream->seekg(0, std::ios::beg);
					pPathName = new char[blobSize+1];
					blob_stream->read(pPathName, blobSize);
					pPathName[blobSize] = 0;
					//blob_stream->getline(key, 600);
				}
				catch (...) {

				}
				//std::string retrievedPassword(pws); // also, should handle case where Password length > PASSWORD_LENGTH
				if ( pPathName ) {
					strPathName = pPathName;
					delete pPathName;
				}
			}

			//json::value msg ;
			//answer [ L"message" ] = msg ;

			string strFullFilePathName ;
			strFullFilePathName = m_szServerRootFolder ;
			strFullFilePathName += MODEL_FILE_PATH ;
			strFullFilePathName += strPathName.c_str() ;

			string strBase64FileName = strFullFilePathName + ".base64" ;

			wchar_t* pszModelBase64 = NULL ;

			FILE* pFile = fopen ( strBase64FileName.c_str(), "rb" ) ;
			if ( ! pFile ) {
				string strModelFileName = strFullFilePathName /*+ MODEL_FILE_EXTENSION*/ ;
				pFile = fopen ( strModelFileName.c_str (), "rb" ) ;

				if ( ! pFile ) {
					answer [ L"error_message" ] = json::value::string ( L"Could not open model file!" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_COULD_NOT_OPEN_FILE ) ;
					http_result = status_codes::NotFound ;
				}
				else { // Open original binary model

					fseek ( pFile, 0, SEEK_END ) ;
					int iSize = ftell ( pFile ) ;

					char* pBuf = new char [ iSize ] ;
					fseek ( pFile, 0, SEEK_SET ) ;
					fread ( pBuf, iSize, 1, pFile ) ;
					fclose ( pFile ) ;

					int iDestSize = Base64EncodeGetRequiredLength ( iSize, 0 ) ;
					int iEncLen = iDestSize ;

					PSTR pDest = new CHAR [ iDestSize + 1 ] ;
					Base64Encode ( (BYTE*)pBuf, iSize, pDest, &iEncLen ) ;
					pDest [ iEncLen ] = 0 ;

					string s = pDest ;

					pszModelBase64 = new wchar_t [ iEncLen + 1 ] ;
					int iLen = MultiByteToWideChar ( CP_ACP, 0, s.c_str (), s.length (), pszModelBase64, iEncLen ) ;
					pszModelBase64 [ iLen ] = 0 ;

					pFile = fopen ( strBase64FileName.c_str (), "wb" ) ;
					fwrite ( pszModelBase64, sizeof ( wchar_t ), iLen, pFile ) ;
					fclose ( pFile ) ;


					if ( pBuf )
						delete pBuf ;

					answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
					answer [ L"model" ] = json::value::string ( pszModelBase64 ) ;
					http_result = status_codes::OK ;
				}

			}
			else { // Opened base64 file

				fseek ( pFile, 0, SEEK_END ) ;
				int iSize = ftell ( pFile ) ;

				pszModelBase64 = new wchar_t [ iSize + 1 ] ;
				fseek ( pFile, 0, SEEK_SET ) ;
				fread ( pszModelBase64, iSize, 1, pFile ) ;
				fclose ( pFile ) ;
				
				pszModelBase64 [ iSize ] = 0 ;

				answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
				answer [ L"model" ] = json::value::string ( pszModelBase64 ) ;
				http_result = status_codes::OK ;
			}

			if ( pszModelBase64 )
				delete pszModelBase64 ;

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

void CModelPublishServer::OnGetAd ( json::value& params, json::value& answer, status_code& http_result )
{
// 	sql::Driver *driver;
 	sql::Connection *con;
	if ( !driver ) {
		answer [ L"error_message" ] = json::value::string ( L"Database error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}
	// 
	// 
	// 	try {
	// 		// Create a connection
	// 		driver = get_driver_instance();
	con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
	// 
	// 		// Connect to the MySQL test database
	con->setSchema ( SERVICE_DATABASE_NAME ) ;
	// 	}
	// 	catch ( sql::SQLException &e ) {
	// 		cout << e.what() << endl ;
	// 		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
	// 		http_result = status_codes::InternalError ;
	// 		return ;
	// 	}
	if ( !con ) {
		answer [ L"error_message" ] = json::value::string ( L"Database connection error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}

	int iAdId = -1 ;

	{ // Fetch session id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		wstring strPname = params.at ( L"subsid" ).as_string () ;
		char szModelId [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szModelId, 256, "", NULL ) ;
		szModelId [ iLen ] = 0 ;

		// 		char szSessionId [ 256 ] ;
		// 		int iLen = WideCharToMultiByte ( CP_ACP, 0, sessionId.c_str(), sessionId.length(), szSessionId, 256, "", NULL ) ;
		// 		szSessionId [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_subscription WHERE HashId='%s'", szModelId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Subscription id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_SUBSCRIPTION_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {
			iAdId = res->getInt ( "AdId" ) ;
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


	if ( iAdId != -1 ) {
		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_advertisement WHERE id=%d", iAdId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Ad id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_AD_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {

			string strPathName = "" ;
			string strUrl = "" ;

			{
				std::istream* blob_stream = res->getBlob ( "FilePathName" );

				char* pPathName = NULL;

				if ( blob_stream ) {
					try {
						blob_stream->seekg ( 0, std::ios::end );
						uint32_t blobSize = (uint32_t)blob_stream->tellg ();
						blob_stream->seekg ( 0, std::ios::beg );
						pPathName = new char [ blobSize + 1 ];
						blob_stream->read ( pPathName, blobSize );
						pPathName [ blobSize ] = 0;
					}
					catch ( ... ) {

					}
					if ( pPathName ) {
						strPathName = pPathName;
						delete pPathName;
					}
				}
			}

			{
				std::istream* blob_stream = res->getBlob ( "AdUrl" );

				char* pszUrl = NULL;

				if ( blob_stream ) {
					try {
						blob_stream->seekg ( 0, std::ios::end );
						uint32_t blobSize = (uint32_t)blob_stream->tellg ();
						blob_stream->seekg ( 0, std::ios::beg );
						pszUrl = new char [ blobSize + 1 ];
						blob_stream->read ( pszUrl, blobSize );
						pszUrl [ blobSize ] = 0;
					}
					catch ( ... ) {

					}
					if ( pszUrl ) {
						strUrl = pszUrl ;
						delete pszUrl ;
					}
				}
			}

			wchar_t* pszUrl = new wchar_t [ strUrl.length() + 1 ] ;
			int iLen = MultiByteToWideChar ( CP_ACP, 0, strUrl.c_str (), strUrl.length (), pszUrl, strUrl.length () + 1 ) ;
			pszUrl [ iLen ] = 0 ;

			answer [ L"url" ] = json::value::string ( pszUrl ) ;

			//json::value msg ;
			//answer [ L"message" ] = msg ;

			string strFullFilePathName ;
			strFullFilePathName = m_szServerRootFolder ;
			strFullFilePathName += MODEL_AD_PATH ;
			strFullFilePathName += strPathName.c_str () ;

			string strBase64FileName = strFullFilePathName + ".base64" ;

			wchar_t* pszBase64Ad = NULL ;

			FILE* pFile = fopen ( strBase64FileName.c_str (), "rb" ) ;
			if ( ! pFile ) {
				string strModelFileName = strFullFilePathName /*+ AD_FILE_EXTENSION*/ ;
				pFile = fopen ( strModelFileName.c_str (), "rb" ) ;

				if ( ! pFile ) {
					answer [ L"error_message" ] = json::value::string ( L"Could not open ad file!" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_COULD_NOT_OPEN_FILE ) ;
					http_result = status_codes::NotFound ;
				}
				else { // Open original binary model

					fseek ( pFile, 0, SEEK_END ) ;
					int iSize = ftell ( pFile ) ;

					char* pBuf = new char [ iSize ] ;
					fseek ( pFile, 0, SEEK_SET ) ;
					fread ( pBuf, iSize, 1, pFile ) ;
					fclose ( pFile ) ;

					int iDestSize = Base64EncodeGetRequiredLength ( iSize, 0 ) ;
					int iEncLen = iDestSize ;

					PSTR pDest = new CHAR [ iDestSize + 1 ] ;
					Base64Encode ( (BYTE*)pBuf, iSize, pDest, &iEncLen ) ;
					pDest [ iEncLen ] = 0 ;

					string s = pDest ;

					pszBase64Ad = new wchar_t [ iEncLen + 1 ] ;
					int iLen = MultiByteToWideChar ( CP_ACP, 0, s.c_str (), s.length (), pszBase64Ad, iEncLen ) ;
					pszBase64Ad [ iLen ] = 0 ;

					pFile = fopen ( strBase64FileName.c_str (), "wb" ) ;
					fwrite ( pszBase64Ad, sizeof ( wchar_t ), iLen, pFile ) ;
					fclose ( pFile ) ;


					if ( pBuf )
						delete pBuf ;

					answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
					answer [ L"ad" ] = json::value::string ( pszBase64Ad ) ;
					http_result = status_codes::OK ;
				}

			}
			else { // Opened base64 file

				fseek ( pFile, 0, SEEK_END ) ;
				int iSize = ftell ( pFile ) ;

				pszBase64Ad = new wchar_t [ iSize + 1 ] ;
				fseek ( pFile, 0, SEEK_SET ) ;
				fread ( pszBase64Ad, iSize, 1, pFile ) ;
				fclose ( pFile ) ;

				pszBase64Ad [ iSize ] = 0 ;

				answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
				answer [ L"ad" ] = json::value::string ( pszBase64Ad ) ;
				http_result = status_codes::OK ;
			}

			if ( pszBase64Ad )
				delete pszBase64Ad ;

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

void CModelPublishServer::OnGetInfo ( json::value& params, json::value& answer, status_code& http_result )
{
	// 	sql::Driver *driver;
	sql::Connection *con;
	if ( ! driver ) {
		answer [ L"error_message" ] = json::value::string ( L"Database error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}
	// 
	// 
	// 	try {
	// 		// Create a connection
	// 		driver = get_driver_instance();
	con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
	// 
	// 		// Connect to the MySQL test database
	con->setSchema ( SERVICE_DATABASE_NAME ) ;
	// 	}
	// 	catch ( sql::SQLException &e ) {
	// 		cout << e.what() << endl ;
	// 		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
	// 		http_result = status_codes::InternalError ;
	// 		return ;
	// 	}
	if ( ! con ) {
		answer [ L"error_message" ] = json::value::string ( L"Database connection error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}

	int iModelId = -1 ;

	{ // Fetch session id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		wstring strPname = params.at ( L"subsid" ).as_string () ;
		char szModelId [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szModelId, 256, "", NULL ) ;
		szModelId [ iLen ] = 0 ;

		// 		char szSessionId [ 256 ] ;
		// 		int iLen = WideCharToMultiByte ( CP_ACP, 0, sessionId.c_str(), sessionId.length(), szSessionId, 256, "", NULL ) ;
		// 		szSessionId [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_subscription WHERE HashId='%s'", szModelId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Subscription id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_SUBSCRIPTION_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {
			iModelId = res->getInt ( "ModelId" ) ;
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

	int iFileId = -1 ;
	if ( iModelId != -1 ) { // Fetch product id from database

		sql::Statement *stmt ;
		sql::ResultSet *res ;

		// Make query string
		char szQuery [ 5000 ] ;

		//wstring strPname = params.at(L"package_name").as_string() ;
		//  		char szPaclageName [ 256 ] ;
		//  		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str(), strPname.length(), szPaclageName, 256, "", NULL ) ;
		//  		szPaclageName [ iLen ] = 0 ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_model_desc WHERE id=%d", iModelId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Model id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_MODEL_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {
			iFileId = res->getInt ( "PCFileId" ) ;
		}
		delete res;
		delete stmt;
	}

	if ( iFileId != -1 ) {
		sql::Statement *stmt;
		sql::ResultSet *res;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_file_address WHERE id=%d", iFileId ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Model file address not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_FILE_ADDRESS_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {

			string strPathName = "" ;//= res->getString ( "UserId" ) ;

			std::istream* blob_stream = res->getBlob ( "FilePathName" );

			char* pPathName = NULL;

			if ( blob_stream ) {
				try {
					blob_stream->seekg ( 0, std::ios::end );
					uint32_t blobSize = (uint32_t)blob_stream->tellg ();
					blob_stream->seekg ( 0, std::ios::beg );
					pPathName = new char [ blobSize + 1 ];
					blob_stream->read ( pPathName, blobSize );
					pPathName [ blobSize ] = 0;
					//blob_stream->getline(key, 600);
				}
				catch ( ... ) {

				}
				//std::string retrievedPassword(pws); // also, should handle case where Password length > PASSWORD_LENGTH
				if ( pPathName ) {
					strPathName = pPathName;
					delete pPathName;
				}
			}

			//json::value msg ;
			//answer [ L"message" ] = msg ;

			string strFullFilePathName ;
			strFullFilePathName = m_szServerRootFolder ;
			strFullFilePathName += MODEL_FILE_PATH ;
			strFullFilePathName += strPathName.c_str () ;

			string strBase64FileName = strFullFilePathName + ".base64" ;

			wchar_t* pszBase64Hdr = NULL ;

			FILE* pFile = fopen ( strBase64FileName.c_str (), "rb" ) ;
			if ( ! pFile ) {
				string strModelFileName = strFullFilePathName + MODEL_FILE_EXTENSION ;
				pFile = fopen ( strModelFileName.c_str (), "rb" ) ;

				if ( ! pFile ) {
					answer [ L"error_message" ] = json::value::string ( L"Could not open model file!" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_COULD_NOT_OPEN_FILE ) ;
					http_result = status_codes::NotFound ;
				}
				else { // Open original binary model

					int iSize = 64 ;

					char* pBuf = new char [ iSize ] ;
					fseek ( pFile, 0, SEEK_SET ) ;
					fread ( pBuf, iSize, 1, pFile ) ;
					
					fseek ( pFile, 0, SEEK_END ) ;

					int iTotalFileSize = ftell ( pFile ) ;
					int iTotalFizeSizeBase64 = Base64EncodeGetRequiredLength ( iTotalFileSize, 0 ) ;

					fclose ( pFile ) ;

					int iDestSize = Base64EncodeGetRequiredLength ( iSize, 0 ) ;
					int iEncLen = iDestSize ;

					PSTR pDest = new CHAR [ iDestSize + 1 ] ;
					Base64Encode ( (BYTE*)pBuf, iSize, pDest, &iEncLen ) ;
					pDest [ iEncLen ] = 0 ;

					string s = pDest ;

					pszBase64Hdr = new wchar_t [ iEncLen + 1 ] ;
					int iLen = MultiByteToWideChar ( CP_ACP, 0, s.c_str (), s.length (), pszBase64Hdr, iEncLen ) ;
					pszBase64Hdr [ iLen ] = 0 ;

					if ( pBuf )
						delete pBuf ;

					answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
					answer [ L"info" ] = json::value::string ( pszBase64Hdr ) ;
					answer [ L"size" ] = iTotalFizeSizeBase64 ;
					http_result = status_codes::OK ;
				}

			}
			else { // Opened base64 file

				int iSize = 64 ;
				iSize = Base64EncodeGetRequiredLength ( iSize, 0 ) * 2 ;

				pszBase64Hdr = new wchar_t [ iSize + 1 ] ;
				fseek ( pFile, 0, SEEK_SET ) ;
				fread ( pszBase64Hdr, iSize, sizeof(wchar_t), pFile ) ;

				fseek ( pFile, 0, SEEK_END ) ;
				int iTotalFileSize = ftell ( pFile ) ;

				int iTotalFizeSizeBase64 = iTotalFileSize ;

				fclose ( pFile ) ;

				pszBase64Hdr [ iSize ] = 0 ;

				answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
				answer [ L"info" ] = json::value::string ( pszBase64Hdr ) ;
				answer [ L"size" ] = iTotalFizeSizeBase64 ;
				http_result = status_codes::OK ;
			}

			if ( pszBase64Hdr )
				delete pszBase64Hdr ;

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


void CModelPublishServer::SetServerRootFolder ( char* pszRoot )
{
	if ( pszRoot )
		strcpy ( m_szServerRootFolder, pszRoot ) ;
}

// void CStoreRegServer::OnAnalyticsData ( wstring& sessionId, json::value& params, json::value& answer, status_code& http_result )
// {
// }

void CModelPublishServer::HandleGet ( http_request request )
{
	wcout << L"Get Request Arrived:" << endl << request.to_string () << endl ;

	json::value answer ;

	answer [ L"error_message" ] = json::value::string(L"Unknown error") ;
	answer [ L"error_code" ] = json::value::number(MS_ERROR_UNKNOWN) ;
	status_code http_result = status_codes::ServiceUnavailable ;

	//wcout << answer.as_string() << endl ;

	//auto itAuth = request.headers().find (L"Authorization") ;
	//auto itHash = request.headers ().find ( L"Hash" ) ;

	try {
		//wstring strSessionId = itAuth->second ;

		auto uri = request.relative_uri();
		auto path_comps  = uri::split_path ( web::uri::decode(uri.path()) ) ;
		auto query_comps = uri::split_query ( web::uri::decode(uri.query()) ) ;

		json::value jsonParams2 ;//= request.extract_json(true).get() ;
		//ucout << jsonParams.serialize() << endl ;
		
		if ( path_comps.size() > 0 ) {

			if ( query_comps.find ( L"magic" ) == query_comps.end () || query_comps [ L"magic" ] != U ( MODEL_API_MAGIC ) ) {
				// Request not sent from client
				answer [ L"error_message" ] = json::value::string ( L"Request from unknown client!" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN_CLIENT ) ;
				http_result = status_codes::BadRequest ;
			}
			else {

				wstring strMethod = path_comps [ path_comps.size () - 1 ] ;

				if ( strMethod == U ( MODEL_API_GET ) ) {

					if ( query_comps.find ( L"subsid" ) != query_comps.end () )
						jsonParams2 [ L"subsid" ] = json::value::string ( query_comps [ L"subsid" ] ) ;
					if ( query_comps.find ( L"clientid" ) != query_comps.end () )
						jsonParams2 [ L"clientid" ] = json::value::string ( query_comps [ L"clientid" ] ) ;

					OnGetModel ( jsonParams2, answer, http_result ) ;
				}
				else if ( strMethod == U ( MODEL_API_GET_AD ) ) {

					if ( query_comps.find ( L"subsid" ) != query_comps.end () )
						jsonParams2 [ L"subsid" ] = json::value::string ( query_comps [ L"subsid" ] ) ;
					if ( query_comps.find ( L"clientid" ) != query_comps.end () )
						jsonParams2 [ L"clientid" ] = json::value::string ( query_comps [ L"clientid" ] ) ;

					OnGetAd ( jsonParams2, answer, http_result ) ;
				}
				else if ( strMethod == U ( MODEL_API_GET_INFO ) ) {

					if ( query_comps.find ( L"subsid" ) != query_comps.end () )
						jsonParams2 [ L"subsid" ] = json::value::string ( query_comps [ L"subsid" ] ) ;
					if ( query_comps.find ( L"clientid" ) != query_comps.end () )
						jsonParams2 [ L"clientid" ] = json::value::string ( query_comps [ L"clientid" ] ) ;

					OnGetInfo ( jsonParams2, answer, http_result ) ;
				}
				else {
					answer [ L"error_message" ] = json::value::string ( L"Bad method name!" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN_METHOD ) ;
					http_result = status_codes::BadRequest ;
				}
			}

		}
		else {
			// Invalid request
			answer [ L"error_message" ] = json::value::string ( L"Invalid request format!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN_CLIENT ) ;
			http_result = status_codes::BadRequest ;
		}
	}
	catch ( http_exception const & e ) {
		wcout << e.what() << endl;
	}

	if ( answer [ L"error_code" ].as_integer() == MS_ERROR_OK ) {
		wcout << L"Method call successful, huge json sent to client" << endl ;
	}
	else {
		wcout << L"Error in method call, reply json: " << endl << answer << endl ;
	}
	wcout << L"HTTP Result: " << http_result << endl << endl ;

	request.reply ( http_result, answer ) ;
}

void CModelPublishServer::HandlePost ( http_request& request )
{
	wcout << L"Post Request Arrived:" << endl /*<< request.to_string () << endl*/ ;

	json::value answer ;

	answer [ L"error_message" ] = json::value::string ( L"Unknown error" ) ;
	answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN ) ;
	status_code http_result = status_codes::ServiceUnavailable ;

	//auto itAuth = request.headers().find (L"Authorization") ;
	//auto itHash = request.headers ().find ( L"Hash" ) ;

	try {
		//wstring strSessionId = itAuth->second ;

		auto uri = request.relative_uri ();
		auto path_comps = uri::split_path ( web::uri::decode ( uri.path () ) ) ;
		auto query_comps = uri::split_query ( web::uri::decode ( uri.query () ) ) ;

		json::value jsonParams2 ;//= request.extract_json(true).get() ;
								 //ucout << jsonParams.serialize() << endl ;
		json::value jsonBody = request.extract_json (true).get () ;


		if ( path_comps.size () > 0 ) {

			wstring strMethod = path_comps [ path_comps.size () - 1 ] ;

			if ( strMethod == U ( MODEL_API_UPLOAD_MODEL ) ) {

				if ( jsonBody.has_field ( L"model" ) )
					jsonParams2 [ L"model" ] = jsonBody [ L"model" ] ;

				if ( jsonBody.has_field ( L"user" ) )
					jsonParams2 [ L"user" ] = jsonBody [ L"user" ] ;

				if ( jsonBody.has_field ( L"pass" ) )
					jsonParams2 [ L"pass" ] = jsonBody [ L"pass" ] ;

				if ( jsonBody.has_field ( L"name" ) )
					jsonParams2 [ L"name" ] = jsonBody [ L"name" ] ;

				if ( jsonBody.has_field ( L"desc" ) )
					jsonParams2 [ L"desc" ] = jsonBody [ L"desc" ] ;

				OnUploadModel ( jsonParams2, answer, http_result ) ;
			}
			else {
				answer [ L"error_message" ] = json::value::string ( L"Bad method name!" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN_METHOD ) ;
				http_result = status_codes::BadRequest ;
			}

		}
		else {
			// Invalid request
			answer [ L"error_message" ] = json::value::string ( L"Invalid request format!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_UNKNOWN_CLIENT ) ;
			http_result = status_codes::BadRequest ;
		}
	}
	catch ( http_exception const & e ) {
		wcout << e.what () << endl;
	}

	if ( answer [ L"error_code" ].as_integer () == MS_ERROR_OK ) {
		wcout << L"Method call successful, huge json sent to client" << answer << endl ;
	}
	else {
		wcout << L"Error in method call, reply json: " << endl << answer << endl ;
	}
	wcout << L"HTTP Result: " << http_result << endl << endl ;

	request.reply ( http_result, answer ) ;
}
 
void CModelPublishServer::OnUploadModel ( json::value& params, json::value& answer, status_code& http_result )
{
	// 	sql::Driver *driver;
	sql::Connection *con;

	if ( !driver ) {
		answer [ L"error_message" ] = json::value::string ( L"Database error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}
	// 
	// 
	// 	try {
	// 		// Create a connection
	// 		driver = get_driver_instance();
	con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
	// 
	// 		// Connect to the MySQL test database
	con->setSchema ( SERVICE_DATABASE_NAME ) ;
	// 	}
	// 	catch ( sql::SQLException &e ) {
	// 		cout << e.what() << endl ;
	// 		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
	// 		http_result = status_codes::InternalError ;
	// 		return ;
	// 	}
	if ( ! con ) {
		answer [ L"error_message" ] = json::value::string ( L"Database connection error!" ) ;
		answer [ L"error_code" ] = json::value::number ( MS_ERROR_DB ) ;
		http_result = status_codes::InternalError ;
	}

	int iUserId = -1 ;
	{ // Fetch user id from database
		sql::Statement *stmt;
		sql::ResultSet *res;

		wstring strPname = params.at ( L"user" ).as_string () ;
		char szUser [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szUser, 256, "", NULL ) ;
		szUser [ iLen ] = 0 ;

		strPname = params.at ( L"pass" ).as_string () ;
		char szPass [ 256 ] ;
		iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szPass, 256, "", NULL ) ;
		szPass [ iLen ] = 0 ;


		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "SELECT * FROM tbl_owner_desc WHERE Username='%s' AND Password='%s'", szUser, szPass ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Invalid username or password!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_INVALID_USER_PASS ) ;
			http_result = status_codes::Forbidden ;
		}

		while ( res->next () ) {
			iUserId = res->getInt ( "id" ) ;
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

	int iFileId = -1 ;

	if ( iUserId != -1 ) { // Insert model into database


		sql::Statement *stmt;
		sql::ResultSet *res;

		string strFullFilePathName ;
		int iDataSize = 0 ;

		{
			wstring str1 = params.at ( L"model" ).as_string () ;

			char* pszAnsi = new char [ str1.length () + 1 ] ;
			int iLen = WideCharToMultiByte ( CP_ACP, 0, str1.c_str (), str1.length (), pszAnsi, str1.length (), pszAnsi, NULL );
			pszAnsi [ iLen ] = 0 ;

			char* pData = new char [ iLen + 1 ] ;

			int iDecSize = iLen ;
			Base64Decode ( pszAnsi, iLen, (BYTE*)pData, &iDecSize ) ;

			iDataSize = iDecSize ;

			UUID uuid ;
			UuidCreate ( &uuid );
			char *uuid_str;
			UuidToStringA ( &uuid, (RPC_CSTR*)&uuid_str );

			strFullFilePathName = uuid_str ;
			strFullFilePathName += ".3dscan" ;

			string strFile = m_szServerRootFolder + (string)MODEL_FILE_PATH + strFullFilePathName ;

			RpcStringFreeA ( (RPC_CSTR*)&uuid_str );

			FILE* pFile = fopen ( strFile.c_str (), "wb" ) ;
			fwrite ( pData, iDecSize, 1, pFile ) ;
			fclose ( pFile ) ;

			cout << strFullFilePathName << endl ;

			delete pData ;
		}

		// Make query string
		char szQuery [ 5000 ] ;

		string strBase64FileName = strFullFilePathName + ".base64" ;

		sprintf_s ( szQuery, 5000, "INSERT INTO tbl_file_address (FilePathName, Base64FilePathName, Size) VALUES('%s','%s',%d);", strFullFilePathName.c_str (), strBase64FileName.c_str (), iDataSize ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		int iInsert = stmt->executeUpdate ( szQuery ) ;

		if ( iInsert == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Subscription id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_SUBSCRIPTION_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		delete stmt;

		sprintf_s ( szQuery, 5000, "SELECT LAST_INSERT_ID() AS id" ) ;
		cout << szQuery << endl ;

		stmt = con->createStatement () ;
		res = stmt->executeQuery ( szQuery ) ;

		if ( res->rowsCount () == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Subscription id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_SUBSCRIPTION_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		}

		while ( res->next () ) {
			iFileId = res->getInt ( "id" ) ;
		}

		delete stmt;
		delete res;
	}

	int iModelId = -1 ;
	if ( iFileId != -1 ) { // 

		sql::Statement *stmt ;
		sql::ResultSet *res ;

		wstring strPname = params.at ( L"name" ).as_string () ;
		char szModelName [ 256 ] ;
		int iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szModelName, 256, "", NULL ) ;
		szModelName [ iLen ] = 0 ;

		strPname = params.at ( L"desc" ).as_string () ;
		char szModelDesc [ 256 ] ;
		iLen = WideCharToMultiByte ( CP_ACP, 0, strPname.c_str (), strPname.length (), szModelDesc, 256, "", NULL ) ;
		szModelDesc [ iLen ] = 0 ;

		// Make query string
		char szQuery [ 5000 ] ;

		sprintf_s ( szQuery, 5000, "INSERT INTO tbl_model_desc (PCFileId, MobileFileId, ModelName, ModelDesc) VALUES(%d, %d, '%s', '%s')", iFileId, iFileId, szModelName, szModelDesc ) ;
		cout << szQuery << endl ;

		// Run query
		stmt = con->createStatement () ;
		int iInsert = stmt->executeUpdate ( szQuery ) ;

		if ( iInsert == 0 ) {
			answer [ L"error_message" ] = json::value::string ( L"Model id not found!" ) ;
			answer [ L"error_code" ] = json::value::number ( MS_ERROR_MODEL_NOT_FOUND ) ;
			http_result = status_codes::NotFound ;
		} 
		else {
			delete stmt;

			sprintf_s ( szQuery, 5000, "SELECT LAST_INSERT_ID() AS id" ) ;
			cout << szQuery << endl ;

			stmt = con->createStatement () ;
			res = stmt->executeQuery ( szQuery ) ;

			if ( res->rowsCount () == 0 ) {
				answer [ L"error_message" ] = json::value::string ( L"Model id not found!" ) ;
				answer [ L"error_code" ] = json::value::number ( MS_ERROR_MODEL_NOT_FOUND ) ;
				http_result = status_codes::NotFound ;
			}
			else {
				while ( res->next () ) {
					iModelId = res->getInt ( "id" ) ;
				}
				if ( iModelId != -1 ) { // Success
					answer [ L"error_message" ] = json::value::string ( L"Success" ) ;
					answer [ L"error_code" ] = json::value::number ( MS_ERROR_OK ) ;
					answer [ L"model_id" ] = json::value::number ( iModelId ) ;
					http_result = status_codes::OK ;
				}
			}

			delete res;
		}

		delete stmt;
	}

	delete con ;
}
