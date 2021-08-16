/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include "app_events.h"

#define BURSIZE 128

static const char *TAG="http server";

static int hex2dec(char c)
{
    if ('0' <= c && c <= '9') 
    {
        return c - '0';
    } 
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    } 
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    } 
    else 
    {
        return -1;
    }
}
 
static char dec2hex(short int c)
{
    if (0 <= c && c <= 9) 
    {
        return c + '0';
    } 
    else if (10 <= c && c <= 15) 
    {
        return c + 'A' - 10;
    } 
    else 
    {
        return -1;
    }
}
 
 
//编码一个url
static void urlencode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (    ('0' <= c && c <= '9') ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') || 
                c == '/' || c == '.') 
        {
            res[res_len++] = c;
        } 
        else 
        {
            int j = (short int)c;
            if (j < 0)
                j += 256;
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            res[res_len++] = '%';
            res[res_len++] = dec2hex(i1);
            res[res_len++] = dec2hex(i0);
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}
 
// 解码url
static void urldecode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (c != '%') 
        {
            res[res_len++] = c;
        }
        else 
        {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}

/* An HTTP GET handler */
static esp_err_t config_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    const char *bool_str[]             = {"false", "true"};
	char temp_str[64];
	wifi_config_t wifi_config;
	uint16_t apCount = 0;
	wifi_scan_config_t scanConf	= {
		.ssid = NULL,
		.bssid = NULL,
		.channel = 0,
		.show_hidden = false
	};//定义scanConf结构体，供函数esp_wifi_scan_start调用

	esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			
            ESP_LOGI(TAG, "Found URL query => %s", buf);

			APP_tsEvent sEvent;
			sEvent.eType = APP_E_EVENT_WIFI_CONFIG;
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => ssid=%s", param);
				strncpy((char *)sEvent.uEvent.wifi_config.ssid, (const char *)param, sizeof(wifi_config.sta.ssid));
				strncpy((char *)wifi_config.sta.ssid, (const char *)param, sizeof(wifi_config.sta.ssid));
            }
            if (httpd_query_key_value(buf, "password", (const char *)param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => password=%s", param);
				strncpy((char *)sEvent.uEvent.wifi_config.password, (const char *)param, sizeof(wifi_config.sta.password));
				strncpy((char *)wifi_config.sta.password, (const char *)param, sizeof(wifi_config.sta.password));
			}
			app_events_send(&sEvent, 100);
			
        }
        free(buf);
    }else{
		ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, false));//扫描所有可用的AP
		vTaskDelay(200);
		esp_wifi_scan_stop();
		esp_wifi_scan_get_ap_num(&apCount);//Get number of APs found in last scan
		ESP_LOGI(TAG, "Number of access points found: %d", apCount);
	}
	
    /* Send response with custom headers and body set as the
     * string passed in user context*/
    httpd_resp_send_chunk(req, "<html>", -1);
	httpd_resp_send_chunk(req, "<head>", -1);
	httpd_resp_send_chunk(req, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">", -1);
	httpd_resp_send_chunk(req, "<title>Paloma</title>", -1);
	
	httpd_resp_send_chunk(req, "<style>\
		#config-box{\
			width:100%;\
			height:auto;\
			margin:0 auto ;\
			margin-top:13%;\
			text-align:center;\
			background:#DCDCDC;\
			padding:20px 50px;\
		}\
		#config-box h1,#config-box h2{\
			color:#696969;\
		}\
		#config-box .form .item{\
			margin-top:15px;\
		}\
		#config-box .form .item i{\
			font-size:36px;\
			color:#696969;\
		}\
		#config-box .form .item input{\
			width:500px;\
			font-size:36px;\
			border:0;\
			border-bottom:2px solid #696969;\
			padding:5px 10px;\
			background:#ffffff00;\
			color:#696969;\
		}\
		#config-box #submit_button{\
			margin-top:20px;\
			width:190px;\
			height:50px;\
			font-size:30px;\
			font-weight:700;\
			color:#696969;\
			background-image: linear-gradient(to right, #74ebd5 0%, #9face6 100%);\
			border:0;\
			border-radius:15px;\
		}\
		#ap_list\
		{\
			width:80%;\
			height:auto;\
			margin:0 auto;\
			padding:20px 50px;\
		}\
		#ap_list td, #ap_list th\
		{\
			font-size:36px;\
        	text-align:left;\
			border:2px solid #696969;\
			padding:5px 10px;\
			background:#ffffff00;\
			color:#696969;\
		}\
		</style>", -1);
	httpd_resp_send_chunk(req, "<script>\
		function select_ap_click(){\
	　　　　var td = event.srcElement;\
			if(td.parentElement.rowIndex >= 0 && td.cellIndex >= 0){\
	　　　　	document.getElementById(\"ssid\").value=td.innerHTML; \
			}\
		}\
	</script>",-1);

	httpd_resp_send_chunk(req, "</head>", -1);
	httpd_resp_send_chunk(req, "<body>", -1);

	httpd_resp_send_chunk(req, "<div id=\"config-box\">", -1);
	if(apCount > 0){
		wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);//定义一个wifi_ap_record_t的结构体的链表空间
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));//获取上次扫描中找到的AP列表。
		int i;
		httpd_resp_send_chunk(req, "<h1>附近的 Wi-Fi</h1>", -1);
		httpd_resp_send_chunk(req, "<h2>请点击Wi-Fi名称选择</h2>", -1);
		httpd_resp_send_chunk(req, "<table id=\"ap_list\", onclick=\"select_ap_click();\">", -1);
		for (i=0; i<apCount; i++) {
			if(strlen((const char *)list[i].ssid) > 0){
				httpd_resp_send_chunk(req, "<tr>", -1);
				httpd_resp_send_chunk(req, "<td>", -1);
				httpd_resp_send_chunk(req, (const char *)list[i].ssid, -1);
				httpd_resp_send_chunk(req, "</td>", -1);
				httpd_resp_send_chunk(req, "</tr>", -1);
			}
		}
		httpd_resp_send_chunk(req, "</table>", -1);
	}

	httpd_resp_send_chunk(req, "<h1>Wi-Fi 配置</h1>", -1);
	httpd_resp_send_chunk(req, "<form  class=\"form\" method=GET>", -1);
	
	// WIFI SSID
	httpd_resp_send_chunk(req, "<div class=\"item\">", -1);
    httpd_resp_send_chunk(req, "<i>Wi-Fi 名称 </i><input id=\"ssid\" type=\"text\" name=\"ssid\" value=\"", -1);
	if(strlen((const char *)wifi_config.sta.ssid) > 0){
		httpd_resp_send_chunk(req, (const char *)wifi_config.sta.ssid, -1);
	}
	httpd_resp_send_chunk(req, "\">", -1);
	httpd_resp_send_chunk(req, "</div>", -1);

	// WIFI PASSWORD
	httpd_resp_send_chunk(req, "<div class=\"item\">", -1);
	httpd_resp_send_chunk(req, "<i>Wi-Fi 密码 </i><input type=\"text\" name=\"password\" value=\"", -1);
	if(strlen((const char *)wifi_config.sta.password) > 0){
		httpd_resp_send_chunk(req, (const char *)wifi_config.sta.password, -1);
	}
	httpd_resp_send_chunk(req, "\">", -1);
	httpd_resp_send_chunk(req, "</div>", -1);
	
	// submit button
	httpd_resp_send_chunk(req, "<div>", -1);
	httpd_resp_send_chunk(req, "<input id=\"submit_button\" type=\"submit\" value=\"保存\">", -1);
	httpd_resp_send_chunk(req, "</div>", -1);

	httpd_resp_send_chunk(req, "</form>", -1);
	httpd_resp_send_chunk(req, "</div>", -1);
	httpd_resp_send_chunk(req, "</body>", -1);
	httpd_resp_send_chunk(req, "</html>", -1);
	httpd_resp_send_chunk(req, NULL, 0);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI( TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t wifi_config = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = config_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &wifi_config);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

