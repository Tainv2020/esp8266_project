#include "app_mqtt.h"
#include "esp_log.h"

static const char *TAG = "MQTTS_APP";

static esp_mqtt_conn_params_t mqtt_conn_param;

extern char HOMEID[30];
extern char USERID[30];
extern char CLIENTID[30];

void subscribe_state_callback(const char *topic, void *payload, size_t payload_len, void *priv_data)
{
    ESP_LOGI(TAG,"data sub: %.*s",payload_len,(char *)payload);
}


void app_mqtt_init(void)
{
    mqtt_conn_param.mqtt_host = "broker.hivemq.com";
    mqtt_conn_param.client_id = CLIENTID;
    // mqtt_conn_param.lwt_topic = stalib_get_topic_heartbeat();
    // mqtt_conn_param.lwt_msg = "Offline";
    // mqtt_conn_param.lwt_msg_len = 7;
    // mqtt_conn_param.lwt_qos = MQTT_QOS1;
    // mqtt_conn_param.lwt_retain = 1;
    // mqtt_conn_param.keepalive = 20;
    esp_mqtt_init(&mqtt_conn_param);
}

void app_mqtt_start(void)
{
    char topic[90];
    snprintf(topic, 90, "%s/%s/%s", USERID, HOMEID, CLIENTID);
    esp_mqtt_add_topic_to_subscribe_list(topic,subscribe_state_callback, MQTT_QOS1,NULL);
    esp_mqtt_connect();
}

void app_mqtt_publish(const char *topic, uint8_t *data, int data_len)
{
    esp_mqtt_publish(topic,data,data_len,MQTT_QOS1,NULL);
}

void app_mqtt_subscriber(const char *topic, esp_mqtt_subscribe_cb_t cb)
{
    esp_mqtt_subscribe(topic,cb,MQTT_QOS1,NULL);
}
