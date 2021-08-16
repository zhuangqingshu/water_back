/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/time.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "smartconfig_ack.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "tm_api.h"
#include "tm_user.h"

#include "app_common.h"
#include "driver_gn1629a.h"
#include "app_blink_led.h"
#include "app_buttons.h"
#include "app_events.h"
#include "app_event_handler.h"
#include "app_dispaly.h"
#include "board.h"
#include "flow_sensor.h"
#include "ntc_sensor.h"
#include "water_pump.h"
#include "app_water_back.h"
#include "driver_adc.h"
#include "app_sntp.h"
#include "app_menu.h"
#include "app_led_interface.h"
#include "mlink.h"
#include "user_httpserver.h"
#include "dns_server.h"

#define PRODUCT_ID   "4vF2Zeag9N" 
#define ACCESS_KEY   "VWWrmd0HVWtr74E/DTPtQVvfq5lpyQPBOcs/t8Ux1RM="

#define VACATION_TIME (24*60*60*1000/portTICK_RATE_MS) //vacation_timer
#define PUMP_PROTECTION_TIME_SEC (3600)

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t s_wifi_event_group;

xTimerHandle menu_exit_timer;
xTimerHandle back_led_timer;
httpd_handle_t user_http_server = NULL;

static const char *TAG = "app_main";
static struct tm_downlink_tbl_t downlink_tbl = {0};
static TaskHandle_t smartconfig_task_handle = NULL;
static xTimerHandle system_info_print_timer;
static xTimerHandle led_refresh_timer;
static xTimerHandle vacation_timer;
static xTimerHandle wifi_config_timer;
static char device_name[40];

static void smartconfig_task(void* parm);
void start_smartconfig();

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	ESP_LOGI(TAG, "Wifi event %s %d", event_base, event_id);	
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		
		esp_wifi_connect();
		
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
		
		xEventGroupSetBits(s_wifi_event_group, STA_CONNECTED_BIT);

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

		xEventGroupClearBits(s_wifi_event_group, STA_CONNECTED_BIT);
		if(water_back.wifi_status != E_WB_WIFI_CONFIG)
			water_back.wifi_status = E_WB_WIFI_UNCONNECT;
		
		mlink_notice_deinit();
		
		if (0 != tm_logout(5000)){
			ESP_LOGE(TAG, "tm api login out fail");
		}

		if(water_back.wifi_status != E_WB_WIFI_CONFIG){
			esp_wifi_connect();
		}

    } else if (event_base == WIFI_EVENT && event_id == SYSTEM_EVENT_AP_START) {
			
		if(user_http_server == NULL){
			user_http_server = start_webserver();
		}
	} else if (event_base == WIFI_EVENT && event_id == SYSTEM_EVENT_AP_STOP) {
		
		if(user_http_server){
			stop_webserver(user_http_server);
			user_http_server = NULL;
		}

	} else if (event_base == WIFI_EVENT && event_id == SYSTEM_EVENT_AP_STACONNECTED) {
		
		wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
		ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
				 MAC2STR(event->mac), event->aid);

	} else if (event_base == WIFI_EVENT && event_id == SYSTEM_EVENT_AP_STADISCONNECTED) {
		
		wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
		ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
				 MAC2STR(event->mac), event->aid);

	}

}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	ESP_LOGI(TAG, "ip event %s %d", event_base, event_id);	
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		  
	  xEventGroupSetBits(s_wifi_event_group, GOT_IP_BIT);
	  
	  mlink_notice_init();
	  
	  if(is_need_obtain_time()){
		  sntp_start_obtain_time();
	  }
	  APP_tsEvent sEvent;
	  sEvent.eType = APP_E_IP_EVENT_STA_GOT_IP;
	  app_events_send(&sEvent, 10);
	  if(0 != tm_init(&downlink_tbl))
	  {
		  ESP_LOGE(TAG,"tm init fail\n");
	  }
	  
	  if (0 != tm_login(PRODUCT_ID, device_name, ACCESS_KEY, 5000)){
		  ESP_LOGE(TAG, "tm api login fail");
	  }
	  water_back.wifi_status = E_WB_WIFI_CONNECT;

  } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
	  xEventGroupClearBits(s_wifi_event_group, GOT_IP_BIT);
  }

}

