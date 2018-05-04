#include <windows.h>
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace web; 
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;


class CModelPublishServer
{
public:
	CModelPublishServer () ;
    CModelPublishServer(utility::string_t url);

	inline pplx::task<void> open() { return m_listener.open(); }
    inline pplx::task<void> close() { return m_listener.close(); }

	static void OnGetModel ( json::value& params, json::value& answer, status_code& http_result ) ;
	static void OnGetAd ( json::value& params, json::value& answer, status_code& http_result ) ;
	static void OnGetInfo ( json::value& params, json::value& answer, status_code& http_result ) ;
	
	static void OnCreateSubscription ( json::value& params, json::value& answer, status_code& http_result ) ;

	static void OnUploadModel ( json::value& params, json::value& answer, status_code& http_result ) ;
	static void OnUploadAd ( json::value& params, json::value& answer, status_code& http_result ) ;

	static void SetServerRootFolder ( char* pszRoot ) ;

private:
    void HandleGet ( http_request message ) ;
    void HandlePost ( http_request& message ) ;
    
	http_listener m_listener ;
	static char	m_szServerRootFolder [ MAX_PATH ] ;

 	static sql::Driver *driver;
 	static sql::Connection *con;

};

