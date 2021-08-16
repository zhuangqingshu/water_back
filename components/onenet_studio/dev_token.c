/**
 * Copyright (c), 2012~2020 iot.10086.cn All Rights Reserved
 *
 * @file dev_token.c
 * @brief
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "dev_token.h"

#include <wolfcrypt/hmac.h>
#include <wolfcrypt/sha.h>
#include <esp_base64.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/
#define DEV_TOKEN_LEN 256
#define DEV_TOKEN_VERISON_STR "2018-10-31"

#define DEV_TOKEN_SIG_METHOD_MD5 "md5"
#define DEV_TOKEN_SIG_METHOD_SHA1 "sha1"
#define DEV_TOKEN_SIG_METHOD_SHA256 "sha256"
/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
int32_t dev_token_generate(char *token, enum sig_method_e method, uint32_t exp_time, const char *product_id,
                           const char *dev_name, const char *access_key)
{
    char base64_data[64] = { 0 };
    char str_for_sig[64] = { 0 };
    char sign_buf[128] = { 0 };
    uint32_t base64_data_len = sizeof(base64_data);
    char *sig_method_str = NULL;
    uint32_t sign_len = 0;
    int i = 0;
    char *tmp = NULL;
    Hmac hmac;

    sprintf(token, "version=%s", DEV_TOKEN_VERISON_STR);

    sprintf(token + strlen(token), "&res=products%%2F%s%%2Fdevices%%2F%s", product_id, dev_name);

    sprintf(token + strlen(token), "&et=%u", exp_time);

    base64_data_len = esp_base64_decode(access_key, strlen(access_key), base64_data, base64_data_len);

    wc_HmacSetKey(&hmac, MD5, (byte *)base64_data, base64_data_len);
    sig_method_str = DEV_TOKEN_SIG_METHOD_MD5;
    sign_len = 16;

    sprintf(token + strlen(token), "&method=%s", sig_method_str);

    sprintf(str_for_sig, "%u\n%s\nproducts/%s/devices/%s\n%s", exp_time, sig_method_str, product_id, dev_name, DEV_TOKEN_VERISON_STR);
    wc_HmacUpdate(&hmac, (byte *)str_for_sig, strlen(str_for_sig));
    wc_HmacFinal(&hmac, (byte *)sign_buf);

    memset(base64_data, 0, sizeof(base64_data));
    base64_data_len = sizeof(base64_data);
    base64_data_len = esp_base64_encode(sign_buf, sign_len, base64_data, base64_data_len);

    strcat(token, "&sign=");
    tmp = token + strlen(token);

    for(i = 0; i < base64_data_len; i++)
    {
        switch(base64_data[i])
        {
        case '+':
            strcat(tmp, "%2B");
            tmp += 3;
            break;
        case ' ':
            strcat(tmp, "%20");
            tmp += 3;
            break;
        case '/':
            strcat(tmp, "%2F");
            tmp += 3;
            break;
        case '?':
            strcat(tmp, "%3F");
            tmp += 3;
            break;
        case '%':
            strcat(tmp, "%25");
            tmp += 3;
            break;
        case '#':
            strcat(tmp, "%23");
            tmp += 3;
            break;
        case '&':
            strcat(tmp, "%26");
            tmp += 3;
            break;
        case '=':
            strcat(tmp, "%3D");
            tmp += 3;
            break;
        default:
            *tmp = base64_data[i];
            tmp += 1;
            break;
        }
    }

    return 0;
}

