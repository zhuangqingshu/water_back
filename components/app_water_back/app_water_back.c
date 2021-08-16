
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "app_water_back.h"
#include "water_pump.h"
#include "esp_log.h"
#include "tm_user.h"
#include "err_def.h"
#include "app_common.h"

water_back_t water_back;
static water_back_cloud_t water_back_cloud;
static water_back_store_t water_back_store;
static xTimerHandle work_timer; 

static const char *NVS_WB_NAME = "water_back";
static const char *TAG = "app_water_back";

static void cb_work_timer_handle( xTimerHandle pxTimer)
{
	water_back_set_onoff(false, 0);
}

void water_back_init(void)
{
	memset(&water_back, 0 , sizeof(water_back));
	vDefaultRecords();
	vLoadRecord();
	vCopyRecordsToApp();

	ESP_LOGD(TAG, "Water back params:\r\n");
	ESP_LOGD(TAG, "flow_time_max:%d", water_back.flow_time_max);
	ESP_LOGD(TAG, "flow_time_min:%d", water_back.flow_time_min);
	ESP_LOGD(TAG, "mode:%d", water_back.mode);
	ESP_LOGD(TAG, "supercharge:%d", water_back.supercharge);
	ESP_LOGD(TAG, "target_temperature:%d", water_back.target_temperature);
	ESP_LOGD(TAG, "protection:%d", water_back.protection);
	ESP_LOGD(TAG, "vacation:%d", water_back.vacation);
	ESP_LOGD(TAG, "work_time:%d", water_back.work_time);
	int i;
	for(i=0; i < 3; i++){
		ESP_LOGD(TAG, "Local timer [%d] %s", i,  water_back.timer[i].enable ? "en" : "disable");
		ESP_LOGD(TAG, "Start at %s" , water_back.timer[i].time_start);
		ESP_LOGD(TAG, "End at %s" , water_back.timer[i].time_end);
	}

	work_timer = xTimerCreate("work_timer", water_back.work_time * 1000 / portTICK_RATE_MS, pdFALSE, ( void * ) 0, cb_work_timer_handle);

}

/*
	onoff on/off water pump
	work_time_ms water pump work time in ms,if set 0, than water pump work forever.
*/
void water_back_set_onoff(bool onoff, int work_time)
{
	// 水泵保护状态下直接退出
	if(water_back.protection_status && onoff == true)
		return;

	if(onoff){
		if(!water_back.onoff){

			water_back.onoff = onoff;
			water_back.work_time_real = work_time;
			water_pump_set_onoff(true);
			water_back.start_work_time = esp_timer_get_time()/1000000;
			ESP_LOGD(TAG, "Water back pump %s %d", onoff ? "on" : "off", work_time);

			if( work_time > 0 ){
				// countdown for off water pump
				xTimerStop(work_timer, 0);
				xTimerChangePeriod(work_timer, work_time * 1000 / portTICK_RATE_MS, 100);
				xTimerStart(work_timer, 0);
			}
		}
	}else{
		if(water_back.onoff){

			xTimerStop(work_timer, 0);
			water_back.onoff = onoff;
			water_pump_set_onoff(false);
			ESP_LOGD(TAG, "Water back pump %s %d", onoff ? "on" : "off", work_time);
		}
	}

}

#if 0

int water_back_auto_report()
{
	return 0;
}

