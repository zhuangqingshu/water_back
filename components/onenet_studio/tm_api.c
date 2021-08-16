/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_api.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "dev_token.h"
#include "tm_data.h"
#include "tm_onejson.h"
#include "tm_api.h"
#include "config.h"
#include "err_def.h"
#include "plat_time.h"
#include "esp_log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
#include "tm_mqtt.h"
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
#include "tm_coap.h"
#endif

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define TM_TOPIC_PREFIX "$sys/%s/%s/thing"

#define TM_TOPIC_PROP_POST            "/property/post"
#define TM_TOPIC_PROP_SET             "/property/set"
#define TM_TOPIC_PROP_GET             "/property/get"
#define TM_TOPIC_EVENT_POST           "/event/post"
#define TM_TOPIC_DESIRED_PROPS_GET    "/property/desired/get"
#define TM_TOPIC_DESIRED_PROPS_DELETE "/property/desired/delete"
#define TM_TOPIC_SERVICE_INVOKE       "/service/%s/invoke"
#define TM_TOPIC_PACK_DATA_POST       "/pack/post"
#define TM_TOPIC_HISTORY_DATA_POST    "/history/post"

#define TM_TOPIC_PROP_POST_REPLY            "/property/post/reply"
#define TM_TOPIC_PROP_SET_REPLY             "/property/set_reply"
#define TM_TOPIC_PROP_GET_REPLY             "/property/get_reply"
#define TM_TOPIC_EVENT_POST_REPLY           "/event/post/reply"
#define TM_TOPIC_DESIRED_PROPS_GET_REPLY    "/property/desired/get/reply"
#define TM_TOPIC_DESIRED_PROPS_DELETE_REPLY "/property/desired/delete/reply"
#define TM_TOPIC_SERVICE_INVOKE_REPLY       "/service/%s/invoke_reply"
#define TM_TOPIC_PACK_DATA_POST_REPLY       "/pack/post/reply"
#define TM_TOPIC_HISTORY_DATA_POST_REPLY    "/history/post/reply"

#ifdef FEATURE_TM_OTA_ENABLED
#define TM_TOPIC_OTA_PREFIX "$sys/%s/%s/ota"
#define TM_TOPIC_OTA_REPLY "/inform_reply"
#endif

static const char *TAG = "tm_api";

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
#define REPLY_STATUS_NONE     0
#define REPLY_STATUS_WAIT     1
#define REPLY_STATUS_RECEIVED 2
struct tm_reply_info_t
{
    char reply_id[16];
    int32_t reply_code;
    void *reply_data;
    uint8_t resp_flag;
    uint16_t reply_status;
    uint8_t reply_as_raw;
};

struct tm_obj_t
{
    struct tm_downlink_tbl_t downlink_tbl;
    char *topic_prefix;
    int32_t post_id;
    struct tm_reply_info_t reply_info;
#ifdef FEATURE_TM_GATEWAY_ENABLED
    tm_subdev_cb subdev_callback;
#endif
#ifdef FEATURE_TM_OTA_ENABLED
    char *ota_topic_prefix;
#endif
#ifdef FEATURE_TM_FMS
    char *product_id;
    char *dev_name;
    char *dev_token;
#endif
};

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
struct tm_obj_t g_tm_obj;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
static char *construct_topic(const char *prefix, const char *suffix)
{
    uint32_t topic_len = strlen(prefix) + strlen(suffix) + 1;
    char *topic_buf = malloc(topic_len);

    if(topic_buf)
    {
        memset(topic_buf, 0, topic_len);
        strcat(topic_buf, prefix);
        strcat(topic_buf, suffix);
    }

    return topic_buf;
}

static int32_t get_post_id(void)
{
    if(0x7FFFFFFF == ++(g_tm_obj.post_id))
    {
        g_tm_obj.post_id = 1;
    }

    return g_tm_obj.post_id;
}

