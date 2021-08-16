/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.c
 * @date 2020/05/14
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"

#include "tm_data.h"
#include "tm_api.h"
#include "tm_user.h"
#include "app_water_back.h"
#include "app_common.h"
#include "app_ota.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
/*************************** Property Func List ******************************/
struct tm_prop_tbl_t tm_prop_list[] = {
    TM_PROPERTY_RO(app_version),
    TM_PROPERTY_RO(flow_rate),
    TM_PROPERTY_RW(flow_time_max),
    TM_PROPERTY_RW(flow_time_min),
    TM_PROPERTY_RW(local_timer),
    TM_PROPERTY_RW(mode),
    TM_PROPERTY_RW(onoff),
    TM_PROPERTY_RW(protection),
    TM_PROPERTY_RO(remaining_work_time),
    TM_PROPERTY_RW(supercharge),
    TM_PROPERTY_RW(target_temperature),
    TM_PROPERTY_RO(temperature),
    TM_PROPERTY_RW(vacation),
    TM_PROPERTY_RO(wifi_info),
    TM_PROPERTY_RW(work_time)
};
uint16_t tm_prop_list_size = ARRAY_SIZE(tm_prop_list);
/****************************** Auto Generated *******************************/


/***************************** Service Func List *******************************/
struct tm_svc_tbl_t tm_svc_list[] = {
    TM_SERVICE(firmware_upgrade)
};
uint16_t tm_svc_list_size = ARRAY_SIZE(tm_svc_list);
/****************************** Auto Generated *******************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
static const char *TAG = "tm_user";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
/**************************** Property Func Read *****************************/
int32_t tm_prop_app_version_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = APP_VERSION;
	
    tm_data_struct_set_int32(data, "app_version", val);

    return 0;
}

int32_t tm_prop_flow_rate_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.flow_rate;


    tm_data_struct_set_int32(data, "flow_rate", val);

    return 0;
}

int32_t tm_prop_flow_time_max_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.flow_time_max;

    tm_data_struct_set_int32(data, "flow_time_max", val);

    return 0;
}

int32_t tm_prop_flow_time_min_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.flow_time_min;

    tm_data_struct_set_int32(data, "flow_time_min", val);

    return 0;
}

int32_t tm_prop_local_timer_rd_cb(void *data)
{
    void *structure = NULL;
    struct prop_local_timer_t val;

    /** 根据实际情况修改需要返回的数组元素个数*/
    int32_t element_cnt = 3;
    void *array = tm_data_array_create(element_cnt);
    int32_t i = 0;

    for(i = 0; i < element_cnt; i++)
    {
        /** 根据业务逻辑获取功能点值，设置到val */
		val.time_start = water_back.timer[i].time_start;
		val.time_end = water_back.timer[i].time_end;
		val.enable =  water_back.timer[i].enable;
		val.is_valid = water_back.timer[i].is_valid;
		
        structure = tm_data_struct_create();
        tm_data_struct_set_string(structure, "time_start", val.time_start);
        tm_data_struct_set_bool(structure, "enable", val.enable);
        tm_data_struct_set_bool(structure, "is_valid", val.is_valid);
        tm_data_struct_set_string(structure, "time_end", val.time_end);
        tm_data_array_set_struct(array, structure);

    }

    tm_data_struct_set_data(data, "local_timer", array);

    return 0;
}

int32_t tm_prop_mode_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.mode;

    tm_data_struct_set_enum(data, "mode", val);

    return 0;
}

int32_t tm_prop_onoff_rd_cb(void *data)
{
    boolean val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	ESP_LOGD(TAG, "read onoff %d", water_back.onoff);
	val = water_back.onoff;

    tm_data_struct_set_bool(data, "onoff", val);

    return 0;
}

int32_t tm_prop_protection_rd_cb(void *data)
{
    boolean val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.protection;

    tm_data_struct_set_bool(data, "protection", val);

    return 0;
}

int32_t tm_prop_remaining_work_time_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.remaining_work_time;

    tm_data_struct_set_int32(data, "remaining_work_time", val);

    return 0;
}