static void smart_config_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
	ESP_LOGI(TAG, "sc event %s %d", event_base, event_id);

	if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t* evt = (smartconfig_event_got_ssid_pswd_t*)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;

        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:%s", rvd_data);
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
		
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &smart_config_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_config_sta(const char *_ssid, const char *_password)
{
	wifi_config_t wifi_config;

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &smart_config_event_handler, NULL));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
	strcpy((char *)wifi_config.sta.ssid, (const char *)_ssid);
	strcpy((char *)wifi_config.sta.password, (const char *)_password);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", _ssid, _password);
    
}


void wifi_config_softap(const char *_ssid, const char *_password)
{
	if(_ssid == NULL){
		ESP_LOGE(TAG, "ssid null");
		return;
	}
	wifi_config_t wifi_config;

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	strcpy((char *)wifi_config.ap.ssid, _ssid);
	wifi_config.ap.ssid_len = strlen(_ssid);
    if (strlen(_password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }else{
    	strcpy((char *)wifi_config.ap.password, _password);
		wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	}
	//wifi_config.ap.max_connection = 1;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             _ssid, _password);
}
static void smartconfig_task(void* parm)
{
    EventBits_t uxBits;
	water_back.wifi_status = E_WB_WIFI_CONFIG;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, STA_CONNECTED_BIT | GOT_IP_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);

        if (uxBits & STA_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }

        if (uxBits & GOT_IP_BIT) {
            ESP_LOGI(TAG, "WiFi got ip");
        }

        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
			smartconfig_task_handle = NULL;
            vTaskDelete(NULL);
        }
    }
}

void start_smartconfig()
{
	if(smartconfig_task_handle == NULL){
		xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, DEFAULT_THREAD_PRIO, &smartconfig_task_handle);
	}
}

static void falow_sensor_handle(void)
{
	static int pre_rate = 0;
	static uint32_t time_start = 0;
    uint32_t time_escape;
    struct timeval now;

	gettimeofday(&now, NULL);
	
	int rate = flow_sensor_get_rate();
	water_back.flow_rate = rate;
	//ESP_LOGD(TAG, "Now rate %d, pre rate %d", rate, pre_rate);

	// 水控功能检测
	if(rate > 0 && pre_rate == 0 && water_back.onoff == false){
		// flow start
		time_start = now.tv_sec;
		water_back.flow_time = 0;
		water_back.flow_time_escape = 0;
	}
	
	if(time_start > 0){
		water_back.flow_time = now.tv_sec - time_start;
		ESP_LOGI(TAG, "Flow time %d", water_back.flow_time);
	}
	
	if(rate == 0 && pre_rate > 0){
		// flow end
		water_back.flow_time_escape = now.tv_sec - time_start;
		ESP_LOGI(TAG, "Flow time escape %d", water_back.flow_time_escape);

		APP_tsEvent sEvent;
		sEvent.eType = APP_E_EVENT_WB_FLOW_SENSOR;
		sEvent.uEvent.sFlowSensor.time_escape = water_back.flow_time_escape;
		app_events_send(&sEvent, 0);
		
		time_start = 0;
		water_back.flow_time = 0;
	}
	pre_rate = rate;
}

static void temperature_sensor_handle(void)
{
	// read temp
	float t = ntc_convert2c(driver_adc_get_value());
	water_back.temperature = (int32_t)(t+0.5f);
	//ESP_LOGD(TAG, "Temp %d, %d, Target Temp %d", (uint32_t)(t*100), water_back.temperature, water_back.target_temperature);

}