int32_t tm_send_response(const char *name, char *msg_id, int32_t msg_code, uint8_t as_raw, void *resp_data,
                         uint32_t resp_data_len, uint32_t timeout_ms)
{
    char *topic = NULL;
    uint8_t *payload = NULL;
    uint32_t payload_len = 0;

    if(NULL == (payload = malloc(FEATURE_TM_PAYLOAD_BUF_LEN)))
    {
        return ERR_NO_MEM;
    }

    memset(payload, 0, FEATURE_TM_PAYLOAD_BUF_LEN);
    payload_len = tm_onejson_pack_reply(payload, msg_id, msg_code, resp_data, as_raw);
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
    topic = construct_topic(g_tm_obj.topic_prefix, name);
	//ESP_LOGD(TAG,"mqtt resp:%s", payload);
    tm_mqtt_send_packet(topic, payload, payload_len, FEATURE_TM_REPLY_TIMEOUT);
    free(topic);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    tm_coap_send_packet(NULL, payload, payload_len, FEATURE_TM_REPLY_TIMEOUT);
#endif

    free(payload);

    return ERR_OK;
}

#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
static int32_t wait_post_reply(int32_t post_id, handle_t cd_hdl)
{
    char temp_id[16] = {0};
    int32_t ret = ERR_TIMEOUT;

    sprintf(temp_id, "%d", post_id);

    do
    {
        ret = tm_mqtt_step(countdown_left(cd_hdl));

        if(0 > ret)
        {
            ESP_LOGE(TAG, "wait reply error\n");
            break;
        }
        else
        {
            if(REPLY_STATUS_RECEIVED == g_tm_obj.reply_info.reply_status)
            {
                if(0 == strcmp(temp_id, g_tm_obj.reply_info.reply_id))
                {
                    g_tm_obj.reply_info.reply_status = REPLY_STATUS_NONE;
                    ret = ERR_OK;
                    break;
                }
                else
                {
                    g_tm_obj.reply_info.reply_status = REPLY_STATUS_WAIT;
                }
            }
        }
    } while(0 == countdown_is_expired(cd_hdl));

    return ret;
}
#endif

int32_t tm_send_request(const char *name, uint8_t as_raw, void *data, uint32_t data_len, void **reply_data,
                        uint32_t *reply_data_len, uint32_t timeout_ms)
{
    char *topic = NULL;
    uint8_t *payload = NULL;
    uint32_t payload_len = 0;
    int32_t post_id = get_post_id();
    handle_t cd_hdl = 0;
    int32_t ret = ERR_OTHERS;

	if(g_tm_obj.topic_prefix == NULL){
	  ESP_LOGE(TAG, "g_tm_obj.topic_prefix is null");
	  return ERR_INVALID_PARAM;
	}

    cd_hdl = countdown_start(timeout_ms);

    if(NULL == (payload = malloc(FEATURE_TM_PAYLOAD_BUF_LEN)))
    {
        return ERR_NO_MEM;
    }

    memset(payload, 0, FEATURE_TM_PAYLOAD_BUF_LEN);
    payload_len = tm_onejson_pack_request(payload, post_id, data, as_raw);
    g_tm_obj.reply_info.reply_as_raw = as_raw;

    topic = construct_topic(g_tm_obj.topic_prefix, name);
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
	//ESP_LOGD(TAG,"mqtt send:%s", payload);
    ret = tm_mqtt_send_packet(topic, payload, payload_len, countdown_left(cd_hdl));
	if(ret < 0)
	{
		ESP_LOGE(TAG,"tm_mqtt_send_packet failed %d.", ret);
	}

/*
    if(ERR_OK == ret)
    {
        g_tm_obj.reply_info.reply_status = REPLY_STATUS_WAIT;
        if(0 == wait_post_reply(post_id, cd_hdl))
        {
            ESP_LOGD(TAG, "reply err code: %d\n", g_tm_obj.reply_info.reply_code);
            if(200 == g_tm_obj.reply_info.reply_code)
            {
                ESP_LOGD(TAG,"post data ok\n");
                if(NULL != reply_data)
                {
                    *reply_data = g_tm_obj.reply_info.reply_data;
                }
                ret = ERR_OK;
            }
            g_tm_obj.reply_info.reply_data = NULL;
        }
        g_tm_obj.reply_info.reply_code = 0;
        g_tm_obj.reply_info.reply_as_raw = 0;
    }
*/
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    ret = tm_coap_send_packet(topic, payload, payload_len, countdown_left(cd_hdl));
    if(ERR_OK == ret)
    {
        ESP_LOGD(TAG,"tm_send_request ok.\n");
    }
    else
    {
        ESP_LOGD(TAG,"tm_send_request failed.\n");
    }
#endif

    free(topic);
    free(payload);
    countdown_stop(cd_hdl);

    return ret;
}

