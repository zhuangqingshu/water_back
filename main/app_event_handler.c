/*****************************************************************************
 *
 * MODULE:          JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:       app_event_handler.c
 *
 * DESCRIPTION:     ZLO Demo: Handles all the different type of Application events
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2017. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "app_common.h"
#include "app_events.h"
#include "app_event_handler.h"
#include "app_led_interface.h"
#include "app_blink_led.h"
#include "board.h"
#include "app_water_back.h"
#include "app_menu.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void vDioEventHandler(te_TransitionCode eTransitionCode);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static const char *TAG = "event_handler";

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern xTimerHandle menu_exit_timer;
extern xTimerHandle back_led_timer;

extern void start_smartconfig();

/****************************************************************************
 *
 * NAME: menu_main_button_down_handle
 *
 * DESCRIPTION:
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
int menu_main_event_handle(APP_tsEvent *sEvent)
{
	switch(sEvent->eType)
	{
		case APP_E_EVENT_BUTTON_DOWN:
			ESP_LOGD(TAG, "Button [%d] Down", sEvent->uEvent.sButton.u8Button);
			break;
		case APP_E_EVENT_BUTTON_UP:
			ESP_LOGD(TAG, "Button [%d] Up", sEvent->uEvent.sButton.u8Button);
			break;

	    case APP_E_EVENT_BUTTON_SINGLE_CLICK:
			ESP_LOGD(TAG, "Button [%d] single click", sEvent->uEvent.sButton.u8Button);
	    	switch(sEvent->uEvent.sButton.u8Button){
				case APP_E_BUTTONS_BUTTON_1:{
					// srart work
					if(E_WB_MODE_WATER_CONTROL == water_back.mode){
						water_back_set_onoff(!water_back.onoff, water_back.work_time);
						water_back_auto_report();
					}else if(E_WB_MODE_TIMER == water_back.mode) {
						if( !water_back.time_up ){
							water_back_set_onoff(!water_back.onoff, water_back.work_time);
							water_back_auto_report();
						}
					}
				}
					break;
				case APP_E_BUTTONS_BUTTON_2:
					// setting 
					// TODO add mutx for current_menu;
					current_menu = app_menu_get_handler(E_APP_MENU_ID_SETTING);
					ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
					current_menu->curros = 0;
					// start count down 20S,for exit setting.
					xTimerStart(menu_exit_timer, 100);
					break;
					
				case APP_E_BUTTONS_BUTTON_5:
					// change mode
					water_back_set_onoff(false, 0);
					water_back.mode++;
					if(water_back.mode >= E_WB_MODE_MAX){
						water_back.mode = 0;
					}
					ESP_LOGI(TAG, "Water back mode %d", water_back.mode);
					vSaveRecord();
					water_back_auto_report();
					break;
				default:
					break;
			}
			break;
			
		case APP_E_EVENT_BUTTON_SINGLE_LONG_CLICK:
			switch(sEvent->uEvent.sButton.u8Button){
				case APP_E_BUTTONS_BUTTON_1:
					water_back.supercharge = !water_back.supercharge;
					water_back.supercharge_status = false;
					ESP_LOGI(TAG, "Water back supercharge %d", water_back.supercharge);
					vSaveRecord();
					water_back_auto_report();
					break;
				case APP_E_BUTTONS_BUTTON_2:{
					// timer setting 
					current_menu = app_menu_get_handler(E_APP_MENU_ID_TIMER_SETTING);
					ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
					ESP_ERROR_CHECK(current_menu->context != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
					current_menu->curros = 0;
					time_t now;
				    time(&now);
				    localtime_r(&now, (struct tm *)current_menu->context);
					// start count down 20S,for exit setting.
					xTimerStart(menu_exit_timer, 100);
				}
					break;

#ifndef ENABLE_UART_LOG_OUT
				case APP_E_BUTTONS_BUTTON_3:
					// recovery
					water_back_set_onoff(false, 0);
					vResetRecords();
					water_back_auto_report();
					break;
#endif

#ifndef ENABLE_UART_LOG_OUT
				case APP_E_BUTTONS_BUTTON_4:
#else
				case APP_E_BUTTONS_BUTTON_3:
#endif
					// start smartconfig
					//start_smartconfig();
					water_back.wifi_configed = false;
					vSaveRecord();
					esp_restart();
					break;
				case APP_E_BUTTONS_BUTTON_5:
					water_back.protection = !water_back.protection;
					water_back.protection_status = false;
					ESP_LOGI(TAG, "Water back protection %d", water_back.protection);
					vSaveRecord();
					water_back_auto_report();
					break;

				default:
					break;
			}
			break;
		default:
			ESP_LOGD(TAG, "APP Event: Unhandle event %d", sEvent->eType);
			break;
	}
	return 0;
}


int menu_setting_event_handle(APP_tsEvent *sEvent)
{
	switch(sEvent->eType)
	{
	    case APP_E_EVENT_BUTTON_SINGLE_CLICK:
			ESP_LOGD(TAG, "Button [%d] single click", sEvent->uEvent.sButton.u8Button);
			xTimerReset(menu_exit_timer, 100);
	    	switch(sEvent->uEvent.sButton.u8Button){
				case APP_E_BUTTONS_BUTTON_2:{
					// mode
					current_menu->curros++;
					if(current_menu->curros > 7){
						// save params
						vSaveRecord();
						water_back_auto_report();
						// exit 
						xTimerStop(menu_exit_timer, 100);
						current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
						ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
						
					}
				}
				break;
				case APP_E_BUTTONS_BUTTON_3:
					// up
					switch (current_menu->curros)
					{
						case 0:
							if((water_back.target_temperature + 10) <= 99)
								water_back.target_temperature += 10;
						break;
						case 1:
							if((water_back.target_temperature + 1) <= 99)
								water_back.target_temperature += 1;
						break;
						case 2:
							if((water_back.work_time + 10 * 60) <= 5999)
								water_back.work_time += 10 * 60;
						break;
						case 3:
							if((water_back.work_time+ 60)  <= 5999)
								water_back.work_time += 60;
						break;
						case 4:
							if((water_back.work_time + 10) <= 5999)
								water_back.work_time += 10;
						break;
						case 5:
							if((water_back.work_time) + 1 <= 5999)
								water_back.work_time += 1;
						break;
						case 6:
							if((water_back.flow_time_min+ 1) <= 9)
								water_back.flow_time_min += 1;
						break;
						case 7:
							if((water_back.flow_time_max + 1) <= 9)
								water_back.flow_time_max += 1;
						break;
						default:
						break;
					}
				break;
					
				case APP_E_BUTTONS_BUTTON_4:
					// down
					switch (current_menu->curros)
					{
						case 0:
							if(water_back.target_temperature >= 13)
								water_back.target_temperature -= 10;
						break;
						case 1:
							if(water_back.target_temperature >= 4)
								water_back.target_temperature -= 1;
						break;
						case 2:
							if(water_back.work_time >= 10 * 60)
								water_back.work_time -= 10 * 60;
						break;
						case 3:
							if(water_back.work_time >= 60)
								water_back.work_time -= 60;
						break;
						case 4:
							if(water_back.work_time >= 10)
								water_back.work_time -= 10;
						break;
						case 5:
							if(water_back.work_time >= 1)
								water_back.work_time -= 1;
						break;
						case 6:
							if(water_back.flow_time_min >= 1)
								water_back.flow_time_min -= 1;
						break;
						case 7:
							if(water_back.flow_time_max >= 1)
								water_back.flow_time_max -= 1;
						break;
						default:
						break;
					}
					break;

				case APP_E_BUTTONS_BUTTON_5:
					// ok
					// save params
					vSaveRecord();
					water_back_auto_report();
					// exit 
					xTimerStop(menu_exit_timer, 100);
					current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
					ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
					break;
				default:
					break;
			}
			break;
			
		default:
			ESP_LOGD(TAG, "APP Event: Unhandle event %d", sEvent->eType);
			break;
	}
	return 0;

}

int menu_timer_setting_event_handle(APP_tsEvent *sEvent)
{
	switch(sEvent->eType)
	{
	    case APP_E_EVENT_BUTTON_SINGLE_CLICK:
			ESP_LOGD(TAG, "Button [%d] single click", sEvent->uEvent.sButton.u8Button);
			xTimerReset(menu_exit_timer, 100);
	    	switch(sEvent->uEvent.sButton.u8Button){
				case APP_E_BUTTONS_BUTTON_2:{
					// mode
					current_menu->curros++;
					if(current_menu->curros > 27){
						current_menu->curros = 0;
						// set time
						struct timeval set_time;
						struct tm * timeinfo = (struct tm *)current_menu->context;
						ESP_LOGD(TAG, "Set local time %d %d:%d", timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min);
						set_time.tv_sec = (long)mktime(timeinfo);
						set_time.tv_usec = 0;
						ESP_LOGD(TAG, "Set time %ld ", set_time.tv_sec);
						settimeofday(&set_time, NULL);
						// TODO save params
						vSaveRecord();
						water_back_auto_report();
						// exit 
						xTimerStop(menu_exit_timer, 100);
						current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
						ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
					}
				}
				break;
				
				case APP_E_BUTTONS_BUTTON_3:{
					// up
					if(current_menu->curros >= 0 && current_menu->curros < 4){
						struct tm *timeinfo = (struct tm *)current_menu->context;
						switch (current_menu->curros)
						{
							case 0:
								if((timeinfo->tm_hour + 10) <= 23)
									timeinfo->tm_hour += 10;
							break;
							case 1:
								if((timeinfo->tm_hour + 1) <= 23)
									timeinfo->tm_hour += 1;
							break;
							case 2:
								if((timeinfo->tm_min + 10) <= 59)
									timeinfo->tm_min += 10;
							break;
							case 3:
								if((timeinfo->tm_min) + 1 <= 59)
									timeinfo->tm_min += 1;
							break;
							default:
							break;
						}

					}
					else if(current_menu->curros >= 4 && current_menu->curros <= 27){
						int hour=0,minute=0;
						uint8_t timer_index = current_menu->curros / 4 - 1; // 0 1 2 3 4 5
						if(timer_index % 2 == 0){
							APP_ERROR_CHECK((2==sscanf(water_back.timer[timer_index/2].time_start, "%d:%d", &hour,&minute)), "sscanf time", ESP_ERR_INVALID_ARG);
						}else{
							APP_ERROR_CHECK((2==sscanf(water_back.timer[timer_index/2].time_end, "%d:%d", &hour,&minute)), "sscanf time", ESP_ERR_INVALID_ARG);
						}
						if(current_menu->curros % 4 == 0){
							if((hour + 10) <= 23)
								hour += 10;
						
						}else if(current_menu->curros % 4 == 1){
							if((hour + 1) <= 23)
								hour += 1;
						
						}else if(current_menu->curros % 4 == 2){
							if((minute + 10) <= 59)
								minute += 10;
						
						}else if(current_menu->curros % 4 == 3){
							if((minute + 1) <= 59)
								minute += 1;
						}
						if(timer_index % 2 == 0){
							sprintf(water_back.timer[timer_index/2].time_start, "%d:%d", hour, minute);
							ESP_LOGD(TAG, "Set start timer %s", water_back.timer[timer_index / 2].time_start);
						}else{
							sprintf(water_back.timer[timer_index/2].time_end, "%d:%d", hour, minute);
							ESP_LOGD(TAG, "Set end timer %s", water_back.timer[timer_index / 2].time_end);
						}
						ESP_LOGD(TAG, " New %s time [%d] %d:%d", timer_index % 2 == 0 ? "start" : "end", timer_index / 2, hour, minute);
						
					}
				}
				break;
					
				case APP_E_BUTTONS_BUTTON_4:{
					// down
					if(current_menu->curros >= 0 && current_menu->curros < 4){
						struct tm *timeinfo = (struct tm *)current_menu->context;
						switch (current_menu->curros)
						{
							case 0:
								if(timeinfo->tm_hour >= 10)
									timeinfo->tm_hour -= 10;
							break;
							case 1:
								if(timeinfo->tm_hour >= 1)
									timeinfo->tm_hour -= 1;
							break;
							case 2:
								if(timeinfo->tm_min >= 10)
									timeinfo->tm_min -= 10;
							break;
							case 3:
								if(timeinfo->tm_min >= 1)
									timeinfo->tm_min -= 1;
							break;
							default:
							break;
						}

					}
					else if(current_menu->curros >= 4 && current_menu->curros <= 27){
						int hour=0,minute=0;
						uint8_t timer_index = current_menu->curros / 4 - 1; // 0 1 2 3 4 5
						if(timer_index % 2 == 0){
							sscanf(water_back.timer[timer_index / 2].time_start, "%d:%d", &hour,&minute);
						}else{
							sscanf(water_back.timer[timer_index / 2].time_end, "%d:%d", &hour,&minute);
						}
						if(current_menu->curros % 4 == 0){
							if(hour >= 10)
								hour -= 10;
						
						}else if(current_menu->curros % 4 == 1){
							if(hour >= 1)
								hour -= 1;
						
						}else if(current_menu->curros % 4 == 2){
							if(minute >= 10)
								minute -= 10;
						
						}else if(current_menu->curros % 4 == 3){
							if(minute >= 1)
								minute -= 1;
						}
						if(timer_index % 2 == 0){
							sprintf(water_back.timer[timer_index / 2].time_start, "%d:%d", hour, minute);
						}else{
							sprintf(water_back.timer[timer_index / 2].time_end, "%d:%d", hour, minute);
						}

					}

				}
					break;

				case APP_E_BUTTONS_BUTTON_5:{
					// ok
					// TODO set time
					struct timeval set_time;
					struct tm * timeinfo = (struct tm *)current_menu->context;
					ESP_LOGD(TAG, "Set local time %d %d:%d", timeinfo->tm_year, timeinfo->tm_hour, timeinfo->tm_min);
					set_time.tv_sec = (long)mktime(timeinfo);
					set_time.tv_usec = 0;
					ESP_LOGD(TAG, "Set time %ld ", set_time.tv_sec);
					settimeofday(&set_time, NULL);
					// TODO save params
					vSaveRecord();
					water_back_auto_report();

					xTimerStop(menu_exit_timer, 100);
					current_menu = app_menu_get_handler(E_APP_MENU_ID_MAIN);
					ESP_ERROR_CHECK(current_menu != NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
				}
					break;
				default:
					break;
			}
			break;
			
		default:
			ESP_LOGD(TAG, "APP Event: Unhandle event %d", sEvent->eType);
			break;
	}
	return 0;


}


/****************************************************************************
 *
 * NAME: vAppHandleAppEvent
 *
 * DESCRIPTION:
 * interprets the button press and calls the state machine.
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
 void vAppHandleAppEvent(APP_tsEvent *sEvent)
{

	if(current_menu != NULL){
		current_menu->event_handler(sEvent);
	}
	
	switch(sEvent->eType)
	{
		case APP_E_EVENT_BUTTON_DOWN:
			APP_vSetLed(APP_E_LEDS_LED_1, true);
			xTimerReset(back_led_timer, 100);
		break;
		
		case APP_E_EVENT_WB_FLOW_SENSOR:
		{
			int time_escape = sEvent->uEvent.sFlowSensor.time_escape;
			
			ESP_LOGD(TAG, "APP Event: Flow Sensor time_escape %d", time_escape);
			if(water_back.mode == E_WB_MODE_WATER_CONTROL || (water_back.mode == E_WB_MODE_TIMER && !water_back.time_up)){
				if(water_back.temperature < water_back.target_temperature - 1) {
					if(time_escape >= water_back.flow_time_min && time_escape <= water_back.flow_time_max){
						water_back_set_onoff(true, water_back.work_time);
						water_back_auto_report();
					}
				}
			}
			break;
		}
		
		case APP_E_EVENT_WB_TIME_IN_ZONE:
		{
			ESP_LOGD(TAG, "APP Event: Water back time in zone");
			
			break;
		}
		
		case APP_E_EVENT_WB_TIME_OUT_ZONE:
		{
			ESP_LOGD(TAG, "APP Event: Water back timer out zone");
			if(water_back.mode == E_WB_MODE_TIMER && !water_back.time_up){
				water_back_set_onoff(false, 0);
			}
			break;
		}

		case APP_E_EVENT_TICK:
		{
			ESP_LOGD(TAG, "APP Event: tick");
			
			break;
		}
		
		case APP_E_EVENT_RTC_MIN_UPDATE:
		{
			ESP_LOGD(TAG, "APP Event: minute update");
			
			break;
		}
		
		case APP_E_EVENT_WIFI_CONFIG:{
			ESP_LOGD(TAG, "APP Event: wifi config\n");
			// wait a while for http send response
			vTaskDelay(3000 / portTICK_RATE_MS);
	        memcpy(water_back.ssid, sEvent->uEvent.wifi_config.ssid, sizeof(water_back.ssid));
	        memcpy(water_back.password, sEvent->uEvent.wifi_config.password, sizeof(water_back.password));
			water_back.wifi_configed = true;
			vSaveRecord();
			esp_restart();
			break;
		}

	    default :
			//ESP_LOGD(TAG, "APP Event: Unhandle event %d", sEvent->eType);
	        break;

    }

}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/



/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
