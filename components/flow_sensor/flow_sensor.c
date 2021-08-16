
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "flow_sensor";

#define FLOW_SENSOR_CHECK(a, str, ret_val) \
    if (!(a)) { \
        ESP_LOGE(TAG,"%s(%d): %s", __FUNCTION__, __LINE__, str); \
        return (ret_val); \
    }

typedef struct {
    uint32_t io_num;
	uint8_t flow_rate;
} flow_sensor_obj_t;

flow_sensor_obj_t *flow_sensor_obj = NULL;

/*
get flow rate,and set rate 0.
*/
int flow_sensor_get_rate(void)
{
	if(flow_sensor_obj){
		int rate;
		rate = flow_sensor_obj->flow_rate;
		flow_sensor_obj->flow_rate = 0;
		return rate;
	}
	return 0;
}

/**
 * @brief ir rx state machine via gpio intr
 */
static void flow_sensor_intr_handler(void *arg)
{
    static uint32_t time_last = 0;

    uint32_t time_escape, time_current;
    struct timeval now;

    gettimeofday(&now, NULL);
    time_current = now.tv_sec * 1000 * 1000 + now.tv_usec;
    time_escape = time_current - time_last;
    time_last = time_current;

	if(flow_sensor_obj)
		flow_sensor_obj->flow_rate = 1000*1000 / time_escape;
	
    return;
}

esp_err_t flow_sensor_disable()
{
    FLOW_SENSOR_CHECK(flow_sensor_obj, "ir rx not been initialized yet", ESP_FAIL);
    gpio_isr_handler_remove(flow_sensor_obj->io_num);

    return ESP_OK;
}

esp_err_t flow_sensor_enable()
{
    FLOW_SENSOR_CHECK(flow_sensor_obj, "ir rx not been initialized yet", ESP_FAIL);
    gpio_isr_handler_add(flow_sensor_obj->io_num, flow_sensor_intr_handler, (void *) flow_sensor_obj->io_num);

    return ESP_OK;
}

static esp_err_t flow_sensor_gpio_init(uint32_t io_num)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = 1ULL << io_num;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);

    return ESP_OK;
}

esp_err_t flow_sensor_deinit()
{
	FLOW_SENSOR_CHECK(flow_sensor_obj, "ir rx has not been initialized yet.", ESP_FAIL);
    flow_sensor_disable();
    heap_caps_free(flow_sensor_obj);
    flow_sensor_obj = NULL;

    return ESP_OK;
}


esp_err_t flow_sensor_init(uint32_t io_num)
{
    FLOW_SENSOR_CHECK(NULL == flow_sensor_obj, "ir rx has been initialized", ESP_FAIL);

    flow_sensor_obj = heap_caps_malloc(sizeof(flow_sensor_obj_t), MALLOC_CAP_8BIT);
    FLOW_SENSOR_CHECK(flow_sensor_obj, "ir rx object malloc error", ESP_ERR_NO_MEM);
    flow_sensor_obj->io_num = io_num;

    // gpio configure for IR rx pin
    flow_sensor_gpio_init(flow_sensor_obj->io_num);
    flow_sensor_enable();

    return ESP_OK;
}


