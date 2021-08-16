/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_user.h
 * @date 2020/05/14
 * @brief
 */

#ifndef __TM_USER_H__
#define __TM_USER_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "tm_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
/****************************** Structure type *******************************/
struct event_error_t
{
    int32_t error_code;
};
struct event_upgrade_percent_t
{
    int32_t percent;
};

struct prop_local_timer_t
{
    char *time_start;
    boolean enable;
    boolean is_valid;
    char *time_end;
};

struct prop_wifi_info_t
{
    char *ssid;
    char *password;
};

struct svc_firmware_upgrade_in_t
{
    char *file_name;
    char *server_ip;
    char *server_port;
};
struct svc_firmware_upgrade_out_t
{
};
/****************************** Auto Generated *******************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/*************************** Property Func List ******************************/
extern struct tm_prop_tbl_t tm_prop_list[];
extern uint16_t tm_prop_list_size;
/****************************** Auto Generated *******************************/


/**************************** Service Func List ******************************/
extern struct tm_svc_tbl_t tm_svc_list[];
extern uint16_t tm_svc_list_size;
/****************************** Auto Generated *******************************/


/**************************** Property Func Read ****************************/
int32_t tm_prop_app_version_rd_cb(void *data);
int32_t tm_prop_flow_rate_rd_cb(void *data);
int32_t tm_prop_flow_time_max_rd_cb(void *data);
int32_t tm_prop_flow_time_min_rd_cb(void *data);
int32_t tm_prop_local_timer_rd_cb(void *data);
int32_t tm_prop_mode_rd_cb(void *data);
int32_t tm_prop_onoff_rd_cb(void *data);
int32_t tm_prop_protection_rd_cb(void *data);
int32_t tm_prop_remaining_work_time_rd_cb(void *data);
int32_t tm_prop_supercharge_rd_cb(void *data);
int32_t tm_prop_target_temperature_rd_cb(void *data);
int32_t tm_prop_temperature_rd_cb(void *data);
int32_t tm_prop_vacation_rd_cb(void *data);
int32_t tm_prop_wifi_info_rd_cb(void *data);
int32_t tm_prop_work_time_rd_cb(void *data);
/****************************** Auto Generated *******************************/


/**************************** Property Func Write ****************************/
int32_t tm_prop_flow_time_max_wr_cb(void *data);
int32_t tm_prop_flow_time_min_wr_cb(void *data);
int32_t tm_prop_local_timer_wr_cb(void *data);
int32_t tm_prop_mode_wr_cb(void *data);
int32_t tm_prop_onoff_wr_cb(void *data);
int32_t tm_prop_protection_wr_cb(void *data);
int32_t tm_prop_supercharge_wr_cb(void *data);
int32_t tm_prop_target_temperature_wr_cb(void *data);
int32_t tm_prop_vacation_wr_cb(void *data);
int32_t tm_prop_work_time_wr_cb(void *data);
/****************************** Auto Generated *******************************/


/**************************** Service Func Invoke ****************************/
int32_t tm_svc_firmware_upgrade_cb(void *in_data, void *out_data);
/****************************** Auto Generated *******************************/


/**************************** Property Func Notify ***************************/
int32_t tm_prop_app_version_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_flow_rate_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_flow_time_max_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_flow_time_min_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_local_timer_notify(void *data, struct prop_local_timer_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_mode_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_onoff_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_remaining_work_time_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_supercharge_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_target_temperature_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_temperature_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_vacation_notify(void *data, boolean val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_wifi_info_notify(void *data, struct prop_wifi_info_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_prop_work_time_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms);
/****************************** Auto Generated *******************************/


/***************************** Event Func Notify *****************************/
int32_t tm_event_error_notify(void *data, struct event_error_t val, uint64_t timestamp, uint32_t timeout_ms);
int32_t tm_event_upgrade_percent_notify(void *data, struct event_upgrade_percent_t val, uint64_t timestamp, uint32_t timeout_ms);
/****************************** Auto Generated *******************************/

#ifdef __cplusplus
}
#endif

#endif