static void local_timer_handle(void)
{
	bool time_up = false;
    time_t now;
    struct tm timeinfo;

	// read local time
    time(&now);
    localtime_r(&now, &timeinfo);
	ESP_LOGD(TAG, "Now timer %d:%d", timeinfo.tm_hour, timeinfo.tm_min);
		
	int i;
	for(i=0; i < sizeof(water_back.timer) / sizeof(water_back.timer[0]); i++){
		/*if(!water_back.timer[i].enable){
			continue;
		}*/
		int hour_start=0,minute_start=0;
		int hour_end=0,minute_end=0;
		if(2 != sscanf(water_back.timer[i].time_start, "%d:%d", &hour_start,&minute_start)){
			continue;
		}
		if(2 != sscanf(water_back.timer[i].time_end, "%d:%d", &hour_end,&minute_end)){
			continue;
		}

		//ESP_LOGD(TAG, "Target timer [%d] %d:%d - %d:%d", i, hour_start, minute_start, hour_end, minute_end);

		int time_start = hour_start * 3600 + minute_start * 60;
		int time_end = hour_end * 3600 + minute_end * 60;
		int time_current = timeinfo.tm_hour *3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

		//ESP_LOGD(TAG, "Target timer [%d] now %d min %d max %d", i, time_current, time_start, time_end);
		
		if( time_current >=  time_start && time_current <= time_end ){
			if(water_back.timer[i].is_valid == false){
				
				water_back.timer[i].is_valid = true;
				
				APP_tsEvent sEvent;
				sEvent.eType = APP_E_EVENT_WB_TIME_IN_ZONE;
				sEvent.uEvent.sWBTimer.u8Timer = i;
				app_events_send(&sEvent, 0);
			}
			time_up = true;
		}else{
			if(water_back.timer[i].is_valid == true){
				
				water_back.timer[i].is_valid = false;
				
				APP_tsEvent sEvent;
				sEvent.eType = APP_E_EVENT_WB_TIME_OUT_ZONE;
				sEvent.uEvent.sWBTimer.u8Timer = i;
				app_events_send(&sEvent, 0);
			}
		}
		//ESP_LOGD(TAG, "Target timer [%d] is valid %d", i, water_back.timer[i].is_valid);

	}
	water_back.time_up =  time_up;
}

// 增压功能
void water_back_supercharge_handle()
{
	if(!water_back.supercharge)
		return;
	// 水流动足够长时间，且水泵未启动
	if(water_back.flow_rate > 0 &&
		water_back.flow_time > water_back.flow_time_max &&
		water_back.onoff == false
	){
		water_back.supercharge_status = true;
		water_back_set_onoff(true, 0);
	}
	// 增压状态下，水流停止，且不在清洗状态
	if(water_back.supercharge_status == true &&
		water_back.flow_rate == 0 &&
		water_back.vacation_status == false
	){
		water_back.supercharge_status = false;
		water_back_set_onoff(false, 0);
	}
}

// 水泵保护功能 水泵长时间工作启动保护，断电后恢复正常
void water_back_pump_protection_handle()
{
	if(!water_back.protection)
		return;

	time_t now = esp_timer_get_time()/1000000;

	if( water_back.onoff == true && 
		(now - water_back.start_work_time) > PUMP_PROTECTION_TIME_SEC
	){
		water_back.protection_status = true;
		ESP_LOGI(TAG, "protection status %d", water_back.protection_status);
		water_back_set_onoff(false, 0);
	}
}

// 低温保护功能 水控模式下有效
void water_back_temperature_protection_handle()
{
	//低温保护
	if( water_back.temperature < 5 && water_back.onoff == false){
		water_back.temp_protection_status = true;
		water_back_set_onoff(true, water_back.work_time);
	}
	// 水温足够，且不在清洗状态
	if( water_back.temp_protection_status == true && water_back.temperature > 7 && water_back.vacation_status == false){
		water_back_set_onoff(false, 0);
		water_back.temp_protection_status = false;
	}
}
// 水控模式处理
void water_back_water_control_handle()
{
	// is the flow_time_escape between	flow_time_min and flow_time_max
	// is the temperature less than target_temperature
	// if yes, water back stark work	
	// handle in vAppHandleAppEvent();

	// off water back?
	// 水温足够，且不在增压，清洗状态
	if(water_back.temperature > water_back.target_temperature + 1 &&
		water_back.supercharge_status == false && 
		water_back.vacation_status == false
	) {
		water_back_set_onoff(false, 0);
		water_back_auto_report();
	}
	water_back_temperature_protection_handle();
	water_back_supercharge_handle();
	water_back_pump_protection_handle();
}

