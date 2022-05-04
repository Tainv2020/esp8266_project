#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "app_http_server.h"

static const char *TAG="APP";
static void app_http_server_post_default_handler_func(char* buf,int len);
static void app_http_server_get_default_handler_func(char* userid);
static http_post_handler_func_t http_post_handler_func = app_http_server_post_default_handler_func;
static http_get_handler_func_t  http_get_handler_func = app_http_server_get_default_handler_func;
static httpd_req_t* resp;
static char http_post_buf[APP_HTTP_SERVER_POST_MAX_BUFFER];
static httpd_handle_t http_server;
/**
 * bref: Default callback post receive data
 * pram: buf: data receive from client
 *       len: length of data receive 
 * return: None
*/
static void app_http_server_post_default_handler_func(char* buf,int len)
{
    ESP_LOGI(TAG, "=========== RECEIVED POST DATA ==========");
}
/**
 * bref: Default callback for receive param 
 * pram: type:  HTTP_GET_RECEIVE_HEADER : received header
 *              HTTP_GET_RECEIVE_QUERY, : received Query string
 *              HTTP_GET_RECEIVE_LOST   : lost request
 *       buf: data receive from client
 *       len: length of data receive 
 * return: None
*/
static void app_http_server_get_default_handler_func(char* userid)
{
    ESP_LOGI(TAG, "=========== RECEIVED GET PARAM ==========");
}

void app_http_server_send_resp(char* buf, int len)
{
    httpd_resp_send(resp, buf, len);
}

/* An HTTP GET handler */
esp_err_t http_get_handler(httpd_req_t *req)
{
    resp = req;  /*Set current http resp pointer to req*/

    int buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        char * buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "userid", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
                http_get_handler_func(param);
            }
        }
        free(buf);
    }
    return ESP_OK;
}

httpd_uri_t http_get = {
    .uri       = APP_HTTP_SERVER_GET_DEFAULT_URI,
    .method    = HTTP_GET,
    .handler   = http_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

/* An HTTP POST handler */
static esp_err_t http_post_handler(httpd_req_t *req)
{
    char* buf = http_post_buf;
    int buf_len = sizeof(http_post_buf);
    int ret, remaining = req->content_len;
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, buf_len))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        //httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;
        http_post_handler_func(buf,req->content_len);
    }
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t http_post = {
    .uri       = APP_HTTP_SERVER_POST_DEFAULT_URI,
    .method    = HTTP_POST,
    .handler   = http_post_handler,
    .user_ctx  = NULL
};

void app_http_server_start(void)
{
    // httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&http_server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(http_server, &http_get);
        httpd_register_uri_handler(http_server, &http_post);
        return;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void app_http_server_stop(void)
{
    // Stop the httpd server
    if(http_server){
        httpd_stop(http_server);
        http_server = NULL;
    }
}
/**
 * bref: Set callback for post receive data from client
 * pram: None
 * return: None
*/
void app_http_server_post_set_callback(void* post_handler_callback)
{
    if(post_handler_callback != NULL){
        http_post_handler_func = (http_post_handler_func_t) post_handler_callback;
    }
}
/**
 * bref: Set callback for http from client
 * pram: None
 * return: None
*/
void app_http_server_get_set_callback(void* get_handler_callback)
{
    if(get_handler_callback != NULL)
    {
        http_get_handler_func = (http_get_handler_func_t) get_handler_callback;
    }
}