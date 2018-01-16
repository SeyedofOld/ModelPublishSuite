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

#include "GlobalDefines.h"

#include <map>
#include <set>

using namespace std;

using namespace web; 
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

using namespace sql ;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

char CModelPublishServer::m_szServerRootFolder [ MAX_PATH ] = { 0 } ;

sql::Driver* CModelPublishServer::driver= NULL ;
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
// 
// 
// 	try {
// 		// Create a connection
// 		driver = get_driver_instance();
 		con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
// 
// 		// Connect to the MySQL test database
 		con->setSchema ( STORE_DATABASE_NAME ) ;
// 	}
// 	catch ( sql::SQLException &e ) {
// 		cout << e.what() << endl ;
// 		answer [ L"message" ] = json::value::string(L"Database connectivity error!") ;
// 		http_result = status_codes::InternalError ;
// 		return ;
// 	}

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
			answer [ L"message" ] = json::value::string(L"Invalid token. Token is expired.") ;
			answer [ L"result_number" ] = json::value::number(9999) ;
			http_result = status_codes::Unauthorized ;
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
			answer [ L"message" ] = json::value::string(L"Model not found!") ;
			http_result = status_codes::InternalError ;
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
			answer [ L"message" ] = json::value::string(L"This user does not have permission to access this product!") ;
			answer [ L"result_number" ] = json::value::number(2053) ;
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
				string strModelFileName = strFullFilePathName + MODEL_FILE_EXTENSION ;
				pFile = fopen ( strModelFileName.c_str (), "rb" ) ;

				if ( ! pFile ) {
					answer [ L"result_number" ] = json::value::number ( 100 ) ;
					answer [ L"message" ] = json::value::string ( L"Could not open model file!" ) ;
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

					answer [ L"error" ] = json::value::boolean ( false ) ;
					answer [ L"result_number" ] = json::value::number ( 1 ) ;
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

				answer [ L"error" ] = json::value::boolean ( false ) ;
				answer [ L"result_number" ] = json::value::number ( 1 ) ;
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
// 
// 	try {
// 		// Create a connection
// 		driver = get_driver_instance ();
 		con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
// 
// 		// Connect to the MySQL test database
 		con->setSchema ( STORE_DATABASE_NAME ) ;
// 	}
// 	catch ( sql::SQLException &e ) {
// 		cout << e.what () << endl ;
// 		answer [ L"message" ] = json::value::string ( L"Database connectivity error!" ) ;
// 		http_result = status_codes::InternalError ;
// 		return ;
// 	}

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
			answer [ L"message" ] = json::value::string ( L"Invalid token. Token is expired." ) ;
			answer [ L"result_number" ] = json::value::number ( 9999 ) ;
			http_result = status_codes::Unauthorized ;
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
			answer [ L"message" ] = json::value::string ( L"This user does not have permission to access this product!" ) ;
			answer [ L"result_number" ] = json::value::number ( 2053 ) ;
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
			if ( !pFile ) {
				string strModelFileName = strFullFilePathName + AD_FILE_EXTENSION ;
				pFile = fopen ( strModelFileName.c_str (), "rb" ) ;

				if ( !pFile ) {
					answer [ L"result_number" ] = json::value::number ( 100 ) ;
					answer [ L"message" ] = json::value::string ( L"Could not open model file!" ) ;
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

					answer [ L"error" ] = json::value::boolean ( false ) ;
					answer [ L"result_number" ] = json::value::number ( 1 ) ;
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

				answer [ L"error" ] = json::value::boolean ( false ) ;
				answer [ L"result_number" ] = json::value::number ( 1 ) ;
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
	// 
	// 	try {
	// 		// Create a connection
	// 		driver = get_driver_instance ();
	con = driver->connect ( MYSQL_SERVER, MYSQL_USER, MYSQL_PASS ) ;
	// 
	// 		// Connect to the MySQL test database
	con->setSchema ( STORE_DATABASE_NAME ) ;
	// 	}
	// 	catch ( sql::SQLException &e ) {
	// 		cout << e.what () << endl ;
	// 		answer [ L"message" ] = json::value::string ( L"Database connectivity error!" ) ;
	// 		http_result = status_codes::InternalError ;
	// 		return ;
	// 	}

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
			answer [ L"message" ] = json::value::string ( L"Invalid token. Token is expired." ) ;
			answer [ L"result_number" ] = json::value::number ( 9999 ) ;
			http_result = status_codes::Unauthorized ;
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
			answer [ L"message" ] = json::value::string ( L"Model not found!" ) ;
			http_result = status_codes::InternalError ;
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
			answer [ L"message" ] = json::value::string ( L"This user does not have permission to access this product!" ) ;
			answer [ L"result_number" ] = json::value::number ( 2053 ) ;
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

				if ( !pFile ) {
					answer [ L"result_number" ] = json::value::number ( 100 ) ;
					answer [ L"message" ] = json::value::string ( L"Could not open model file!" ) ;
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

					answer [ L"error" ] = json::value::boolean ( false ) ;
					answer [ L"result_number" ] = json::value::number ( 1 ) ;
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
				int iTotalFizeSizeBase64 = Base64EncodeGetRequiredLength ( iTotalFileSize, 0 ) ;

				fclose ( pFile ) ;

				pszBase64Hdr [ iSize ] = 0 ;

				answer [ L"error" ] = json::value::boolean ( false ) ;
				answer [ L"result_number" ] = json::value::number ( 1 ) ;
				answer [ L"info" ] = json::value::string ( pszBase64Hdr ) ;
				answer [ L"size" ] = iTotalFizeSizeBase64 ;

				//wcout << answer.as_string() << endl ;
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
	//TRACE("\nhandle GET\n");

	wcout << request.to_string () << endl ;

	json::value answer ;

	status_code http_result = status_codes::NotFound ;

	answer [ L"message" ] = json::value::string(L"Unknown error") ;
	answer [ L"error" ] = json::value::boolean(true) ;
	answer [ L"result_number" ] = json::value::number(0) ;

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
		
		if ( query_comps.find ( L"magic" ) == query_comps.end () || query_comps [ L"magic" ] != U(MODEL_API_MAGIC) ) {
			// Request not sent from client
			answer [ L"message" ] = json::value::string ( L"Request from unknown source!" ) ;
		}

		if ( path_comps.size() > 0 ) {

			//jsonParams2 [ L"hash" ] = json::value::string ( query_comps [ L"hash" ] ) ;

			wstring strMethod = path_comps [ path_comps.size() - 1 ] ;

			if ( strMethod == U(MODEL_API_GET) ) {

				if ( query_comps.find(L"subsid") != query_comps.end() )
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
				answer [ L"message" ] = json::value::string(L"Unrecognized method name!") ;
			}
		}
		else {
			// Invalid request
			answer [ L"message" ] = json::value::string(L"Invalid request!") ;
		}
	}
	catch ( http_exception const & e ) {
		wcout << e.what() << endl;
	}

	wcout << L"HTTP Result: " << http_result << endl << endl ;
	
	// wcout << answer << endl ;

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
 