#else
int water_back_auto_report()
{
	int ret;
	time_t now = 0;
	if(water_back.wifi_status!=E_WB_WIFI_CONNECT){
		return -1;
	}

	if(water_back_cloud.flow_time_max != water_back.flow_time_max){
		
		water_back_cloud.flow_time_max = water_back.flow_time_max;
		ret = tm_prop_flow_time_max_notify(NULL, water_back.flow_time_max, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);
	}
	if(water_back_cloud.flow_time_min != water_back.flow_time_min){
		
		water_back_cloud.flow_time_min = water_back.flow_time_min;
		ret = tm_prop_flow_time_min_notify(NULL, water_back.flow_time_min, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);

	}
	if(water_back_cloud.onoff != water_back.onoff){
		
		water_back_cloud.onoff = water_back.onoff;
		ret = tm_prop_onoff_notify(NULL, water_back.onoff, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);	
	}
	if(water_back_cloud.work_time != water_back.work_time){
		
		water_back_cloud.work_time = water_back.work_time;
		ret = tm_prop_work_time_notify(NULL, water_back.work_time, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);
	}

	if(water_back_cloud.temperature != water_back.temperature){
		
		water_back_cloud.temperature = water_back.temperature;
		ret = tm_prop_temperature_notify(NULL, water_back.temperature, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);
	}
	if(water_back_cloud.mode != water_back.mode){
		
		water_back_cloud.mode = water_back.mode;
		ret = tm_prop_mode_notify(NULL, water_back.mode, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);

	}
	if(water_back_cloud.supercharge != water_back.supercharge){
		
		water_back_cloud.supercharge = water_back.supercharge;
		ret = tm_prop_supercharge_notify(NULL, water_back.supercharge, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);

	}
	if(water_back_cloud.vacation != water_back.vacation){
		
		water_back_cloud.vacation = water_back.vacation;
		ret = tm_prop_vacation_notify(NULL, water_back.vacation, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);

	}
	if(water_back_cloud.protection != water_back.protection){
		water_back_cloud.protection = water_back.protection;
		if(water_back.protection){
			struct event_error_t event_error;
			event_error.error_code = 0;
			ret = tm_event_error_notify(NULL, event_error, now, 1000);
			APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);
		}
	}

	if(0 != memcmp(water_back_cloud.timer, &water_back.timer, sizeof(water_back_cloud.timer))){

		memcpy(water_back_cloud.timer, water_back.timer, sizeof(water_back_cloud.timer) );
		struct prop_local_timer_t val;
		// report all timer
		ret = tm_prop_local_timer_notify(NULL, val, now, 1000);
		APP_ERROR_CHECK(ret == ERR_OK, "tm notify fail", ret);
	}

	return 0;
}
#endif
void vLoadRecord( void)
{
	// read data from falsh
	nvs_handle handle;
	uint32_t len = sizeof(water_back_store);
	esp_err_t ret = nvs_open( NVS_WB_NAME, NVS_READONLY, &handle);
	if(ret != ESP_OK)
		return;
	ESP_ERROR_CHECK ( nvs_get_blob(handle, NVS_WB_NAME, &water_back_store, &len) );
	nvs_close(handle);
}
void vSaveRecord( void)
{
	bool save = false;
	
	if(water_back_store.flow_time_max != water_back.flow_time_max){
		water_back_store.flow_time_max = water_back.flow_time_max;
		save = true;
	}
	if(water_back_store.flow_time_min != water_back.flow_time_min){
		water_back_store.flow_time_min = water_back.flow_time_min;
		save = true;
	}
	if(water_back_store.mode != water_back.mode){
		water_back_store.mode = water_back.mode;
		save = true;
	}
	if(water_back_store.protection != water_back.protection){
		water_back_store.protection = water_back.protection;
		save = true;
	}
	if(water_back_store.supercharge != water_back.supercharge){
		water_back_store.supercharge = water_back.supercharge;
		save = true;
	}
	if(water_back_store.target_temperature != water_back.target_temperature){
		water_back_store.target_temperature = water_back.target_temperature;
		save = true;
	}
	if(water_back_store.vacation != water_back.vacation){
		water_back_store.vacation = water_back.vacation;
		save = true;
	}
	if(water_back_store.work_time != water_back.work_time){
		water_back_store.work_time = water_back.work_time;
		save = true;
	}
	if(0 != memcmp(water_back_store.timer, water_back.timer, sizeof(water_back_store.timer))){
		memcpy(water_back_store.timer, water_back.timer, sizeof(water_back_store.timer));
		save = true;
	}
	if(water_back_store.wifi_configed != water_back.wifi_configed){
		water_back_store.wifi_configed = water_back.wifi_configed;
		save = true;
	}
	if(0 != memcmp(water_back_store.ssid, water_back.ssid, sizeof(water_back_store.ssid))){
		memcpy(water_back_store.ssid, water_back.ssid, sizeof(water_back_store.ssid));
		save = true;
	}
	if(0 != memcmp(water_back_store.password, water_back.password, sizeof(water_back_store.password))){
		memcpy(water_back_store.password, water_back.password, sizeof(water_back_store.password));
		save = true;
	}

	if(save){
		nvs_handle handle;
		ESP_ERROR_CHECK( nvs_open( NVS_WB_NAME, NVS_READWRITE, &handle));
		ESP_ERROR_CHECK ( nvs_set_blob(handle, NVS_WB_NAME, &water_back_store, sizeof(water_back_store)) );
		ESP_ERROR_CHECK( nvs_commit(handle) );
		nvs_close(handle);
	}
}
void vDefaultRecords(void)
{
	water_back_store.flow_time_max = 6;
	water_back_store.flow_time_min = 2;
	water_back_store.mode = E_WB_MODE_WATER_CONTROL;
	water_back_store.protection = false;
	water_back_store.supercharge = false;
	water_back_store.target_temperature = 38;
	water_back_store.vacation = false;
	water_back_store.work_time = 120;
	
	water_back_store.timer[0].enable = true;
	strcpy(water_back_store.timer[0].time_start, "06:00");
	strcpy(water_back_store.timer[0].time_end, "08:00");
	water_back_store.timer[1].enable = true;
	strcpy(water_back_store.timer[1].time_start, "12:00");
	strcpy(water_back_store.timer[1].time_end, "14:00");
	water_back_store.timer[2].enable = true;
	strcpy(water_back_store.timer[2].time_start, "18:00");
	strcpy(water_back_store.timer[2].time_end, "21:00");

}
void vCopyRecordsToApp( void)
{
	water_back.flow_time_max = water_back_store.flow_time_max;
	water_back.flow_time_min = water_back_store.flow_time_min;
	water_back.mode = water_back_store.mode;
	water_back.protection = water_back_store.protection;
	water_back.supercharge = water_back_store.supercharge;
	water_back.target_temperature = water_back_store.target_temperature;
	water_back.vacation = water_back_store.vacation;
	water_back.work_time = water_back_store.work_time;
	memcpy(water_back.timer ,water_back_store.timer, sizeof(water_back.timer));
	water_back.wifi_configed = water_back_store.wifi_configed;
	memcpy(water_back.ssid ,water_back_store.ssid, sizeof(water_back.ssid));
	memcpy(water_back.password ,water_back_store.password, sizeof(water_back.password));
}
void vResetRecords( void)
{
	vDefaultRecords();
	vCopyRecordsToApp();
	vSaveRecord();
}