static int32_t tm_prop_set_handle(const char *name, void *res)
{
    uint16_t i = 0;
    int32_t ret = 0;

    for(i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++)
    {
        if(0 == strcmp(name, g_tm_obj.downlink_tbl.prop_tbl[i].name))
        {
            ret = g_tm_obj.downlink_tbl.prop_tbl[i].tm_prop_wr_cb(res);
            break;
        }
    }

    return ret;
}

static void tm_prop_set(uint8_t *payload, uint32_t payload_len)
{
    void *props_data = NULL;
    uint8_t *reply_payload = NULL;
    uint32_t reply_payload_len = 0;
    char *topic = NULL;
    char id[16] = {0};

    if(NULL == (reply_payload = malloc(128)))
    {
        ESP_LOGE(TAG,"prop set error, malloc failed.\n");
        return;
    }
    memset(reply_payload, 0, 128);

    props_data = tm_onejson_parse_request(payload, payload_len, id, 0);

    if(NULL != props_data)
    {
        if(0 == tm_data_list_each(props_data, tm_prop_set_handle))
        {
            tm_send_response(TM_TOPIC_PROP_SET_REPLY, id, 200, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }
        else
        {
            tm_send_response(TM_TOPIC_PROP_SET_REPLY, id, 100, 0, NULL, 0, FEATURE_TM_REPLY_TIMEOUT);
        }

        tm_data_delete(props_data);
    }

    free(reply_payload);
}

static void tm_prop_get(uint8_t *payload, uint32_t payload_len)
{
    void *props_data = NULL;
//    uint8_t *reply_payload = NULL;
//    uint32_t reply_payload_len = 0;
    char *topic = NULL;
    uint32_t i = 0;
    char id[16] = {0};
/*
    if(NULL == (reply_payload = malloc(128)))
    {
        ESP_LOGE(TAG,"prop set error, malloc failed.\n");
        return;
    }
    memset(reply_payload, 0, 128);
*/
    props_data = tm_onejson_parse_request(payload, payload_len, id, 0);

    if(NULL != props_data)
    {
        uint32_t i, j = 0;
        uint32_t prop_cnt = tm_data_array_size(props_data);
        char *prop_name = NULL;
        void *reply_data = tm_data_create();

        for(i = 0; i < prop_cnt; i++)
        {
            tm_data_get_string(tm_data_array_get_element(props_data, i), &prop_name);
            for(j = 0; j < g_tm_obj.downlink_tbl.prop_tbl_size; j++)
            {
                if(0 == strcmp(prop_name, g_tm_obj.downlink_tbl.prop_tbl[j].name))
                {
                    g_tm_obj.downlink_tbl.prop_tbl[j].tm_prop_rd_cb(reply_data);
                }
            }
        }

        tm_send_response(TM_TOPIC_PROP_GET_REPLY, id, 200, 0, reply_data, 0, FEATURE_TM_REPLY_TIMEOUT);

        tm_data_delete(props_data);
    }

//    free(reply_payload);
}

static void tm_post_reply(uint8_t *payload, uint32_t payload_len)
{
    if(REPLY_STATUS_WAIT == g_tm_obj.reply_info.reply_status)
    {
        g_tm_obj.reply_info.reply_data =
            tm_onejson_parse_reply(payload, payload_len, g_tm_obj.reply_info.reply_id, &g_tm_obj.reply_info.reply_code,
                                   g_tm_obj.reply_info.reply_as_raw);
        g_tm_obj.reply_info.reply_status = REPLY_STATUS_RECEIVED;
    }
}

static int32_t tm_prop_set_desired_handle(const char *name, void *res)
{
    uint16_t i = 0;
    int32_t ret = 0;

    for(i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++)
    {
        if(0 == strcmp(name, g_tm_obj.downlink_tbl.prop_tbl[i].name))
        {
            ret = g_tm_obj.downlink_tbl.prop_tbl[i].tm_prop_wr_cb(res);
            break;
        }
    }

    return ret;
}

static void tm_service_invoke(char *svc_id, uint8_t *payload, uint32_t payload_len)
{
    void *svc_data = NULL;
    char *topic = NULL;
    uint8_t *reply_payload = NULL;
    uint32_t reply_payload_len = 0;
    uint32_t i = 0;
    char id[16] = {0};

    svc_data = tm_onejson_parse_request(payload, payload_len, id, 0);

    if(NULL != svc_data)
    {
        void *reply_data = tm_data_struct_create();

        for(i = 0; i < g_tm_obj.downlink_tbl.svc_tbl_size; i++)
        {
            if(0 == strcmp(svc_id, g_tm_obj.downlink_tbl.svc_tbl[i].name))
            {
                g_tm_obj.downlink_tbl.svc_tbl[i].tm_svc_cb(svc_data, reply_data);

                tm_data_delete(svc_data);
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
                topic = malloc(strlen(TM_TOPIC_SERVICE_INVOKE_REPLY) + strlen(svc_id));
                sprintf(topic, TM_TOPIC_SERVICE_INVOKE_REPLY, svc_id);
#endif
                tm_send_response(topic, id, 200, 0, reply_data, 0, FEATURE_TM_REPLY_TIMEOUT);
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
                free(topic);
#endif
            }
        }
    }
}

#ifdef FEATURE_TM_OTA_ENABLED
static void tm_ota(uint8_t *payload, uint32_t payload_len)
{
    void *ota_data = NULL;
    char id[16] = { 0 };
    //for response
    char *topic = NULL;
    uint8_t *response_payload = NULL;
    uint32_t response_payload_len = 0;

    ota_data = tm_onejson_parse_request(payload, payload_len, id, 1);

    if(NULL != ota_data)
    {
        free(ota_data);
        if(NULL == (response_payload = malloc(FEATURE_TM_PAYLOAD_BUF_LEN)))
        {
            return ERR_NO_MEM;
        }

        memset(response_payload, 0, FEATURE_TM_PAYLOAD_BUF_LEN);
        response_payload_len = tm_onejson_pack_reply(response_payload, id, 200, NULL, 0);
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
        topic = construct_topic(g_tm_obj.ota_topic_prefix, TM_TOPIC_OTA_REPLY);
        tm_mqtt_send_packet(topic, response_payload, response_payload_len, FEATURE_TM_REPLY_TIMEOUT);
        free(topic);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
        tm_coap_send_packet(NULL, response_payload, response_payload_len, FEATURE_TM_REPLY_TIMEOUT);
#endif
        free(response_payload);
        g_tm_obj.downlink_tbl.ota_cb();
        return ERR_OK;
    }
}
#endif

static int32_t tm_data_parse(const char *res_name, uint8_t *payload, uint32_t payload_len)
{
    char *action = (char*)res_name + strlen(g_tm_obj.topic_prefix);
#ifdef FEATURE_TM_OTA_ENABLED
    char *ota_action = (char *)res_name + strlen(g_tm_obj.ota_topic_prefix);
#endif
    if(0 == strncmp(action, TM_TOPIC_PROP_SET, strlen(TM_TOPIC_PROP_SET)))
    {
        tm_prop_set(payload, payload_len);
    }
    else if(0 == strncmp(action, TM_TOPIC_PROP_GET, strlen(TM_TOPIC_PROP_GET)))
    {
        tm_prop_get(payload, payload_len);
    }
    else if((0 == strncmp(action, TM_TOPIC_PROP_POST, strlen(TM_TOPIC_PROP_POST)))
            || (0 == strncmp(action, TM_TOPIC_EVENT_POST, strlen(TM_TOPIC_EVENT_POST)))
            || (0 == strncmp(action, TM_TOPIC_DESIRED_PROPS_GET, strlen(TM_TOPIC_DESIRED_PROPS_GET)))
            || (0 == strncmp(action, TM_TOPIC_DESIRED_PROPS_DELETE, strlen(TM_TOPIC_DESIRED_PROPS_DELETE)))
            || (0 == strncmp(action, TM_TOPIC_PACK_DATA_POST, strlen(TM_TOPIC_PACK_DATA_POST)))
            || (0 == strncmp(action, TM_TOPIC_HISTORY_DATA_POST, strlen(TM_TOPIC_HISTORY_DATA_POST))))
    {
        tm_post_reply(payload, payload_len);
    }
    else if(0 == strncmp(action, "/service/", 9))
    {
        char svc_id[32] = {0};
        char *tmp_ptr = NULL;
        const char *svc_id_ptr = res_name + strlen(g_tm_obj.topic_prefix) + 9;

        tmp_ptr = strstr(svc_id_ptr, "/");
        memcpy(svc_id, svc_id_ptr, tmp_ptr - svc_id_ptr);

        tm_service_invoke(svc_id, payload, payload_len);
    }
#ifdef FEATURE_TM_GATEWAY_ENABLED
    else if(0 == strncmp(action, "/sub/", 5))
    {
        ESP_LOGD(TAG,"");
        if(g_tm_obj.subdev_callback)
        {
        	ESP_LOGD(TAG,"");
            g_tm_obj.subdev_callback(action, payload, payload_len);
        }
        if(NULL != strstr(action, "/reply/"))
        {
        	ESP_LOGD(TAG,"");
            tm_post_reply(payload, payload_len);
        }
    }
#endif
#ifdef FEATURE_TM_OTA_ENABLED
    else if(0 == strncmp(ota_action, "/inform", 7))
    {
        tm_ota(payload, payload_len);
    }
#endif
    return 0;
}

int32_t tm_init(struct tm_downlink_tbl_t *downlink_tbl)
{
    memset(&g_tm_obj, 0, sizeof(g_tm_obj));
    g_tm_obj.downlink_tbl.prop_tbl = downlink_tbl->prop_tbl;
    g_tm_obj.downlink_tbl.prop_tbl_size = downlink_tbl->prop_tbl_size;
    g_tm_obj.downlink_tbl.svc_tbl = downlink_tbl->svc_tbl;
    g_tm_obj.downlink_tbl.svc_tbl_size = downlink_tbl->svc_tbl_size;
#if FEATURE_TM_OTA_ENABLED
    g_tm_obj.downlink_tbl.ota_cb = downlink_tbl->ota_cb;
#endif

#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
    return tm_mqtt_init(tm_data_parse);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    return tm_coap_init(tm_data_parse);
#endif
}

#ifdef FEATURE_TM_GATEWAY_ENABLED
int32_t tm_set_subdev_callback(tm_subdev_cb callback)
{
    g_tm_obj.subdev_callback = callback;

    return ERR_OK;
}
#endif

int32_t tm_login(const char *product_id, const char *dev_name, const char *access_key, uint32_t timeout_ms)
{
    int32_t ret = ERR_OK;
    char dev_token[256] = {0};
    uint32_t topic_prefix_len = strlen(product_id) + strlen(dev_name);
#ifdef FEATURE_TM_OTA_ENABLED
    uint32_t ota_topic_prefix_len = topic_prefix_len + strlen(TM_TOPIC_OTA_PREFIX);
#endif
    topic_prefix_len += strlen(TM_TOPIC_PREFIX);

    if(g_tm_obj.topic_prefix)
    {
        free(g_tm_obj.topic_prefix);
    }

    g_tm_obj.topic_prefix = malloc(topic_prefix_len);
    if(NULL == g_tm_obj.topic_prefix)
    {
        return ERR_NO_MEM;
    }
    memset(g_tm_obj.topic_prefix, 0, topic_prefix_len);
    sprintf(g_tm_obj.topic_prefix, TM_TOPIC_PREFIX, product_id, dev_name);

#ifdef FEATURE_TM_OTA_ENABLED
    if(g_tm_obj.ota_topic_prefix)
    {
        free(g_tm_obj.ota_topic_prefix);
    }

    g_tm_obj.ota_topic_prefix = malloc(ota_topic_prefix_len);
    if(NULL == g_tm_obj.ota_topic_prefix)
    {
        return ERR_NO_MEM;
    }
    memset(g_tm_obj.ota_topic_prefix, 0, ota_topic_prefix_len);
    sprintf(g_tm_obj.ota_topic_prefix, TM_TOPIC_OTA_PREFIX, product_id, dev_name);
#endif

    dev_token_generate(dev_token, SIG_METHOD_MD5, 2524579200, product_id, dev_name, access_key);
    ESP_LOGD(TAG,"token = %s\n", dev_token);

#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
    ret = tm_mqtt_login(product_id, dev_name, dev_token, FEATURE_TM_LIFE_TIME, timeout_ms);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    ret = tm_coap_login(product_id, dev_name, dev_token, FEATURE_TM_LIFE_TIME, timeout_ms);
#endif
#ifdef FEATURE_TM_FMS
    if(ERR_OK == ret)
    {
        g_tm_obj.product_id = strdup(product_id);
        g_tm_obj.dev_name = strdup(dev_name);
        g_tm_obj.dev_token = strdup(dev_token);
    }
#endif
    return ret;
}

int32_t tm_logout(uint32_t timeout_ms)
{
#ifdef FEATURE_TM_FMS
    free(g_tm_obj.product_id);
    free(g_tm_obj.dev_name);
    free(g_tm_obj.dev_token);
#endif
    if(g_tm_obj.topic_prefix)
    {
        free(g_tm_obj.topic_prefix);
        g_tm_obj.topic_prefix = NULL;
    }
#ifdef FEATURE_TM_OTA_ENABLED
    if(g_tm_obj.ota_topic_prefix)
    {
        free(g_tm_obj.ota_topic_prefix);
        g_tm_obj.ota_topic_prefix = NULL;
    }
#endif
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
    return tm_mqtt_logout(timeout_ms);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    return tm_coap_logout(timeout_ms);
#endif
}


int32_t tm_post_raw(const char *name, uint8_t *raw_data, uint32_t raw_data_len, uint8_t **reply_data,
                    uint32_t *reply_data_len, uint32_t timeout_ms)
{
    return tm_send_request(name, 1, raw_data, raw_data_len, (void **)reply_data, reply_data_len, timeout_ms);
}

int32_t tm_post_property(void *prop_data, uint32_t timeout_ms)
{
    return tm_send_request(TM_TOPIC_PROP_POST, 0, prop_data, 0, NULL, NULL, timeout_ms);
}

int32_t tm_post_event(void *event_data, uint32_t timeout_ms)
{
    return tm_send_request(TM_TOPIC_EVENT_POST, 0, event_data, 0, NULL, NULL, timeout_ms);
}

int32_t tm_get_desired_props(uint32_t timeout_ms)
{
    void *prop_list = tm_data_array_create(g_tm_obj.downlink_tbl.prop_tbl_size);
    uint32_t i = 0;
    int32_t ret = ERR_OTHERS;
    void *reply_data = NULL;

    for(i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++)
    {
        tm_data_array_set_string(prop_list, (char *)(g_tm_obj.downlink_tbl.prop_tbl[i].name));
    }

    ret = tm_send_request(TM_TOPIC_DESIRED_PROPS_GET, 0, prop_list, 0, &reply_data, NULL, timeout_ms);

    if(ERR_OK == ret)
    {
        tm_data_list_each(reply_data, tm_prop_set_handle);
        ESP_LOGD(TAG,"get desired props ok\n");
    }
    if(NULL != reply_data)
    {
        tm_data_delete(reply_data);
    }

    return ret;
}

int32_t tm_delete_desired_props(uint32_t timeout_ms)
{
    void *prop_list = tm_data_create();
    uint32_t i = 0;
    int32_t ret = ERR_OTHERS;

    for(i = 0; i < g_tm_obj.downlink_tbl.prop_tbl_size; i++)
    {
        tm_data_struct_set_data(prop_list, (char *)(g_tm_obj.downlink_tbl.prop_tbl[i].name), NULL);
    }

    ret = tm_send_request(TM_TOPIC_DESIRED_PROPS_DELETE, 0, prop_list, 0, NULL, NULL, timeout_ms);

    return ret;
}

void *tm_pack_device_data(void *data, const char *product_id, const char *dev_name, void *prop, void *event,
                          char as_raw)
{
    return tm_onejson_pack_props_and_events(data, product_id, dev_name, prop, event, as_raw);
}

int32_t tm_post_pack_data(void *pack_data, uint32_t timeout_ms)
{
    return tm_send_request(TM_TOPIC_PACK_DATA_POST, 0, pack_data, 0, NULL, 0, timeout_ms);
}

int32_t tm_post_history_data(void *history_data, uint32_t timeout_ms)
{
    return tm_send_request(TM_TOPIC_HISTORY_DATA_POST, 0, history_data, 0, NULL, 0, timeout_ms);
}

int32_t tm_step(uint32_t timeout_ms)
{
#if FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_MQTT
    return tm_mqtt_step(timeout_ms);
#elif FEATURE_TM_PROTOCOL == FEATURE_TM_PROTOCOL_COAP
    return tm_coap_step(timeout_ms);
#endif
}

#ifdef FEATURE_TM_FMS
    #include <wolfcrypt/md5.h>
    #include "http_api.h"
    #include "utils.h"

int32_t tm_file_get(tm_file_cb callback, char *file_id, uint32_t file_size, uint32_t segment_size,
                    uint32_t timeout_ms)
{
    handle_t cd_hdl = countdown_start(timeout_ms);
    void *http_ctx = NULL;
    char http_path[128] = {0};
    char http_header_range[32] = {0};
    uint8_t *http_resp = NULL;
    uint32_t http_resp_len = 0;
    uint32_t range_start = 0;
    uint32_t range_end = 0;
    struct tm_file_data_t file_data = {0};
    int32_t ret = 0;
    int32_t retry = 1;

    if((NULL == callback) || (NULL == file_id) || (0 == file_size))
    {
        countdown_stop(timeout_ms);
        return ERR_INVALID_PARAM;
    }

    sprintf(http_path, "/studio/%s/%s/%s/outdownload", g_tm_obj.product_id, g_tm_obj.dev_name, file_id);
    do
    {
        range_start = (range_end) ? (range_end + 1) : 0;
        range_end = ((file_size - range_start) > segment_size) ? (range_start + segment_size - 1) : 0;

        http_ctx = http_new(HTTP_METHOD_GET, "studio-file.heclouds.com", http_path, segment_size + 800);
        http_add_header(http_ctx, "Content-Type", "application/json");
        http_add_header(http_ctx, "Authorization", g_tm_obj.dev_token);

        sprintf(http_header_range, "bytes=%d-", range_start);
        if(range_end)
        {
            sprintf(http_header_range + strlen(http_header_range), "%d", range_end);
        }
        http_add_header(http_ctx, "Range", http_header_range);

        ret = http_send_request(http_ctx, "183.230.102.116", 27001, &(file_data.data), &(file_data.data_len), timeout_ms);
        if((200 <= ret) && (300 > ret))
        {
            retry = 0;
            file_data.seq = range_start;
            callback(TM_FILE_EVENT_GET_DATA, (void *)&file_data);
        }
        else
        {
            http_delete(http_ctx);
            countdown_stop(cd_hdl);
            return ret;
        }
        http_delete(http_ctx);
    } while((0 == countdown_is_expired(cd_hdl)) && (0 != range_end));

    countdown_stop(cd_hdl);

    return ERR_OK;
}

static void fp_add_form_data(void *http_ctx, const char *boundary, const char *name, uint8_t *value,
                             uint32_t value_len, const char *addl)
{
    http_add_body(http_ctx, "--", 2);
    http_add_body(http_ctx, boundary, strlen(boundary));
    if(name)
    {
        char str_tmp[128] = {0};

        http_add_body(http_ctx, "\r\n", 2);
        sprintf(str_tmp, "Content-Disposition:form-data;name=\"%s\"", name);
        if(addl)
        {
            strcat(str_tmp, ";");
            strcat(str_tmp, addl);
        }
        strcat(str_tmp, "\r\n\r\n");
        http_add_body(http_ctx, str_tmp, strlen(str_tmp));
        http_add_body(http_ctx, value, value_len);
    }
    else
    {
        http_add_body(http_ctx, "--", 2);
    }
    http_add_body(http_ctx, "\r\n", 2);
}

int32_t tm_file_post(tm_file_cb callback, const char *product_id, const char *dev_name, const char *access_key,
                     const char *file_name, uint8_t *file, uint32_t file_size, uint32_t timeout_ms)
{
    char boundary[] = "----7MA4YWxkTrZu0gW";
    void *http_ctx = NULL;
    uint8_t *http_resp = NULL;
    uint32_t http_resp_len = 0;
    uint8_t file_md5[16] = {0};
    char str_tmp[256] = {0};
    int32_t ret = ERR_OK;

    if((NULL == callback) || (NULL == product_id) || (NULL == dev_name) || (NULL == access_key) || (NULL == file_name)
       || (NULL == file) || (0 == file_size))
    {
        return ERR_INVALID_PARAM;
    }

    sprintf(str_tmp, "/studio/%s/%s/outupload", product_id, dev_name);
    http_ctx = http_new(HTTP_METHOD_POST, "studio-file.heclouds.com", str_tmp, file_size + 800);
    sprintf(str_tmp, "multipart/form-data;boundary=%s", boundary);
    http_add_header(http_ctx, "Content-Type", str_tmp);
    dev_token_generate(str_tmp, SIG_METHOD_SHA1, 2524579200, product_id, dev_name, access_key);
    http_add_header(http_ctx, "Authorization", str_tmp);

    /** Set Body*/
    wc_Md5Hash(file, file_size, file_md5);
    hex_to_str(str_tmp, file_md5, 16);
    str_tmp[32] = '\0';
    fp_add_form_data(http_ctx, boundary, "md5", str_tmp, strlen(str_tmp), NULL);

    fp_add_form_data(http_ctx, boundary, "filename", file_name, strlen(file_name), NULL);

    sprintf(str_tmp, "%d", file_size);
    fp_add_form_data(http_ctx, boundary, "size", str_tmp, strlen(str_tmp), NULL);

    sprintf(str_tmp, "filename=\"%s\"", file_name);
    fp_add_form_data(http_ctx, boundary, "file", file, file_size, str_tmp);

    fp_add_form_data(http_ctx, boundary, NULL, NULL, 0, NULL);

    ret = http_send_request(http_ctx, "183.230.102.116", 27001, &http_resp, &http_resp_len, timeout_ms);
    if(200 == ret)
    {
        if(http_resp)
        {
            char *tmp_ptr = strstr(http_resp, "\"uuid\":");
            char *cb_data = tmp_ptr + 8;

            if(tmp_ptr)
            {
                tmp_ptr = strstr(cb_data, "\"");
                *tmp_ptr = '\0';
                callback(TM_FILE_EVENT_POST_SUCCESSED, cb_data);
                ret = ERR_OK;
            }
            else
            {
                tmp_ptr = strstr(http_resp, "\"error\":");
                cb_data = tmp_ptr + 9;
                tmp_ptr = strstr(cb_data, "\"");
                *tmp_ptr = '\0';
                callback(TM_FILE_EVENT_POST_FAILED, cb_data);
                ret = ERR_OTHERS;
            }
        }
    }
    else
    {
        ret = ERR_OTHERS;
    }

    http_delete(http_ctx);

    return ret;
}
#endif
