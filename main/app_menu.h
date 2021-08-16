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
#ifndef __APP_MENU_H_
#define __APP_MENU_H_

/* Includes ------------------------------------------------------------------*/
#include "app_events.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public typedef ------------------------------------------------------------*/
typedef enum {
	E_APP_MENU_ID_MAIN = 0,
	E_APP_MENU_ID_SETTING,
	E_APP_MENU_ID_TIMER_SETTING
}app_menu_id_t;

typedef int (*cb_menu_event_handler) (APP_tsEvent *sEvent);
typedef int (*cb_menu_display) (struct app_menu *menu);

typedef struct app_menu {
	int  id;
	char name[8];
	int curros;
	void *context;
	cb_menu_event_handler event_handler;
	cb_menu_display display_handler;
	struct app_menu *next;
}app_menu_t;


/* Public define -------------------------------------------------------------*/
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern app_menu_t *current_menu;

/* Public function prototypes ------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

esp_err_t app_menu_init();
esp_err_t app_menu_add(app_menu_t *m);
esp_err_t app_menu_default_init(app_menu_t *m, char *name, int id);
app_menu_t * app_menu_get_handler(app_menu_id_t id);


#ifdef __cplusplus
}
#endif
#endif /* __APP_CURTAIN_SLIDE_H_ */
/********************************* END OF FILE *********************************/


