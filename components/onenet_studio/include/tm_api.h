/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file tm_api.h
 * @brief
 */

#ifndef __TM_API_H__
#define __TM_API_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define TM_PROPERTY_RW(x)                                                                                              \
    {                                                                                                                  \
        #x, tm_prop_##x##_rd_cb, tm_prop_##x##_wr_cb                                                                   \
    }

#define TM_PROPERTY_RO(x)                                                                                              \
    {                                                                                                                  \
        #x, tm_prop_##x##_rd_cb, NULL                                                                                  \
    }

#define TM_SERVICE(x)                                                                                                  \
    {                                                                                                                  \
        #x, tm_svc_##x##_cb                                                                                            \
    }

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/
typedef int32_t (*tm_prop_read_cb)(void *res);
typedef int32_t (*tm_prop_write_cb)(void *res);
typedef int32_t (*tm_event_read_cb)(void *res);
typedef int32_t (*tm_svc_invoke_cb)(void *in, void *out);

#ifdef FEATURE_TM_OTA_ENABLED
typedef int32_t (*tm_ota_cb)(void);
#endif

struct tm_prop_tbl_t
{
    const char *name;
    tm_prop_read_cb tm_prop_rd_cb;
    tm_prop_write_cb tm_prop_wr_cb;
};

struct tm_svc_tbl_t
{
    const char *name;
    tm_svc_invoke_cb tm_svc_cb;
};

struct tm_downlink_tbl_t
{
    struct tm_prop_tbl_t *prop_tbl;
    struct tm_svc_tbl_t *svc_tbl;
    uint16_t prop_tbl_size;
    uint16_t svc_tbl_size;
#ifdef FEATURE_TM_OTA_ENABLED
    tm_ota_cb ota_cb;
#endif
};
/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
int32_t tm_init(struct tm_downlink_tbl_t *downlink_tbl);
int32_t tm_login(const char *product_id, const char *dev_name, const char *access_key, uint32_t timeout_ms);
int32_t tm_logout(uint32_t timeout_ms);
int32_t tm_post_property(void *prop_data, uint32_t timeout_ms);
int32_t tm_post_event(void *event_data, uint32_t timeout_ms);
int32_t tm_get_desired_props(uint32_t timeout_ms);
int32_t tm_delete_desired_props(uint32_t timeout_ms);

void *tm_pack_device_data(void *data, const char *product_id, const char *dev_name, void *prop, void *event,
                          char as_raw);
int32_t tm_post_pack_data(void *pack_data, uint32_t timeout_ms);
int32_t tm_post_history_data(void *history_data, uint32_t timeout_ms);

#ifdef FEATURE_TM_GATEWAY_ENABLED
typedef int32_t (*tm_subdev_cb)(const char *name, void *data, uint32_t data_len);

int32_t tm_set_subdev_callback(tm_subdev_cb callback);

int32_t tm_post_raw(const char *name, uint8_t *raw_data, uint32_t raw_data_len, uint8_t **reply_data,
                    uint32_t *reply_data_len, uint32_t timeout_ms);
int32_t tm_send_request(const char *name, uint8_t as_raw, void *data, uint32_t data_len, void **reply_data,
                        uint32_t *reply_data_len, uint32_t timeout_ms);
int32_t tm_send_response(const char *name, char *msg_id, int32_t msg_code, uint8_t as_raw, void *resp_data,
                         uint32_t resp_data_len, uint32_t timeout_ms);
#endif
int32_t tm_step(uint32_t timeout_ms);

#ifdef FEATURE_TM_FMS
struct tm_file_data_t
{
    /** ?????????????????????????????????????????????????????????0??????*/
    uint32_t seq;
    /** ?????????????????????????????????*/
    uint8_t *data;
    /** ????????????????????????*/
    uint32_t data_len;
};

enum tm_file_event_e
{
    /** ????????????????????????????????????struct tm_file_data_t*/
    TM_FILE_EVENT_GET_DATA = 0,
    /** ????????????????????????????????????????????????????????????ID???????????????*/
    TM_FILE_EVENT_POST_SUCCESSED,
    /** ????????????????????????????????????????????????????????????????????????*/
    TM_FILE_EVENT_POST_FAILED,
};
typedef int32_t tm_file_cb(enum tm_file_event_e event, void *event_data);

int32_t tm_file_get(tm_file_cb callback, char *file_id, uint32_t file_size, uint32_t segment_size,
                    uint32_t timeout_ms);
int32_t tm_file_post(tm_file_cb callback, const char *product_id, const char *dev_name, const char *access_key,
                     const char *file_name, uint8_t *file, uint32_t file_size, uint32_t timeout_ms);
#endif

#ifdef __cplusplus
}
#endif

#endif
