/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_mqtt.h
 * @brief
 */

#ifndef __TM_MQTT_H__
#define __TM_MQTT_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

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

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
int32_t tm_mqtt_init(int32_t (*recv_cb)(const char * /** data_name*/, uint8_t * /** data*/,
                                        uint32_t /** data_len*/));
int32_t tm_mqtt_login(const char *product_id, const char *dev_name, const char *dev_token,
                      uint32_t life_time, uint32_t timeout_ms);
int32_t tm_mqtt_logout(uint32_t timeout_ms);
int32_t tm_mqtt_step(uint32_t timeout_ms);
int32_t tm_mqtt_send_packet(const char *topic, uint8_t *payload, uint32_t payload_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif