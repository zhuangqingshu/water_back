/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_mqtt.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "dev_token.h"
#include "tm_mqtt.h"
#include "tm_api.h"
#include "config.h"

#include <stdlib.h>
#include "mqtt_client.h"
#include "esp_log.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_NONE
#define THING_MODEL_SERVER_ADDR "218.201.45.7"
#define THING_MODEL_SERVER_PORT 1883
#else
#define THING_MODEL_SERVER_ADDR_TLS "183.230.102.116"
#define THING_MODEL_SERVER_PORT_TLS 8883 
#endif

#define THING_MODEL_SUBED_TOPIC "$sys/%s/%s/thing/#"

#ifdef FEATURE_TM_OTA_ENABLED
#define THING_MODEL_SUBED_OTA_TOPIC "$sys/%s/%s/ota/#"
#endif

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct tm_mqtt_obj_t
{
	bool is_connect;
    esp_mqtt_client_handle_t client;
    esp_mqtt_client_config_t mqtt_param;
    char *subed_topic;
    int32_t (*recv_cb)(const char * /** data_name*/, uint8_t * /** data*/, uint32_t /** data_len*/);
#ifdef FEATURE_TM_OTA_ENABLED
    char *subed_ota_topic;
#endif
};
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

static const char *TAG = "tm_mqtt";

struct tm_mqtt_obj_t *tm_obj = NULL;

#if FEATURE_TM_MQTT_TLS_TYPE == FEATURE_TM_MQTT_TLS_WOLFSSL
static const char tm_cert[] = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV\r\n"
    "BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X\r\n"
    "DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM\r\n"
    "BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3\r\n"
    "DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx\r\n"
    "5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR\r\n"
    "r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS\r\n"
    "UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl\r\n"
    "U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP\r\n"
    "ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G\r\n"
    "A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia\r\n"
    "bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL\r\n"
    "aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7\r\n"
    "q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb\r\n"
    "41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a\r\n"
    "3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR\r\n"
    "NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa\r\n"
    "WzdhzTYwVkxBaU+xf/2w\r\n"
    "-----END CERTIFICATE-----"
 };
#endif

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static void tm_mqtt_message_arrived(void *arg, const char *topic, uint8_t *payload, uint32_t payload_len)
{
    tm_obj->recv_cb(topic, payload, payload_len);
}

static char *tm_topic_construct(const char *format, const char *pid, const char *dev_name)
{
    uint32_t topic_len = 0;
    char *topic = NULL;

    topic_len = strlen(format) + strlen(pid) + strlen(dev_name) - 3;

    if (NULL != (topic = malloc(topic_len)))
    {
        memset(topic, 0, topic_len);
        sprintf(topic, format, pid, dev_name);
    }

    return topic;
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			tm_obj->is_connect = true;
            msg_id = esp_mqtt_client_subscribe(client, tm_obj->subed_topic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			tm_obj->is_connect = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGD("DATA=%.*s\r\n", event->data_len, event->data);

			if(tm_obj->recv_cb)
				tm_obj->recv_cb(event->topic, (uint8_t *)event->data, event->data_len);
			
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

int32_t tm_mqtt_init(int32_t (*recv_cb)(const char * /** data_name*/, uint8_t * /** data*/,
                                        uint32_t /** data_len*/))
{
    if(NULL == (tm_obj = malloc(sizeof(struct tm_mqtt_obj_t))))
    {
        goto exit1;
    }
    memset(tm_obj, 0, sizeof(struct tm_mqtt_obj_t));

    tm_obj->recv_cb = recv_cb;

    return 0;

exit1:
    return -1;
}

int32_t tm_mqtt_login(const char *product_id, const char *dev_name, const char *dev_token, uint32_t life_time,
                      uint32_t timeout_ms)
{
	if(tm_obj == NULL){
	  ESP_LOGE(TAG, "tm_obj null");
	  return -1;
	}

	tm_obj->mqtt_param.uri = CONFIG_BROKER_URL;
    tm_obj->mqtt_param.client_id = dev_name;
    tm_obj->mqtt_param.username = product_id;
    tm_obj->mqtt_param.password = dev_token;
    tm_obj->mqtt_param.keepalive = life_time;

    tm_obj->subed_topic = tm_topic_construct(THING_MODEL_SUBED_TOPIC, product_id, dev_name);
#ifdef FEATURE_TM_OTA_ENABLED
    tm_obj->subed_ota_topic = tm_topic_construct(THING_MODEL_SUBED_OTA_TOPIC, product_id, dev_name);
#endif
	
    tm_obj->client = esp_mqtt_client_init(&tm_obj->mqtt_param);
    esp_mqtt_client_register_event(tm_obj->client, ESP_EVENT_ANY_ID, mqtt_event_handler, tm_obj->client);
    esp_mqtt_client_start(tm_obj->client);

    return 0;

}

int32_t tm_mqtt_logout(uint32_t timeout_ms)
{
	if(tm_obj == NULL){
	  ESP_LOGE(TAG, "tm_obj null");
	  return -1;
	}

    if (tm_obj->client)
    {
        esp_mqtt_client_disconnect(tm_obj->client);
		esp_mqtt_client_destroy(tm_obj->client);
    }
    if(tm_obj->subed_topic)
    {
        free(tm_obj->subed_topic);
    }

    if(tm_obj)
    {
        free(tm_obj);
        tm_obj = NULL;
    }

    return 0;
}

int32_t tm_mqtt_send_packet(const char *topic, uint8_t *payload, uint32_t payload_len, uint32_t timeout_ms)
{	
	if(tm_obj == NULL){
	  ESP_LOGE(TAG, "tm_obj null");
	  return -1;
	}

	// MQTT_STATE_CONNECTED == 3
	if (!(tm_obj->client && tm_obj->is_connect)){
		ESP_LOGE(TAG, "mqtt unconnect");
		return -1;
	}
	int ret = esp_mqtt_client_publish(tm_obj->client, topic, (const char *)payload, payload_len, 0, 0);
	if(0 > ret){
		ESP_LOGE(TAG, "esp_mqtt_client_publish fail %d", ret);
		return -1;
	}
	return 0;
}

int32_t tm_mqtt_step(uint32_t timeout_ms)
{
    return 0;
}
