/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <sys/param.h>
#include "esp_netif.h"
#include "nvs.h"
#include <esp_http_server.h>

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

const char WEB_ROOT[]= "<!DOCTYPE html>\
<html>\
<body>\
<center>\
<h1>WiFi LED on off demo: 1</h1><br>\
Ciclk to turn <a href=\"ON\" target=\"myIframe\">LED ON</a><br>\
Ciclk to turn <a href=\"OFF\" target=\"myIframe\">LED OFF</a><br>\
Ciclk to turn <a href=\"MODE\" target=\"myIframe\">MODE</a><br>\
LED State:<iframe name=\"myIframe\" width=\"100\" height=\"25\" frameBorder=\"0\"><br>\
<hr>\
</body>\
</html>";

static const char *TAG = "webserver";

/* An HTTP GET handler */
esp_err_t root_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "Root web");
    const char *response = (const char*)req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending response", error);
    }
    else ESP_LOGI(TAG, "Response send successfully");

    return error;
}

httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = WEB_ROOT
};

/* An HTTP GET handler */
esp_err_t task1_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "LED ON");
    const char *response = "LED ON";
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending response", error);
    }
    else ESP_LOGI(TAG, "Response send successfully");

    return error;
}

httpd_uri_t task1 = {
    .uri       = "/ON",
    .method    = HTTP_GET,
    .handler   = task1_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP GET handler */
esp_err_t task2_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "LED OFF");
    const char *response = "LED OFF";
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending response", error);
    }
    else ESP_LOGI(TAG, "Response send successfully");

    return error;
}

httpd_uri_t task2 = {
    .uri       = "/OFF",
    .method    = HTTP_GET,
    .handler   = task2_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP GET handler */
esp_err_t task3_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "MODE");
    const char *response = "MODE";
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    {
        ESP_LOGI(TAG, "Error %d while sending response", error);
    }
    else ESP_LOGI(TAG, "Response send successfully");

    return error;
}

httpd_uri_t task3 = {
    .uri       = "/MODE",
    .method    = HTTP_GET,
    .handler   = task3_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &task1);
        httpd_register_uri_handler(server, &task2);
        httpd_register_uri_handler(server, &task3);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static httpd_handle_t server = NULL;

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap()
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
}
