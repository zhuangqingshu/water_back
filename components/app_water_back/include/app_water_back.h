/**
  ******************************************************************************
  * @file    
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */ 
#ifndef __APP_WATER_BACK_H_
#define __APP_WATER_BACK_H_

/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/* Public typedef ------------------------------------------------------------*/

/*
"0": "水控模式",
"1": "自动模式"
"2": "定时模式",
*/
typedef enum {
	E_WB_MODE_WATER_CONTROL = 0,
	E_WB_MODE_AUTO,
	E_WB_MODE_TIMER,
	E_WB_MODE_MAX
}water_back_mode_t;

typedef enum {
	E_WB_WIFI_UNCONNECT = 0,
	E_WB_WIFI_CONFIG,
	E_WB_WIFI_CONNECT,
}water_back_wifi_status_t;

typedef struct water_back_timer {
	bool enable;
	bool is_valid;
	char time_start[16];
	char time_end[16];
}water_back_timer_t;

typedef struct
{
	int32_t flow_time_max;
    int32_t flow_time_min;
	int32_t flow_rate; // 流速
	int32_t flow_time; // 水流流动时间
	int32_t flow_time_escape; // 水流最终流动时间
	
	bool onoff;  // "true": "开启工作",  "false": "停止工作"
	
	int32_t work_time; // 预设工作延时
	int32_t work_time_real; // 工作延时
	time_t start_work_time; // 开始工作时间
	int32_t remaining_work_time; //"剩余工作时间",

	int32_t target_temperature;// 设定温度
	int32_t temperature;// 检测温度

	water_back_mode_t mode;
	bool supercharge; // 增压功能
	bool supercharge_status;
	bool vacation;//度假功能
	bool vacation_status; // 度假状态
	bool protection; // 水泵保护功能
	bool protection_status; // 水泵保护状态
	bool temp_protection_status; // 低温保护状态

	bool time_up;
	water_back_timer_t timer[3];

	water_back_wifi_status_t wifi_status;
	bool wifi_configed;
	uint8_t ssid[32];      /**< SSID of target AP*/
    uint8_t password[64];  /**< password of target AP*/
}water_back_t;


typedef struct
{
	int32_t flow_time_max;
    int32_t flow_time_min;
	int32_t flow_rate; // 流速

	bool onoff;  // "true": "开启工作",  "false": "停止工作"
	int32_t work_time; // 工作延时
	int32_t remaining_work_time; //"剩余工作时间",

	int32_t target_temperature;// 设定温度
	int32_t temperature;// 检测温度

	water_back_mode_t mode;
	bool supercharge; // 增压功能
	bool vacation;//度假功能
	bool protection; // 保护功能

	water_back_timer_t timer[3];

}water_back_cloud_t;

typedef struct
{
	int32_t work_time;// 工作延时
	water_back_mode_t mode;
	int32_t flow_time_max;
    int32_t flow_time_min;
	int32_t target_temperature;// 设定温度
	bool supercharge; // 增压功能
	bool vacation;//度假功能
	bool protection; // 保护功能
	water_back_timer_t timer[3];
	bool wifi_configed;
    uint8_t ssid[32];      /**< SSID of target AP*/
    uint8_t password[64];  /**< password of target AP*/
}water_back_store_t;

/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern water_back_t water_back;

/* Public function prototypes ------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void water_back_init(void);
void water_back_set_onoff(bool onoff, int work_time);
int water_back_auto_report();

void vLoadRecord( void);
void vSaveRecord( void);
void vDefaultRecords(void);
void vCopyRecordsToApp( void);
void vResetRecords( void);

#ifdef __cplusplus
}
#endif
#endif /* __APP_WATER_BACK_H_ */
/********************************* END OF FILE *********************************/

