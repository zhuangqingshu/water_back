#ifndef _USER_WEB_SERVER_H_
#define _USER_WEB_SERVER_H_

/*********************************************************************
 * INCLUDES
 */
#include <esp_system.h>
#include <esp_http_server.h>

/*********************************************************************
 * DEFINITIONS
 */

/*********************************************************************
 * TYPEDEFS
 */
/*********************************************************************
 * VARIABLE
 */
/*********************************************************************
 * API FUNCTIONS
 */
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);

#endif /* _USER_WEB_SERVER_H_ */

