
#include <time.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "driver_gn1629a.h"
#include "app_menu.h"
#include "app_water_back.h"
#include "app_events.h"
#include "app_common.h"

#define SET_BITS(value,bits)   ((value) |= (bits))
#define CLEAR_BITS(value,bits) ((value) &= ~(bits))
#define SET_DIGITAL(value,digital) {CLEAR_BITS(value,0x7F);SET_BITS(value,digital);}while(0)


static const char *TAG = "app_display";

/* ***************************************************** */
// 数码管位选数组定义
/* ***************************************************** */
static uint8_t  led_data[] = 
{               0x3F,  //"0"
                0x06,  //"1"
                0x5B,  //"2"
                0x4F,  //"3"
                0x66,  //"4"
                0x6D,  //"5"
                0x7D,  //"6"
                0x07,  //"7"
                0x7F,  //"8"
                0x6F,  //"9"
                0x77,  //"A"
                0x7C,  //"B"
                0x39,  //"C"
                0x5E,  //"D"
                0x79,  //"E"
                0x71,  //"F"
                0x76,  //"H"
                0x38,  //"L"
                0x37,  //"n"
                0x3E,  //"u"
                0x73,  //"P"
                0x5C,  //"o"
                0x40,  //"-"
                0x00,  //熄灭
                0x00  //自定义};
};

static uint8_t display_buffer[16] = {
	0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF
};

extern EventGroupHandle_t s_wifi_event_group;

void app_display_refresh(void)
{
	driver_gn1629a_write_display(display_buffer);
}

void app_display_text_target_temperature(bool display)
{
	if(display){
		SET_BITS(display_buffer[0], BIT7); // 设定温度
		SET_BITS(display_buffer[1], BIT7); // ℃
	}else{
		CLEAR_BITS(display_buffer[0], BIT7);
		CLEAR_BITS(display_buffer[1], BIT7);
	}
}

void app_display_text_work_delay(bool display)
{
	if(display){
		SET_BITS(display_buffer[2], BIT7); // 设定温度
	}else{
		CLEAR_BITS(display_buffer[2], BIT7);
	}
}

void app_display_text_onoff(bool onoff, bool display)
{
	CLEAR_BITS(display_buffer[14], BIT5); // 开
	CLEAR_BITS(display_buffer[14], BIT4); // 关

	if(display){
		if(onoff){
			SET_BITS(display_buffer[14], BIT5); // 开
		}else{
			SET_BITS(display_buffer[14], BIT4); // 关
		}
	}
}

void app_display_text_timer(uint8_t index, bool display)
{
	switch(index){
		case 0:
			if(display){
				SET_BITS(display_buffer[14], BIT6);
			}else{
				CLEAR_BITS(display_buffer[14], BIT6);
			}
			break;
		case 1:
			if(display){
				SET_BITS(display_buffer[14], BIT7);
			}else{
				CLEAR_BITS(display_buffer[14], BIT7);
			}

			break;
		case 2:
			if(display){
				SET_BITS(display_buffer[5], BIT7);
			}else{
				CLEAR_BITS(display_buffer[5], BIT7);
			}
			break;

		default:
			break;
	}
}

void app_display_text_flow(bool display)
{
	if(display){
		SET_BITS(display_buffer[7], BIT7); // 水流参数
		SET_BITS(display_buffer[6], BIT7); // --
	}else{
		CLEAR_BITS(display_buffer[7], BIT7);
		CLEAR_BITS(display_buffer[6], BIT7); // --
	}
}

void app_display_text_detect_temperature(bool display)
{
	if(display){
		SET_BITS(display_buffer[8], BIT7); // 检测温度
	}else{
		CLEAR_BITS(display_buffer[8], BIT7);
	}
}

void app_display_text_rtc_timer(bool display)
{
	if(display){
		SET_BITS(display_buffer[10], BIT7); // 实时时钟
	}else{
		CLEAR_BITS(display_buffer[10], BIT7);
	}
}

void app_display_text_water_control(bool display)
{
	if(display){
		SET_BITS(display_buffer[9], BIT7); // 水控
	}else{
		CLEAR_BITS(display_buffer[9], BIT7);
	}
}

void app_display_text_auto(bool display)
{
	if(display){
		SET_BITS(display_buffer[15], BIT7); // 自动
	}else{
		CLEAR_BITS(display_buffer[15], BIT7);
	}
}

