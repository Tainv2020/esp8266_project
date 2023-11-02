#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "tcpip_adapter.h"

#include "app_config.h"
#include "app_sc.h"
#include "app_http_server.h"
#include "common_wifi.h"
#include "app_mqtt.h"
#include "app_nvs_flash.h"

static const char *TAG = "app_config";

static wifi_config_t wifi_config;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

static const int WIFI_STA_HTTP_DONE_BIT = BIT0;
static const int WIFI_STA_CONNECTED_BIT = BIT1;
static const int WIFI_STA_RECV_WIFI_INFO_BIT = BIT2;
// static const int WIFI_STA_ESPTOUCH_DONE_BIT =  BIT3;

extern char HOMEID[50];
extern char USERID[50];
extern char CLIENTID[50];

static bool provisioned = false;

esp_provision_mode_t provision_mode = ESP_PROVISION_MODE_SMARTCONFIG;

void smartconfig_example_task(void * parm);

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
        if(provisioned)
        app_mqtt_start();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void http_server_post_callback(char* buf,int len)
{
    ESP_LOGI(TAG,"Post data: %.*s",len,buf);
    static bool recv_ssid = false;
    static bool recv_pass = false;
    static bool recv_homeid = false;
    static bool recv_userid = false;    
    if(buf[0] == 's')
    {
        memcpy(wifi_config.sta.ssid, &buf[1], len-1);
        recv_ssid = true;
    }
    if(buf[0] == 'p')
    {
        memcpy(wifi_config.sta.password, &buf[1], len-1);
        if (strlen((char*)wifi_config.sta.password))
            wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        recv_pass = true;
    }

    if(buf[0] == 'h')
    {
        memcpy(HOMEID, &buf[1], len-1);
        app_flash_set_homeid(HOMEID);
        recv_homeid = true;
    }

    if(buf[0] == 'u')
    {
        memcpy(USERID, &buf[1], len-1);
        app_flash_set_userid(USERID);
        recv_userid = true;
    }

    if(recv_ssid & recv_pass){
        recv_ssid = recv_pass = false;
        xEventGroupSetBits(wifi_event_group, WIFI_STA_RECV_WIFI_INFO_BIT);
    }

    // if(recv_homeid & recv_userid){
    //     recv_homeid = recv_userid = false;
    //     xEventGroupSetBits(wifi_event_group, WIFI_STA_HTTP_DONE_BIT);
    // }    
}

void http_server_get_callback(char* userid)
{
    if(userid){
        printf("userid: %s\n", userid);
        app_http_server_send_resp(CLIENTID,strlen(CLIENTID));
        strcpy(USERID, userid);
        app_flash_set_userid(USERID);
        vTaskDelay(500/portTICK_PERIOD_MS);
        xEventGroupSetBits(wifi_event_group, WIFI_STA_HTTP_DONE_BIT);
    }
}

static void wifi_init(bool *provisioned)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    /* Get WiFi Station configuration */
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg) != ESP_OK) {
        return ESP_FAIL;
    }

    if (strlen((const char*) wifi_cfg.sta.ssid)) {
        *provisioned = true;
        ESP_LOGI(TAG, "Found ssid %s",     (const char*) wifi_cfg.sta.ssid);
        ESP_LOGI(TAG, "Found password %s", (const char*) wifi_cfg.sta.password);
    }
    else{
        ESP_LOGI(TAG, "Not Found Wifi");
    }
}

void app_config_start(void)
{
    wifi_event_group = xEventGroupCreate();
    wifi_init(&provisioned);
    if(!provisioned)
    {
        if(provision_mode == ESP_PROVISION_MODE_ACCESSPOINT){
            /* Start WiFi softAP with specified ssid and password */
            start_wifi_ap();

            /* Initialize http server*/
            app_http_server_post_set_callback(http_server_post_callback);
            app_http_server_get_set_callback(http_server_get_callback);
            app_http_server_start();
            xEventGroupWaitBits(wifi_event_group, WIFI_STA_HTTP_DONE_BIT , false, true, portMAX_DELAY);
            app_http_server_stop();
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_start() );
            xEventGroupWaitBits(wifi_event_group, WIFI_STA_CONNECTED_BIT , false, true, portMAX_DELAY);
            app_mqtt_start();
        } else if(provision_mode == ESP_PROVISION_MODE_SMARTCONFIG){
            start_wifi_sta();
            smartconfig_start();   
            xEventGroupWaitBits(wifi_event_group, WIFI_STA_CONNECTED_BIT , false, true, portMAX_DELAY);
            /* Initialize http server*/
            app_http_server_post_set_callback(http_server_post_callback);
            app_http_server_get_set_callback(http_server_get_callback);
            app_http_server_start();
            xEventGroupWaitBits(wifi_event_group, WIFI_STA_HTTP_DONE_BIT , false, true, portMAX_DELAY);
            app_http_server_stop();
            app_mqtt_start();            
        }
        provisioned = true;
    }
    else{
        /* Start WiFi station with credentials set during provisioning */
        ESP_LOGI(TAG, "Starting WiFi station");
        start_wifi_sta();        
    }
}   