int32_t tm_prop_supercharge_rd_cb(void *data)
{
    boolean val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.supercharge;

    tm_data_struct_set_bool(data, "supercharge", val);

    return 0;
}

int32_t tm_prop_target_temperature_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.target_temperature;


    tm_data_struct_set_int32(data, "target_temperature", val);

    return 0;
}

int32_t tm_prop_temperature_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.temperature;


    tm_data_struct_set_int32(data, "temperature", val);

    return 0;
}

int32_t tm_prop_vacation_rd_cb(void *data)
{
    boolean val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.vacation;


    tm_data_struct_set_bool(data, "vacation", val);

    return 0;
}

int32_t tm_prop_wifi_info_rd_cb(void *data)
{
    void *structure = tm_data_struct_create();
    struct prop_wifi_info_t val;

    /** 根据业务逻辑获取功能点值，设置到val */
	wifi_config_t conf;
	esp_wifi_get_config(ESP_IF_WIFI_STA, &conf);
	val.ssid = (char *)conf.sta.ssid;
	val.password = (char *)conf.sta.password;

    tm_data_struct_set_string(structure, "ssid", val.ssid);
    tm_data_struct_set_string(structure, "password", val.password);

    tm_data_struct_set_data(data, "wifi_info", structure);

    return 0;
}

int32_t tm_prop_work_time_rd_cb(void *data)
{
    int32_t val = 0;

    /** 根据业务逻辑获取功能点值，设置到val */
	val = water_back.work_time;


    tm_data_struct_set_int32(data, "work_time", val);

    return 0;
}

/****************************** Auto Generated *******************************/


/**************************** Property Func Write ****************************/
int32_t tm_prop_flow_time_max_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.flow_time_max = val;
	vSaveRecord();
    return 0;
}

int32_t tm_prop_flow_time_min_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.flow_time_min = val;
	vSaveRecord();
    return 0;
}

int32_t tm_prop_local_timer_wr_cb(void *data)
{
    struct prop_local_timer_t val;
    int32_t element_cnt = tm_data_array_size(data);
    void *element = NULL;
    int32_t i = 0;

    /** 根据业务逻辑获取功能点值，设置到val */

    for(i = 0; i < element_cnt; i++)
    {
        element = tm_data_array_get_element(data, i);
        tm_data_struct_get_string(element, "time_start", &val.time_start);
        tm_data_struct_get_bool(element, "enable", &val.enable);
        tm_data_struct_get_bool(element, "is_valid", &val.is_valid);
        tm_data_struct_get_string(element, "time_end", &val.time_end);


        /** 根据变量val的值，填入下发控制逻辑 */
		strncpy(water_back.timer[i].time_start, val.time_start, sizeof(water_back.timer[i].time_start));
		strncpy(water_back.timer[i].time_end, val.time_end, sizeof(water_back.timer[i].time_end));
		water_back.timer[i].enable = val.enable;
		water_back.timer[i].is_valid = val.is_valid;
		
		vSaveRecord();
    }

    return 0;
}

int32_t tm_prop_mode_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_enum(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back_set_onoff(false, 0);
	water_back.mode = val;
	vSaveRecord();
    return 0;
}

