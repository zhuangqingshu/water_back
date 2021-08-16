
#include <string.h>
#include <time.h>

#include "esp_system.h"
#include "esp_log.h"
#include "app_events.h"
#include "board.h"
#include "app_menu.h"
#include "app_common.h"


static app_menu_t *menu_head = NULL;
app_menu_t *current_menu = NULL;

static const char *TAG = "app_menu";

app_menu_t * app_menu_get_handler(app_menu_id_t id)
{
	app_menu_t *current = menu_head;
	
	while(current != NULL){
		ESP_LOGD(TAG, "Menu %d %s", current->id, current->name);
		if(current->id == id){
			return current;
		}
		current = current->next;
	}

	return NULL;

}

esp_err_t app_menu_default_init(app_menu_t *m, char *name, int id)
{
	APP_ERROR_CHECK(m!=NULL, "point null", ESP_ERR_INVALID_ARG);
	m->id = id;
	strncpy(m->name, name, sizeof(m->name));
	m->curros = -1;
	m->display_handler = NULL;
	m->event_handler = NULL;
	m->next = NULL;
	m->context = NULL;
	return ESP_OK;
}

esp_err_t app_menu_add(app_menu_t *m)
{
	APP_ERROR_CHECK(m!=NULL, "point null", ESP_ERR_INVALID_ARG);
	app_menu_t *current = menu_head;

	m->next = current;
	menu_head = m;
	return ESP_OK;
}

extern void menu_main_display(struct app_menu *menu);
extern int menu_main_event_handle(APP_tsEvent *sEvent);
extern void menu_setting_display(struct app_menu *menu);
extern int menu_setting_event_handle(APP_tsEvent *sEvent);
extern void menu_timer_setting_display(struct app_menu *menu);
extern int menu_timer_setting_event_handle(APP_tsEvent *sEvent);

esp_err_t app_menu_init()
{
	app_menu_t *menu;
	// main menu
	menu = (app_menu_t *)calloc(sizeof(app_menu_t), 1);
	APP_ERROR_CHECK(menu!=NULL, "malloc error", ESP_FAIL);
	app_menu_default_init(menu, "main", E_APP_MENU_ID_MAIN);
	menu->display_handler = menu_main_display;
	menu->event_handler = menu_main_event_handle;
	app_menu_add(menu);
	
	menu = (app_menu_t *)calloc(sizeof(app_menu_t), 1);
	APP_ERROR_CHECK(menu!=NULL, "malloc error", ESP_FAIL);
	app_menu_default_init(menu, "set", E_APP_MENU_ID_SETTING);
	menu->display_handler = menu_setting_display;
	menu->event_handler = menu_setting_event_handle;
	app_menu_add(menu);

	menu = (app_menu_t *)calloc(sizeof(app_menu_t), 1);
	APP_ERROR_CHECK(menu!=NULL, "malloc error", ESP_FAIL);
	app_menu_default_init(menu, "timer", E_APP_MENU_ID_TIMER_SETTING);
	menu->context = calloc(sizeof(struct tm), 1);
	menu->display_handler = menu_timer_setting_display;
	menu->event_handler = menu_timer_setting_event_handle;
	app_menu_add(menu);

	return ESP_OK;
}