// 自动模式处理
void water_back_auto_mode_handle()
{
	// is the temperature less than target_temperature
	// if yes, water back stark work
	if(water_back.temperature < water_back.target_temperature - 1){
		
		water_back_set_onoff(true, 0);
		water_back_auto_report();
	}

	// is need off water back?
	// 水温足够，且不在增压，清洗状态
	if(water_back.temperature > water_back.target_temperature + 1 && 
		water_back.supercharge_status == false && 
		water_back.vacation_status == false
	) {	
		water_back_set_onoff(false, 0);
		water_back_auto_report();
	}
	
	water_back_supercharge_handle();
	water_back_pump_protection_handle();

}

// 定时模式处理
void water_back_timer_mode_handle()
{
	if(water_back.time_up){
		water_back_auto_mode_handle();
	}else{
		water_back_water_control_handle();
	}
}

static void main_loop_task(void* parm)
{
	uint32_t tick_2s = 3;
    while (1) {

		tick_2s++;
		
		falow_sensor_handle();
		temperature_sensor_handle();

		if(tick_2s >= 4){
			tick_2s = 0;
			local_timer_handle();	
		}

		// is need on water back?
		switch(water_back.mode){
			case E_WB_MODE_WATER_CONTROL:
				water_back_water_control_handle();
				break;
			case E_WB_MODE_AUTO:
				water_back_auto_mode_handle();
				break;
			case E_WB_MODE_TIMER:
				water_back_timer_mode_handle();
				break;
			default:
				ESP_LOGE(TAG, "Unknown mode");
				break;
		}


		// 定时清洗功能
		// reset count down
		if(water_back.flow_rate > 0)
			xTimerReset(vacation_timer, 100);

		vTaskDelay(APP_TIME_TO_MS(500));
    }
}

// 定时清洗功能
// 水泵长时间不工作，启动10秒钟起到清洗作用
static void cb_vacation_timeout( TimerHandle_t xTimer )
{
	if(water_back.vacation_status == false ){
		
		if(water_back.onoff == false){
			// 开始清洗工作
			water_back.vacation_status = true;
			water_back_set_onoff(true, 10);
			xTimerChangePeriod(vacation_timer, 10000 /portTICK_RATE_MS , 100);
			xTimerReset(vacation_timer, 100);
		}
	} else {
		// 关闭清洗状态
		xTimerChangePeriod(vacation_timer, VACATION_TIME, 100);
		xTimerReset(vacation_timer, 100);
		water_back.vacation_status = false;
		water_back_set_onoff(false, 0);

	}
}

static void cb_app_display_refresh( TimerHandle_t xTimer )
{

	if(current_menu){
		current_menu->display_handler(current_menu);
	}
	app_display_refresh();
}

static void cb_system_info_print( TimerHandle_t xTimer )
{
	ESP_LOGI(TAG, "Temp %d", water_back.temperature);

	ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
}
// 超时关闭背光灯
static void cb_back_led_timeout( TimerHandle_t xTimer )
{
	APP_vSetLed(APP_E_LEDS_LED_1, false);
}