void app_display_text_timing(bool display)
{
	if(display){
		SET_BITS(display_buffer[3], BIT7); // 定时
	}else{
		CLEAR_BITS(display_buffer[3], BIT7);
	}
}

void app_display_text_supercharge(bool display)
{
	if(display){
		SET_BITS(display_buffer[14], BIT3); // 增压
	}else{
		CLEAR_BITS(display_buffer[14], BIT3);
	}
}

void app_display_text_vacation (bool display)
{
	if(display){
		SET_BITS(display_buffer[11], BIT7); // 度假
	}else{
		CLEAR_BITS(display_buffer[11], BIT7);
	}
}

void app_display_text_protection  (bool display)
{
	if(display){
		SET_BITS(display_buffer[13], BIT7); // 度假
	}else{
		CLEAR_BITS(display_buffer[13], BIT7);
	}
}

/*
status 0 亮一半，1 亮另一半，2 全亮
*/
void app_display_logo_water_pump(uint8_t status, bool display)
{
	if(status == 2){
		SET_BITS(display_buffer[15], BIT1);
		SET_BITS(display_buffer[15], BIT2);
		SET_BITS(display_buffer[15], BIT3);
		SET_BITS(display_buffer[15], BIT4);
		SET_BITS(display_buffer[15], BIT5);
		SET_BITS(display_buffer[15], BIT6); 
	}else if( status == 1 ){
		SET_BITS(display_buffer[15], BIT1);
		CLEAR_BITS(display_buffer[15], BIT2);
		SET_BITS(display_buffer[15], BIT3);
		CLEAR_BITS(display_buffer[15], BIT4);
		SET_BITS(display_buffer[15], BIT5);
		CLEAR_BITS(display_buffer[15], BIT6); 

	}else if( status == 0 ){
		CLEAR_BITS(display_buffer[15], BIT1);
		SET_BITS(display_buffer[15], BIT2);
		CLEAR_BITS(display_buffer[15], BIT3);
		SET_BITS(display_buffer[15], BIT4);
		CLEAR_BITS(display_buffer[15], BIT5);
		SET_BITS(display_buffer[15], BIT6); 
	}
}

void app_display_logo_wifi(bool display)
{
	if(display){
		SET_BITS(display_buffer[14], BIT2);
	}else{
		CLEAR_BITS(display_buffer[14], BIT2);
	}
}

void app_display_logo_paloma(bool display)
{
	if(display){
		SET_BITS(display_buffer[14], BIT0 | BIT1);
	}else{
		CLEAR_BITS(display_buffer[14], BIT0 | BIT1);
	}
}

void app_display_target_temperature(uint8_t t, bool display)
{
	uint8_t t1;
	uint8_t t10;

	if(display){
		t1 = t % 10;
		t10 = t / 10;
		// 温度 10位
		SET_DIGITAL(display_buffer[0], led_data[t10]);
		// 温度 个位
		SET_DIGITAL(display_buffer[1], led_data[t1]);
	}else{
		// 温度 10位
		SET_DIGITAL(display_buffer[0], 0);
		// 温度 个位
		SET_DIGITAL(display_buffer[1], 0);
	}

}

void app_display_detect_temperature(uint8_t t, bool display)
{
	uint8_t t1;
	uint8_t t10;
	if(display){
		t1 = t % 10;
		t10 = t / 10;
		
		// 温度 10位
		SET_DIGITAL(display_buffer[8], led_data[t10]);
		// 温度 个位
		SET_DIGITAL(display_buffer[9], led_data[t1]);

	}else{
		// 温度 10位
		SET_DIGITAL(display_buffer[8], 0);
		// 温度 个位
		SET_DIGITAL(display_buffer[9], 0);

	}


}

void app_display_timer(uint8_t hour, uint8_t min, bool display)
{
	uint8_t t1;
	uint8_t t10;

	if(!display){
		SET_DIGITAL(display_buffer[2], 0);
		SET_DIGITAL(display_buffer[3], 0);
		CLEAR_BITS(display_buffer[4], BIT7);
		SET_DIGITAL(display_buffer[4], 0);
		SET_DIGITAL(display_buffer[5], 0);
		return;
	}
	// hour
	t1 = hour % 10;
	t10 = hour / 10;
	// 10位
	SET_DIGITAL(display_buffer[2], led_data[t10]);
	// 个位
	SET_DIGITAL(display_buffer[3], led_data[t1]);

	// :
	SET_BITS(display_buffer[4], BIT7);

	// mintue
	t1 = min % 10;
	t10 = min / 10;
	// 10位
	SET_DIGITAL(display_buffer[4], led_data[t10]);
	// 个位
	SET_DIGITAL(display_buffer[5], led_data[t1]);

}

