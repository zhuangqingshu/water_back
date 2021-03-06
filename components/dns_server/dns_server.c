/*****************************************************************************
 *
 * MODULE:             JN-AN-1217
 *
 * COMPONENT:          app_led_interface.c
 *
 * DESCRIPTION:        DK4 DR1175 Led interface (White Led)
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2016. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "esp_system.h"
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static const char *TAG = "DNS_SERVER";

static int create_udp_socket(int port)
{
    struct sockaddr_in saddr = { 0 };
    int sock = -1;
    int err = 0;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG,"Failed to create socket. Error %d", errno);
        return -1;
    }

    // Bind the socket to any address
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0) {
        ESP_LOGE(TAG,"Failed to bind socket. Error %d", errno);
        goto err;
    }

    // All set, socket is configured for sending and receiving
    return sock;

err:
    close(sock);
    return -1;
}

static void my_dns_server(void *pvParameters)
{
/* Wait for all the IPs we care about to be set
    */
    uint8_t data[128];
    int len = 0;
    struct sockaddr_in client = { 0 };
    socklen_t  client_len=sizeof(struct sockaddr_in); 
    uint32_t i = 0;

    ESP_LOGI(TAG,"DNS server start ...");

    int sock = create_udp_socket(53);

    if (sock < 0) {
        ESP_LOGE(TAG,"Failed to create IPv4 multicast socket");
    }

    while(1)
    {

        len=recvfrom(sock,data,100,0,(struct sockaddr *)&client,&client_len); //?????????

        if((len < 0) || ( len > 100))
        {
            ESP_LOGE(TAG,"recvfrom error\n");
            continue;
        }

        ESP_LOGD(TAG,"DNS request:");
        for(i = 0x4; i< len;i++)
        {
            if((data[i] >= 'a' && data[i] <= 'z') || (data[i] >= 'A' && data[i] <= 'Z') ||(data[i] >= '0' && data[i] <= '9'))
                printf("%c",data[i]);
            else
                printf("_");

        }
        printf("\r\n");
        //printf("%d\r\n",esp_get_free_heap_size()); //????????????????????????

        //?????????????????????????????????
        if( strstr((const char *)data+0xc,"taobao")||
            strstr((const char *)data+0xc,"qq")    || 
            strstr((const char *)data+0xc,"sogou") ||
            strstr((const char *)data+0xc,"amap")  ||
            strstr((const char *)data+0xc,"alipay")||
            strstr((const char *)data+0xc,"youku") ||
            strstr((const char *)data+0xc,"iqiyi") ||
            strstr((const char *)data+0xc,"baidu"))
        {
            continue;
        }

        data[2] |= 0x80;
        data[3] |= 0x80;
        data[7] =1;

        data[len++] =0xc0;
        data[len++] =0x0c;

        data[len++] =0x00;
        data[len++] =0x01;
        data[len++] =0x00;
        data[len++] =0x01;

        data[len++] =0x00;
        data[len++] =0x00;
        data[len++] =0x00;
        data[len++] =0x0A;

        data[len++] =0x00;
        data[len++] =0x04;

        data[len++] =192;
        data[len++] =168;
        data[len++] =4;
        data[len++] =1;

        /*?????????????????????????????????*/
        // inet_ntop(AF_INET,&client.sin_addr,(char *)data,sizeof(data));
        // printf("client IP is %s, port is %d\n",data,ntohs(client.sin_port));

        sendto(sock,data,len,0,(struct sockaddr*)&client,client_len);

        vTaskDelay(10);
    }

    ESP_LOGE(TAG,"DNS???server stop ...");
    shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL);
}

void dns_server_start()
{
    xTaskCreate(&my_dns_server, "dns_task", 2048, NULL, DEFAULT_THREAD_PRIO, NULL);
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
