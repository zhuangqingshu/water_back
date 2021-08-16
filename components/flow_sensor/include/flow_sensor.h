

#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H


#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

int flow_sensor_get_rate(void);

esp_err_t flow_sensor_disable();
esp_err_t flow_sensor_enable();
esp_err_t flow_sensor_deinit();
esp_err_t flow_sensor_init(uint32_t io_num);

#endif /* FLOW_SENSOR_H */


#ifdef __cplusplus
}
#endif