void app_display_flow_min(uint8_t flow, bool display)
{
	if(!display){
		SET_DIGITAL(display_buffer[6], 0);
		return;
	}
	SET_DIGITAL(display_buffer[6], led_data[flow]);
}

void app_display_flow_max(uint8_t flow, bool display)
{
	if(!display){
		SET_DIGITAL(display_buffer[7], 0);
		return;
	}
	SET_DIGITAL(display_buffer[7], led_data[flow]);

}

void app_display_rtc_timer(uint8_t hour, uint8_t min, bool display)
{
	uint8_t t1;
	uint8_t t10;

	if(!display){
		SET_DIGITAL(display_buffer[10], 0);
		SET_DIGITAL(display_buffer[11], 0);
		SET_DIGITAL(display_buffer[12], 0);
		SET_DIGITAL(display_buffer[13], 0);
		return;
	}
	// hour
	t1 = hour % 10;
	t10 = hour / 10;
	// 10位
	SET_DIGITAL(display_buffer[10], led_data[t10]);
	// 个位
	SET_DIGITAL(display_buffer[11], led_data[t1]);

	// mintue
	t1 = min % 10;
	t10 = min / 10;
	// 10位
	SET_DIGITAL(display_buffer[12], led_data[t10]);
	// 个位
	SET_DIGITAL(display_buffer[13], led_data[t1]);

}

void app_display_rtc_tick( bool display)
{
	if(display)
		SET_BITS(display_buffer[12], BIT7);
	else
		CLEAR_BITS(display_buffer[12], BIT7);		
}


void app_display_digit( uint8_t index, bool display)
{
	if(index >= 0 && index < 14){
		if(!display)
			SET_DIGITAL(display_buffer[index], 0);	
	}
}

void menu_main_display(struct app_menu *menu)
{
	int i;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
	
	static uint8_t blink = 0;
	blink++;
	app_display_logo_paloma(true);
	app_display_text_target_temperature(true);
	app_display_text_detect_temperature(true);
	if(water_back.protection_status){
		app_display_target_temperature((uint8_t)water_back.target_temperature, blink % 2);
		app_display_detect_temperature((uint8_t)(water_back.temperature < 0 ? 0 : water_back.temperature), blink % 2);
	}else{
		app_display_target_temperature((uint8_t)water_back.target_temperature, true);
		app_display_detect_temperature((uint8_t)(water_back.temperature < 0 ? 0 : water_back.temperature), true);
	}

	
	app_display_text_work_delay(true);
	app_display_text_onoff(water_back.onoff, true);
	if(water_back.onoff){
		app_display_logo_water_pump(blink % 2, true);

		if(water_back.mode == E_WB_MODE_WATER_CONTROL || (E_WB_MODE_TIMER == water_back.mode && !water_back.time_up)){
			water_back.remaining_work_time = water_back.work_time_real - (esp_timer_get_time()/1000000 - water_back.start_work_time);
			app_display_timer(water_back.remaining_work_time/60, water_back.remaining_work_time%60, true);// min : sec
		}else{
			app_display_timer(water_back.work_time/60, water_back.work_time%60, true);// min : sec
		}

	}else{
		app_display_logo_water_pump(2, true);		
		app_display_timer(water_back.work_time/60, water_back.work_time%60, true);// min : sec
	}

	for(i=0; i < sizeof(water_back.timer) / sizeof(water_back.timer[0]); i++){
		if(water_back.timer[i].is_valid){
			app_display_text_timer(i, true);
		}else{
			app_display_text_timer(i, false);
		}
	}

	app_display_text_flow(true);
	if(water_back.flow_rate == 0){
		app_display_flow_min(water_back.flow_time_min, true);
		app_display_flow_max(water_back.flow_time_max, true);
	}else{
		
		int32_t rate = water_back.flow_rate / 10;
		if(water_back.flow_rate > 99)
			rate = 99;
		app_display_flow_min(rate / 10, true);
		app_display_flow_max(rate % 10, true);
	}

	// if wifi connect logo light up, unconnect loght off
	// if in smart config logo light blink
	switch(water_back.wifi_status){
		case E_WB_WIFI_UNCONNECT:
			app_display_logo_wifi(false);
		break;
		case E_WB_WIFI_CONFIG:
			app_display_logo_wifi(blink % 2);
		break;
		case E_WB_WIFI_CONNECT:
			app_display_logo_wifi(true);
		break;
		default:
		break;
	}

	app_display_text_rtc_timer(true);
	app_display_rtc_timer((uint8_t) timeinfo.tm_hour, (uint8_t) timeinfo.tm_min, true);
	app_display_rtc_tick(blink % 2);

	switch(water_back.mode){
	case E_WB_MODE_WATER_CONTROL:
		app_display_text_water_control(true);
		app_display_text_timing(false);
		app_display_text_auto(false);
		break;
	case E_WB_MODE_TIMER:
		app_display_text_water_control(false);
		app_display_text_timing(true);
		app_display_text_auto(false);

		break;
	case E_WB_MODE_AUTO:
		app_display_text_water_control(false);
		app_display_text_timing(false);
		app_display_text_auto(true);
		break;

	default:
		break;
	}
	app_display_text_supercharge(water_back.supercharge);
	app_display_text_vacation (water_back.vacation);
	app_display_text_protection(water_back.protection);

}

