#include <stdint.h>
#include <esp_err.h>

#define MQTT_QOS0    0
#define MQTT_QOS1    1

typedef void (*esp_mqtt_subscribe_cb_t)(const char *topic, void *payload, size_t payload_len, void *priv_data);

/** MQTT Connection parameters */
typedef struct {
    /** MQTT Host */
    char *mqtt_host;
    /** Client ID */
    char *client_id;
    /** Client Certificate in NULL terminated PEM format */
    char *client_cert;
    /** Client Key in NULL terminated PEM format */
    char *client_key;
    /** Server Certificate in NULL terminated PEM format */
    char *server_cert;
    /*!< LWT (Last Will and Testament) message topic (NULL by default) */
    const char *lwt_topic;                  
    /*!< LWT message (NULL by default) */
    const char *lwt_msg;                    
    /*!< LWT message qos */
    int lwt_qos;                            
    /*!< LWT retained message flag */
    int lwt_retain;                        
    /*!< LWT message length */
    int lwt_msg_len;       
    /*!< mqtt keepalive, default is 120 seconds */         
    int keepalive;                                  
} esp_mqtt_conn_params_t;

esp_err_t esp_mqtt_init(esp_mqtt_conn_params_t *conn_params);
esp_err_t esp_mqtt_disconnect(void);
esp_err_t esp_mqtt_connect(void);
esp_err_t esp_mqtt_publish(const char *topic, void *data, size_t data_len, uint8_t qos, int *msg_id);
esp_err_t esp_mqtt_unsubscribe(const char *topic);
esp_err_t esp_mqtt_subscribe(const char *topic, esp_mqtt_subscribe_cb_t cb, uint8_t qos, void *priv_data);
esp_err_t esp_mqtt_add_topic_to_subscribe_list(const char *topic, esp_mqtt_subscribe_cb_t cb, uint8_t qos, void *priv_data);

