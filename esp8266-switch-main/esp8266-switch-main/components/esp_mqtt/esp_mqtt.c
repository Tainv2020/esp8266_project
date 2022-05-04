#include "esp_mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

static const char *TAG = "app_mqtt";
static esp_mqtt_client_handle_t mqtt_client;

#define MAX_MQTT_SUBSCRIPTIONS      5

typedef struct {
    char *topic;
    esp_mqtt_subscribe_cb_t cb;
    void *priv;
} esp_mqtt_subscription_t;

typedef struct {
    esp_mqtt_client_handle_t mqtt_client;
    esp_mqtt_conn_params_t *conn_params;
    esp_mqtt_subscription_t *subscriptions[MAX_MQTT_SUBSCRIPTIONS];
} esp_mqtt_data_t;

static esp_mqtt_data_t *mqtt_data;

static void esp_mqtt_subscribe_callback(const char *topic, int topic_len, const char *data, int data_len)
{
    esp_mqtt_subscription_t **subscriptions = mqtt_data->subscriptions;
    int i;
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
        if (subscriptions[i]) {
            if ((strncmp(topic, subscriptions[i]->topic, topic_len) == 0)
                    && (topic_len == strlen(subscriptions[i]->topic))) {
                subscriptions[i]->cb(subscriptions[i]->topic, (void *)data, data_len, subscriptions[i]->priv);
            }
        }
    }
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            /* Resubscribe to all topics after reconnection */
            for (int i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
                if (mqtt_data->subscriptions[i]) {
                    esp_mqtt_client_subscribe(event->client, mqtt_data->subscriptions[i]->topic, 1);
                }
            }            
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, TOPIC = %s", event->topic);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            esp_mqtt_subscribe_callback(event->topic, event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t esp_mqtt_add_topic_to_subscribe_list(const char *topic, esp_mqtt_subscribe_cb_t cb, uint8_t qos, void *priv_data)
{
    if ( !mqtt_data || !topic || !cb) {
        return ESP_FAIL;
    }

    // check duplicate topic
    for(int i=0;i<MAX_MQTT_SUBSCRIPTIONS;i++)
    {
        if (mqtt_data->subscriptions[i]) {
            if(strcmp(topic, mqtt_data->subscriptions[i]->topic) == 0){
                ESP_LOGW(TAG, "Duplicate topic: %s", topic);
                return ESP_OK;
            }
        }
    }

    int i;
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
        if (!mqtt_data->subscriptions[i]) {
            esp_mqtt_subscription_t *subscription = calloc(1, sizeof(esp_mqtt_subscription_t));
            if (!subscription) {
                return ESP_FAIL;
            }
            subscription->topic = strdup(topic);
            if (!subscription->topic) {
                free(subscription);
                return ESP_FAIL;
            }
            subscription->priv = priv_data;
            subscription->cb = cb;
            mqtt_data->subscriptions[i] = subscription;
            ESP_LOGW(TAG, "Subscribed to topic: %s", topic);
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t esp_mqtt_subscribe(const char *topic, esp_mqtt_subscribe_cb_t cb, uint8_t qos, void *priv_data)
{
    if ( !mqtt_data || !topic || !cb) {
        return ESP_FAIL;
    }

    // check duplicate topic
    for(int i=0;i<MAX_MQTT_SUBSCRIPTIONS;i++)
    {
        if (mqtt_data->subscriptions[i]) {
            if(strcmp(topic, mqtt_data->subscriptions[i]->topic) == 0){
                ESP_LOGW(TAG, "Duplicate topic: %s", topic);
                return ESP_OK;
            }
        }
    }

    int i;
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
        if (!mqtt_data->subscriptions[i]) {
            esp_mqtt_subscription_t *subscription = calloc(1, sizeof(esp_mqtt_subscription_t));
            if (!subscription) {
                return ESP_FAIL;
            }
            subscription->topic = strdup(topic);
            if (!subscription->topic) {
                free(subscription);
                return ESP_FAIL;
            }
            int ret = esp_mqtt_client_subscribe(mqtt_data->mqtt_client, subscription->topic, qos);
            if (ret < 0) {
                free(subscription->topic);
                free(subscription);
                return ESP_FAIL;
            }
            subscription->priv = priv_data;
            subscription->cb = cb;
            mqtt_data->subscriptions[i] = subscription;
            ESP_LOGW(TAG, "Subscribed to topic: %s", topic);
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t esp_mqtt_unsubscribe(const char *topic)
{
    if (!mqtt_data || !topic) {
        return ESP_FAIL;
    }
    int ret = esp_mqtt_client_unsubscribe(mqtt_data->mqtt_client, topic);
    if (ret < 0) {
        ESP_LOGW(TAG, "Could not unsubscribe from topic: %s", topic);
    }
    esp_mqtt_subscription_t **subscriptions = mqtt_data->subscriptions;
    int i;
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
        if (subscriptions[i]) {
            if (strncmp(topic, subscriptions[i]->topic, strlen(topic)) == 0) {
                free(subscriptions[i]->topic);
                free(subscriptions[i]);
                subscriptions[i] = NULL;
                return ESP_OK;
            }
        }
    }
    return ESP_FAIL;
}

esp_err_t esp_mqtt_publish(const char *topic, void *data, size_t data_len, uint8_t qos, int *msg_id)
{
    if (!mqtt_data || !topic || !data) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Publishing to %s", topic);
    int ret = esp_mqtt_client_publish(mqtt_data->mqtt_client, topic, data, data_len, qos, 0);
    if (ret < 0) {
        ESP_LOGE(TAG, "MQTT Publish failed");
        return ESP_FAIL;
    }
    if (msg_id) {
        *msg_id = ret;
    }
    return ESP_OK;
}

esp_err_t esp_mqtt_connect(void)
{
    if (!mqtt_data) {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Connecting to %s", mqtt_data->conn_params->mqtt_host);
    esp_err_t ret = esp_mqtt_client_start(mqtt_data->mqtt_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_mqtt_client_start() failed with err = %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Waiting for MQTT connection. This may take time.");
    return ESP_OK;
}

static void esp_mqtt_unsubscribe_all(void)
{
    if (!mqtt_data) {
        return;
    }
    int i;
    for (i = 0; i < MAX_MQTT_SUBSCRIPTIONS; i++) {
        if (mqtt_data->subscriptions[i]) {
            esp_mqtt_unsubscribe(mqtt_data->subscriptions[i]->topic);
        }
    }
}

esp_err_t esp_mqtt_disconnect(void)
{
    if (!mqtt_data) {
        return ESP_FAIL;
    }
    esp_mqtt_unsubscribe_all();
    esp_err_t err = esp_mqtt_client_stop(mqtt_data->mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to disconnect from MQTT");
    } else {
        ESP_LOGI(TAG, "MQTT Disconnected.");
    }
    return err;
}

esp_err_t esp_mqtt_init(esp_mqtt_conn_params_t *conn_params)
{
    if (mqtt_data) {
        ESP_LOGE(TAG, "MQTT already initialized");
        return ESP_OK;
    }
    if (!conn_params) {
        ESP_LOGE(TAG, "Connection params are mandatory for esp_mqtt_init");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Initialising MQTT");
    mqtt_data = calloc(1, sizeof(esp_mqtt_data_t));
    if (!mqtt_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for esp_mqtt_data_t");
        return ESP_ERR_NO_MEM;
    }
    mqtt_data->conn_params = conn_params;
    const esp_mqtt_client_config_t mqtt_client_cfg = {
        .host = conn_params->mqtt_host,
        .port = 1883,
        .client_id = (const char *)conn_params->client_id,
        .keepalive = 120,
        .event_handle = mqtt_event_handler,
        .transport = MQTT_TRANSPORT_OVER_TCP,
#ifdef  ESP_LASS_WILL_MSG_USE
        .lwt_topic = conn_params->lwt_topic,                /*!< LWT (Last Will and Testament) message topic (NULL by default) */
        .lwt_msg = conn_params->lwt_msg,                    /*!< LWT message (NULL by default) */
        .lwt_qos = conn_params->lwt_qos,                            /*!< LWT message qos */
        .lwt_retain = conn_params->lwt_retain,                         /*!< LWT retained message flag */
        .lwt_msg_len = conn_params->lwt_msg_len,                        /*!< LWT message length */
        .keepalive = conn_params->keepalive,
#endif
#ifdef CONFIG_ESP_MQTT_PERSISTENT_SESSION
        .disable_clean_session = 1,
#endif /* CONFIG_ESP_MQTT_PERSISTENT_SESSION */
    };
    mqtt_data->mqtt_client = esp_mqtt_client_init(&mqtt_client_cfg);
    return ESP_OK;
}