void menu_setting_display(struct app_menu *menu)
{
	int i;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
	
	static uint8_t blink = 0;
	blink++;

	app_display_logo_paloma(true);
	app_display_text_target_temperature(true);
	app_display_text_detect_temperature(true);	
	app_display_detect_temperature((uint8_t)(water_back.temperature < 0 ? 0 : water_back.temperature), true);
	app_display_target_temperature((uint8_t)water_back.target_temperature, true);

	app_display_text_work_delay(true);
	app_display_text_onoff(water_back.onoff, true);
	app_display_timer(water_back.work_time/60, water_back.work_time%60, true);// min : sec
	if(water_back.onoff){
		app_display_logo_water_pump(blink % 2, true);
	}else{
		app_display_logo_water_pump(2, true);		
	}

	for(i=0; i < sizeof(water_back.timer) / sizeof(water_back.timer[0]); i++){
		if(water_back.timer[i].is_valid){
			app_display_text_timer(i, true);
		}else{
			app_display_text_timer(i, false);
		}
	}

	app_display_text_flow(true);
	app_display_flow_min(water_back.flow_time_min, true);
	app_display_flow_max(water_back.flow_time_max, true);


	// if wifi connect logo light up, unconnect loght off
	// if in smart config logo light blink
	switch(water_back.wifi_status){
		case E_WB_WIFI_UNCONNECT:
			app_display_logo_wifi(false);
		break;
		case E_WB_WIFI_CONFIG:
			app_display_logo_wifi(blink % 2);
		break;
		case E_WB_WIFI_CONNECT:
			app_display_logo_wifi(true);
		break;
		default:
		break;
	}

	app_display_text_rtc_timer(true);
	app_display_rtc_timer((uint8_t) timeinfo.tm_hour, (uint8_t) timeinfo.tm_min, true);
	app_display_rtc_tick(blink % 2);

	switch(water_back.mode){
	case E_WB_MODE_WATER_CONTROL:
		app_display_text_water_control(true);
		app_display_text_timing(false);
		app_display_text_auto(false);
		break;
	case E_WB_MODE_TIMER:
		app_display_text_water_control(false);
		app_display_text_timing(true);
		app_display_text_auto(false);

		break;
	case E_WB_MODE_AUTO:
		app_display_text_water_control(false);
		app_display_text_timing(false);
		app_display_text_auto(true);
		break;

	default:
		break;
	}
	app_display_text_supercharge(water_back.supercharge);
	app_display_text_vacation (water_back.vacation);
	app_display_text_protection(water_back.protection);

	// blink control
	app_display_digit(menu->curros, blink % 2);

}