// 超时退出设置菜单
static void cb_menu_setting_timeout( TimerHandle_t xTimer )
{
	if(current_menu->id == E_APP_MENU_ID_TIMER_SETTING){
		// set time
		struct timeval set_time;
		struct tm * timeinfo = (struct tm *)current_menu->context;
		ESP_LOGD(TAG, "Set local time %d %d:%d", timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min);
		set_time.tv_sec = (long)mktime(timeinfo);
		set_time.tv_usec = 0;
		ESP_LOGD(TAG, "Set time %ld ", set_time.tv_sec);
		settimeofday(&set_time, NULL);
	}
	// save params
	vSaveRecord();
	water_back_auto_report();
	// exit 
	current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
	ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);

}
// wifi配置超时
static void cb_wifi_config_timeout( TimerHandle_t xTimer )
{
	water_back.wifi_configed = true;
	vSaveRecord();
	esp_restart();
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	ESP_LOGI(TAG, "[APP] APP version: %d", APP_VERSION);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("app_main", ESP_LOG_DEBUG);
	esp_log_level_set("app_water_back", ESP_LOG_DEBUG);
	esp_log_level_set("event_handler", ESP_LOG_DEBUG);
	//esp_log_level_set("tm_api", ESP_LOG_DEBUG);
	//esp_log_level_set("tm_mqtt", ESP_LOG_DEBUG);
	//esp_log_level_set("tm_user", ESP_LOG_DEBUG);
	//esp_log_level_set("tm_onejson", ESP_LOG_DEBUG);
	//esp_log_level_set("app_wifi", ESP_LOG_DEBUG);
	//esp_log_level_set("http server", ESP_LOG_DEBUG);

    ESP_ERROR_CHECK(nvs_flash_init());

	// resource init
    s_wifi_event_group = xEventGroupCreate();
	vacation_timer = xTimerCreate("vacationr", VACATION_TIME, pdTRUE, ( void * ) 0, cb_vacation_timeout);
	menu_exit_timer = xTimerCreate("menu_exit", 30000/portTICK_RATE_MS, pdFALSE, ( void * ) 0, cb_menu_setting_timeout);
	system_info_print_timer = xTimerCreate("info_print", 10000/portTICK_RATE_MS, true, NULL, cb_system_info_print);
	led_refresh_timer = xTimerCreate("led_refresh", 500/portTICK_RATE_MS, pdTRUE, NULL, cb_app_display_refresh);
	back_led_timer = xTimerCreate("back_led", 10000/portTICK_RATE_MS, pdTRUE, NULL, cb_back_led_timeout);

	// set onenet device name
	uint8_t mac[8];
	ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA)) ;
	sprintf(device_name, "paloma_backwater_%02x%02x%02x%02x%02x%02x", MAC2STR(mac));
	ESP_LOGI(TAG, "dev name:%s", device_name);
	ESP_LOGI(TAG, "product id:%s", PRODUCT_ID);
	ESP_LOGI(TAG, "access key:%s", ACCESS_KEY);
	
    downlink_tbl.prop_tbl = tm_prop_list;
    downlink_tbl.prop_tbl_size = tm_prop_list_size;
    downlink_tbl.svc_tbl = tm_svc_list;
    downlink_tbl.svc_tbl_size = tm_svc_list_size;

	app_events_init();
	app_events_set_handler(vAppHandleAppEvent);
	app_menu_init();
	current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
	ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
	water_back_init();

	APP_bButtonInitialise();
	vBlinkLedInit();
	APP_vSetLed(APP_E_LEDS_LED_1, true);
	flow_sensor_init(FLOW_SENSOR_IO_NUM);
	water_pump_init(WATER_PUMP_IO_NUM);
	driver_adc_init();
	driver_gn1629a_init(LED_DISPALY_STB_IO_NUM,LED_DISPALY_DIO_IO_NUM,LED_DISPALY_CLK_IO_NUM);
	driver_gn1629a_setting(3, true);
	
	// wifi init
	//initialise_wifi();
	if(water_back.wifi_configed){
		wifi_config_sta((const char *)water_back.ssid, (const char *)water_back.password);
	}else{
		water_back.wifi_status = E_WB_WIFI_CONFIG;
		wifi_config_softap(device_name, "");
		wifi_config_timer = xTimerCreate("wifi_config", 5*60*1000/portTICK_RATE_MS, pdTRUE, NULL, cb_wifi_config_timeout);
		xTimerStart(wifi_config_timer, 100);
	}

	// create task
	xTaskCreate(main_loop_task, "main_task", 2048, NULL, DEFAULT_THREAD_PRIO, NULL);
	
	xTimerStart(system_info_print_timer, 100);
	xTimerStart(led_refresh_timer, 100);
	xTimerStart(back_led_timer, 100);
	xTimerStart(vacation_timer, 100);

    return 0;

}
