// ModelPublishServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

// Server defines
#define STORE_URL					"localhost"
//#define STORE_URL                   "testapi.ariogames.ir"
#define STORE_PORT					5617
//#define STORE_PORT					443
//#define STORE_SCHEME				"https"
#define STORE_SCHEME				"http"


#define STORE_URI					"/model/"
//#define STORE_URI                   "/sdk/windows"
#define STORE_API_VALIDATE			"validate"
#define STORE_API_GET_KEY			"key"
#define STORE_API_ANALYTIC_DATA		"analyticdata"

// Database defines
#define MYSQL_SERVER		"tcp://127.0.0.1:3306"
#define MYSQL_USER			"root"
#define MYSQL_PASS			"1234"
#define STORE_DATABASE_NAME	"dbModelPublish"

#define HMAC_SECRET_KEY		"Everything" // the light touches is our kingdom"

class CModelPublishServer
{
public:
	CModelPublishServer () {}
	CModelPublishServer ( utility::string_t url );

	pplx::task<void> open () { return m_listener.open (); }
	pplx::task<void> close () { return m_listener.close (); }

private:

	void HandleGet ( http_request message );
	void HandlePost ( http_request message );

	void handle_put ( http_request message );
	void handle_delete ( http_request message );

	http_listener m_listener;
};


std::unique_ptr<CModelPublishServer> g_Server ;


void InitializeServer ( const string_t& address )
{
	// Build our listener's URI from the configured address and the hard-coded path "store/auth"

	//     uri_builder uri(address);
	// 	uri.append_path ( U(STORE_URI) );

	//auto addr = uri.to_uri().to_string();
	g_Server = std::unique_ptr<CModelPublishServer> ( new CModelPublishServer ( address ) ) ;
	g_Server->open ().wait ();

	ucout << utility::string_t ( U ( "Listening for requests at: " ) ) << address << std::endl;

	return;
}

void ShutdownServer ()
{
	g_Server->close ().wait ();
	return;
}

// int _tmain ( int argc, _TCHAR* argv[] )
// {
// }

int main()
{
	uri_builder builder;
	builder.set_scheme ( U ( "http" ) ) ;
	builder.set_host ( U ( STORE_URL ) ) ;
	builder.set_port ( STORE_PORT ) ;
	builder.set_path ( U ( STORE_URI ) ) ;

	auto path_query_fragment = builder.to_string ();

	// 	utility::string_t port = U("5613");
	//     utility::string_t address = U("http://localhost:");
	//     address.append ( port ) ;

	InitializeServer ( path_query_fragment ) ;
	std::cout << "Press ENTER to exit." << std::endl;

	std::string line;
	std::getline ( std::cin, line );

	ShutdownServer ();

	return 0;
}

