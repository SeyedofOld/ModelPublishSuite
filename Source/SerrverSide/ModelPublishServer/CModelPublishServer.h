#include <windows.h>
#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

using namespace web; 
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;


class CModelPublishServer
{
public:
	CModelPublishServer () ;
    CModelPublishServer(utility::string_t url);

	pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

	static void OnGetModel ( json::value& params, json::value& answer, status_code& http_result ) ;
	static void OnGetAd ( json::value& params, json::value& answer, status_code& http_result ) ;
	static void OnGetInfo ( json::value& params, json::value& answer, status_code& http_result ) ;

	static void SetServerRootFolder ( char* pszRoot ) ;

private:
    void HandleGet ( http_request message ) ;
    void HandlePost ( http_request message ) ;
    
	http_listener m_listener ;
	static char	m_szServerRootFolder [ MAX_PATH ] ;
};