void menu_timer_setting_display(struct app_menu *menu)
{
	ESP_ERROR_CHECK(menu!=NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
	ESP_ERROR_CHECK(menu->context!=NULL ? ESP_OK : ESP_ERR_INVALID_ARG);
	int i;
    struct tm *timeinfo = (struct tm *)menu->context;

	static uint8_t blink = 0;
	blink++;

	app_display_logo_paloma(true);
	app_display_text_target_temperature(true);
	app_display_text_detect_temperature(true);	
	app_display_target_temperature((uint8_t)water_back.target_temperature, true);
	app_display_detect_temperature((uint8_t)(water_back.temperature < 0 ? 0 : water_back.temperature), true);

	app_display_text_work_delay(false);
	app_display_text_onoff(water_back.onoff, true);
	if(water_back.onoff){
		app_display_logo_water_pump(blink % 2, true);

	}else{
		app_display_logo_water_pump(2, true);		
	}

	
	for(i=0; i < sizeof(water_back.timer) / sizeof(water_back.timer[0]); i++){
		app_display_text_timer(i, false);
	}
	if(current_menu->curros >= 4){
		int hour=23,minute=23;
		uint8_t timer_index = current_menu->curros / 4 - 1; // 0 1 2 3 4 5
		if(timer_index % 2 == 0){
			timer_index = timer_index / 2;
			APP_ERROR_CHECK(2==sscanf(water_back.timer[timer_index].time_start, "%d:%d", &hour,&minute), "sscanf time", ESP_ERR_INVALID_ARG);
		}else{
			timer_index = timer_index / 2;
			APP_ERROR_CHECK(2==sscanf(water_back.timer[timer_index].time_end, "%d:%d", &hour,&minute), "sscanf time", ESP_ERR_INVALID_ARG);
		}
		app_display_text_timer(timer_index, true);
		app_display_timer(hour, minute, true);// hour : mintue
	}
	

	app_display_text_flow(false);
	app_display_flow_min(0, true);
	if(current_menu->curros >= 4){
		if((current_menu->curros / 4) % 2 == 1){
			app_display_flow_max(0x0A, true);  // water back start timer
		}else{
			app_display_flow_max(0x0F, true); // water back end timer
		}
	}
	

	// if wifi connect logo light up, unconnect loght off
	// if in smart config logo light blink
	switch(water_back.wifi_status){
		case E_WB_WIFI_UNCONNECT:
			app_display_logo_wifi(false);
		break;
		case E_WB_WIFI_CONFIG:
			app_display_logo_wifi(blink % 2);
		break;
		case E_WB_WIFI_CONNECT:
			app_display_logo_wifi(true);
		break;
		default:
		break;
	}

	app_display_text_rtc_timer(true);
	app_display_rtc_timer((uint8_t) timeinfo->tm_hour, (uint8_t) timeinfo->tm_min, true);
	app_display_rtc_tick(true);

	switch(water_back.mode){
	case E_WB_MODE_WATER_CONTROL:
		app_display_text_water_control(true);
		app_display_text_timing(false);
		app_display_text_auto(false);
		break;
	case E_WB_MODE_TIMER:
		app_display_text_water_control(false);
		app_display_text_timing(true);
		app_display_text_auto(false);

		break;
	case E_WB_MODE_AUTO:
		app_display_text_water_control(false);
		app_display_text_timing(false);
		app_display_text_auto(true);
		break;

	default:
		break;
	}
	app_display_text_supercharge(water_back.supercharge);
	app_display_text_vacation (water_back.vacation);
	app_display_text_protection(water_back.protection);

	// blink control
	if(current_menu->curros >= 0 && current_menu->curros < 4){
		app_display_digit(current_menu->curros + 10, blink % 2);
	}else if(current_menu->curros >= 4){
		// current_menu->curros
		// 4 8  12 16 20 24    app_display_digit(2, blink % 2);
		// 5 9  13 17 21 25       ...
		// 6 10 14 18 22 26       ...
		// 7 11 15 19 23 27       ...
		if(current_menu->curros % 4 == 0){
			app_display_digit(2, blink % 2);
		}else if(current_menu->curros % 4 == 1){
			app_display_digit(3, blink % 2);
		}else if(current_menu->curros % 4 == 2){
			app_display_digit(4, blink % 2);
		}else if(current_menu->curros % 4 == 3){
			app_display_digit(5, blink % 2);
		}
	}
}

void app_display_test()
{
	display_buffer[0] = 0x3F;
	display_buffer[1] = led_data[1];
	driver_gn1629a_write_display(display_buffer);
}

