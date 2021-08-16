

#ifndef WATER_PUMP_H
#define WATER_PUMP_H


#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void water_pump_init(uint32_t io_num);
void water_pump_set_onoff(bool bOn);
bool water_pump_get_onoff();


#endif /* WATER_PUMP_H */


#ifdef __cplusplus
}
#endif


