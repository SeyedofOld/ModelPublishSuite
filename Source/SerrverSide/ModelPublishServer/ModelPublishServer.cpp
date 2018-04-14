// ModelPublishServer.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "GlobalDefines.h"
#include "CModelPublishServer.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

char g_szExeName [ MAX_PATH ] ;
wstring g_strHost ;

// class CModelPublishServer
// {
// public:
// 	CModelPublishServer () {}
// 	CModelPublishServer ( utility::string_t url );
// 
// 	pplx::task<void> open () { return m_listener.open (); }
// 	pplx::task<void> close () { return m_listener.close (); }
// 
// private:
// 
// 	void HandleGet ( http_request message );
// 	void HandlePost ( http_request message );
// 
// 	void handle_put ( http_request message );
// 	void handle_delete ( http_request message );
// 
// 	http_listener m_listener;
// };


std::unique_ptr<CModelPublishServer> g_Server ;

bool LoadConfig ()
{
	FILE* pFile = fopen ( "config.txt", "rt" ) ;
	if ( ! pFile )
		return false ;

	wchar_t szHost [ 1000 ] ;
	fwscanf ( pFile, L"%s", szHost ) ;
	g_strHost = szHost ;

	fclose ( pFile ) ;

	return true ;
}

void InitializeServer ( const string_t& address )
{
	char szDrive [ MAX_PATH ], szDir [ MAX_PATH ]  ;
	_splitpath ( g_szExeName, szDrive, szDir, NULL, NULL ) ;
	
	char szFolder [ MAX_PATH ] ;
	strcpy ( szFolder, szDrive ) ;
	strcat ( szFolder, szDir ) ;

	//MessageBox ( NULL, "41", "", MB_OK ) ;

	g_Server = std::unique_ptr<CModelPublishServer> ( new CModelPublishServer ( address ) ) ;
	//MessageBox ( NULL, "42", "", MB_OK ) ;
	g_Server->SetServerRootFolder ( szFolder ) ;
	//MessageBox ( NULL, "43", "", MB_OK ) ;
	try{
		g_Server->open ().wait ();
	}
	catch ( ... )
	{
		int mm = 1;
	}
	//MessageBox ( NULL, "44", "", MB_OK ) ;

	ucout << utility::string_t ( U ( "Listening for requests at: " ) ) << address << std::endl;

	return;
}

void ShutdownServer ()
{
	g_Server->close ().wait ();
}

int main ( int argc, char* argv[] )
{
	strcpy ( g_szExeName, argv [ 0 ] ) ;

	if ( ! LoadConfig () ) {
		return 1 ;
	}

	//MessageBox ( NULL, "1", "", MB_OK ) ;

	uri_builder builder;
	builder.set_scheme ( U ( "http" ) ) ;
	//builder.set_host ( U ( MODEL_SERVICE_SERVER ) ) ;
	builder.set_host ( g_strHost ) ;
	//MessageBox ( NULL, "2", "", MB_OK ) ;
	//builder.set_port ( MODEL_SERVICE_PORT ) ;
	builder.set_path ( U ( MODEL_SERVICE_PATH ) ) ;
	//MessageBox ( NULL, "3", "", MB_OK ) ;

	auto path_query_fragment = builder.to_string (); 
	//MessageBox ( NULL, "4", "", MB_OK ) ;

	InitializeServer ( path_query_fragment ) ;
	//MessageBox ( NULL, "5", "", MB_OK ) ;
	std::cout << "Press ENTER to exit." << std::endl << std::endl ;

	std::string line;
	std::getline ( std::cin, line );

	ShutdownServer ();

	return 0;
}