int32_t tm_prop_onoff_wr_cb(void *data)
{
    boolean val = 0;

    tm_data_get_bool(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	if(E_WB_MODE_WATER_CONTROL == water_back.mode){
		water_back_set_onoff((bool)val, water_back.work_time);
	}else if(E_WB_MODE_TIMER == water_back.mode) {
		if( !water_back.time_up ){
			water_back_set_onoff((bool)val, water_back.work_time);
		}
	}

    return 0;
}

int32_t tm_prop_protection_wr_cb(void *data)
{
    boolean val = 0;

    tm_data_get_bool(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.protection = val;
	water_back.protection_status = false;

    return 0;
}

int32_t tm_prop_supercharge_wr_cb(void *data)
{
    boolean val = 0;

    tm_data_get_bool(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.supercharge = val;

	vSaveRecord();
    return 0;
}

int32_t tm_prop_target_temperature_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	if(val > 99)
		val = 99;
	if(val < 3)
		val = 3;
	water_back.target_temperature = val;
	vSaveRecord();

    return 0;
}

int32_t tm_prop_vacation_wr_cb(void *data)
{
    boolean val = 0;

    tm_data_get_bool(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.vacation = val;

	vSaveRecord();


    return 0;
}

int32_t tm_prop_work_time_wr_cb(void *data)
{
    int32_t val = 0;

    tm_data_get_int32(data, &val);

    /** 根据变量val的值，填入下发控制逻辑 */
	water_back.work_time = val;
	vSaveRecord();


    return 0;
}

/****************************** Auto Generated *******************************/


/**************************** Service Func Invoke ****************************/
static void cb_app_ota_upgrade_percent_process(int percent)
{
	int ret;
	if(water_back.wifi_status!=E_WB_WIFI_CONNECT){
		return;
	}

	struct event_upgrade_percent_t val;
	val.percent = percent;
	tm_event_upgrade_percent_notify(NULL, val, 0, 1000);
}

int32_t tm_svc_firmware_upgrade_cb(void *in_data, void *out_data)
{
    struct svc_firmware_upgrade_in_t in_param;
    struct svc_firmware_upgrade_out_t out_param;

    int32_t ret = 0;

    tm_data_struct_get_string(in_data, "file_name", &in_param.file_name);
    tm_data_struct_get_string(in_data, "server_ip", &in_param.server_ip);
    tm_data_struct_get_string(in_data, "server_port", &in_param.server_port);

    /** 根据输入参数，生成输出参数 */
	app_ota_set_upgrade_percent_process_cb(cb_app_ota_upgrade_percent_process);
	app_ota_start((const char *)in_param.server_ip, (const char *) in_param.server_port, (const char *)in_param.file_name);

    return ret;
}

/****************************** Auto Generated *******************************/


/**************************** Property Func Notify ***************************/
int32_t tm_prop_app_version_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "app_version", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_flow_rate_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "flow_rate", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_flow_time_max_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "flow_time_max", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_flow_time_min_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "flow_time_min", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_local_timer_notify(void *data, struct prop_local_timer_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    int32_t element_cnt = 3;
    void *array = tm_data_array_create(element_cnt);
    int32_t i = 0;

    for(i = 0; i < element_cnt; i++)
    {
        /** 根据业务逻辑获取功能点值，设置到val */
		val.time_start = water_back.timer[i].time_start;
		val.time_end = water_back.timer[i].time_end;
		val.enable =  water_back.timer[i].enable;
		val.is_valid = water_back.timer[i].is_valid;
		
        structure = tm_data_struct_create();
        tm_data_struct_set_string(structure, "time_start", val.time_start);
        tm_data_struct_set_bool(structure, "enable", val.enable);
        tm_data_struct_set_bool(structure, "is_valid", val.is_valid);
        tm_data_struct_set_string(structure, "time_end", val.time_end);
        tm_data_array_set_struct(array, structure);

    }

    tm_data_set_struct(resource, "local_timer", array, timestamp);


    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}
int32_t tm_prop_mode_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_enum(resource, "mode", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_onoff_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_bool(resource, "onoff", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_protection_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_bool(resource, "protection", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_remaining_work_time_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "remaining_work_time", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_supercharge_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_bool(resource, "supercharge", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_target_temperature_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "target_temperature", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_temperature_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "temperature", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_vacation_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_bool(resource, "vacation", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_wifi_info_notify(void *data, struct prop_wifi_info_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_struct_set_string(structure, "ssid", val.ssid);
    tm_data_struct_set_string(structure, "password", val.password);

    tm_data_set_struct(resource, "wifi_info", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_work_time_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "work_time", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

/****************************** Auto Generated *******************************/


/***************************** Event Func Notify *****************************/
int32_t tm_event_error_notify(void *data, struct event_error_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_struct_set_enum(structure, "error_code", val.error_code);

    tm_data_set_struct(resource, "error", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_event(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_event_upgrade_percent_notify(void *data, struct event_upgrade_percent_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_struct_set_int32(structure, "percent", val.percent);

    tm_data_set_struct(resource, "upgrade_percent", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_event(resource, timeout_ms);
    }

    return ret;
}

/****************************** Auto Generated *******************************/

