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


void InitializeServer ( const string_t& address )
{
	char szDrive [ MAX_PATH ], szDir [ MAX_PATH ]  ;
	_splitpath ( g_szExeName, szDrive, szDir, NULL, NULL ) ;
	
	char szFolder [ MAX_PATH ] ;
	strcpy ( szFolder, szDrive ) ;
	strcat ( szFolder, szDir ) ;

	g_Server = std::unique_ptr<CModelPublishServer> ( new CModelPublishServer ( address ) ) ;
	g_Server->SetServerRootFolder ( szFolder ) ;
	g_Server->open ().wait ();

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

	uri_builder builder;
	builder.set_scheme ( U ( "http" ) ) ;
	builder.set_host ( U ( MODEL_SERVICE_SERVER ) ) ;
	//builder.set_port ( STORE_PORT ) ;
	builder.set_path ( U ( MODEL_SERVICE_PATH ) ) ;

	auto path_query_fragment = builder.to_string ();

	InitializeServer ( path_query_fragment ) ;
	std::cout << "Press ENTER to exit." << std::endl;

	std::string line;
	std::getline ( std::cin, line );

	ShutdownServer ();

	return 0;
}

