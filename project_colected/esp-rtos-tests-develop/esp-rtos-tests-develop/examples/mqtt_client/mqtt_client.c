#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include <semphr.h>

#define MQTT_0LEN_CLIENT_ID
#define MQTT_CLEANSESSION

SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;
#define PUB_MSG_LEN 16

#define RGB_MSG_LEN 12

static void  beat_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char msg[PUB_MSG_LEN];
    int count = 0;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, 10000 / portTICK_PERIOD_MS); // == 10 seconds
        printf("beat\r\n");
        snprintf(msg, PUB_MSG_LEN, "Beat %d\r\n", count++);
        if (xQueueSend(publish_queue, (void *)msg, 0) == pdFALSE) {
            printf("Publish queue overflow.\r\n");
        }
    }
}

static char rgb[3] =  { 'r', 'g', 'b' };

/* RBG msg format: r:_g:_b_! */
static void  topic_received(mqtt_message_data_t *md)
{
   int i, parse_loc;
   uint8_t * rgb_colors = NULL;
   mqtt_message_t *message = md->message;
   printf("Received: ");
   if (md->topic->lenstring.len != RGB_MSG_LEN)
   {
      printf("unkown msg len = %d\n", md->topic->lenstring.len);
      return;
   }
   // memcpy(&prev_temp, &latest_temp, sizeof(Temperature));
   // printf("new temp = %c%d.%02d deg.C\n", latest_temp.sign, latest_temp.val, latest_temp.fract);
   // int ret = sprintf(buf, "%c%d.%02d", latest_temp.sign, latest_temp.val, latest_temp.fract);


   parse_loc = 0;

   uint8_t rgb_read[3];

   // RGB_MSG_LEN
   for( i = 0; i < md->topic->lenstring.len; i++)
   {
      char * next_ch = (char*) &md->topic->lenstring.data[ i ];

      switch (rgb[ parse_loc ])
      {
         case 'r': {
            if ( (*next_ch == 'r') && ( (*(next_ch+1) == ':') )
            {
               rgb_read[++parse_loc] = *(next_ch+2);
               i += 2;
               printf("found 'r'. read = %d\n", rgb_read[0]);
            } else {
               printf("?? next char = %c\n", *next_ch);
               return;
            }
         }
      }
   }
   // printf("%c", md->topic->lenstring.data[ i ]);

   // printf(" = ");
   // for( i = 0; i < (int)message->payloadlen; ++i)
   //    printf("%c", ((char *)(message->payload))[i]);

   printf("\r\n");
}

static const char *  get_my_id(void)
{
    // Use MAC address for Station as unique ID
    static char my_id[13];
    static bool my_id_done = false;
    int8_t i;
    uint8_t x;
    if (my_id_done)
        return my_id;
    if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)my_id))
        return NULL;
    for (i = 5; i >= 0; --i)
    {
        x = my_id[i] & 0x0F;
        if (x > 9) x += 7;
        my_id[i * 2 + 1] = x + '0';
        x = my_id[i] >> 4;
        if (x > 9) x += 7;
        my_id[i * 2] = x + '0';
    }
    my_id[12] = '\0';
    my_id_done = true;
    return my_id;
}

static void  mqtt_task(void *pvParameters)
{
   int ret         = 0;
   struct mqtt_network network;
   mqtt_client_t client   = mqtt_client_default;

#ifdef MQTT_0LEN_CLIENT_ID
   char mqtt_client_id[2] = { 0, 0 };
#else
   char mqtt_client_id[20];
#endif
   uint8_t mqtt_buf[100];
   uint8_t mqtt_readbuf[100];
   mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

   mqtt_network_new( &network );
#ifndef MQTT_0LEN_CLIENT_ID
   memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
   strcpy(mqtt_client_id, "ESP-");
   strcat(mqtt_client_id, get_my_id());
#endif

   while(1)
   {
      xSemaphoreTake(wifi_alive, portMAX_DELAY);
      printf("%s: started\n\r", __func__);
      printf("%s: (Re)connecting to MQTT server %s ... ",__func__,
      MQTT_HOST);
      ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
      if( ret )
      {
         printf("error: %d\n\r", ret);
         taskYIELD();
         continue;
      }
      printf("done\n\r");
      mqtt_client_new(&client, &network, 5000, mqtt_buf, 100, mqtt_readbuf, 100);

      data.willFlag       = 0;
      data.MQTTVersion    = 4;
      data.clientID.cstring   = mqtt_client_id;
      data.username.cstring   = MQTT_USER;
      data.password.cstring   = MQTT_PASS;
      data.keepAliveInterval  = 10;
   #ifdef MQTT_CLEANSESSION
      data.cleansession   = 1;
   #else
      data.cleansession   = 0;
   #endif
      printf("Send MQTT connect ... ");
      ret = mqtt_connect(&client, &data);

      if(ret)
      {
         printf("error: %d\n\r", ret);
         mqtt_network_disconnect(&network);
         taskYIELD();
         continue;
      }
      printf("done\r\n");
      mqtt_subscribe(&client, "/cpe439/rgb", MQTT_QOS1, topic_received);
      xQueueReset(publish_queue);

      while(1)
      {
         char msg[PUB_MSG_LEN - 1] = "\0";
         while (xQueueReceive(publish_queue, (void *)msg, 0) == pdTRUE)
         {
            printf("got message to publish\r\n");
            mqtt_message_t message;
            message.payload = msg;
            message.payloadlen = PUB_MSG_LEN;
            message.dup = 0;
            message.qos = MQTT_QOS1;
            message.retained = 0;
            ret = mqtt_publish(&client, "/cpe439/temp", &message);
            if (ret != MQTT_SUCCESS )
            {
               printf("error while publishing message: %d\n", ret );
               break;
            }
         }
         ret = mqtt_yield(&client, 1000);
         if (ret == MQTT_DISCONNECTED)
            break;
      }

      printf("Connection dropped, request restart\n\r");
      mqtt_network_disconnect(&network);
      taskYIELD();
   }
}

static void  wifi_task(void *pvParameters)
{
    uint8_t status  = 0;
    uint8_t retries = 30;
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    printf("WiFi: connecting to WiFi\n\r");
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    while(1)
    {
        while ((status != STATION_GOT_IP) && (retries)){
            status = sdk_wifi_station_get_connect_status();
            printf("%s: status = %d\n\r", __func__, status );
            if( status == STATION_WRONG_PASSWORD ){
                printf("WiFi: wrong password\n\r");
                break;
            } else if( status == STATION_NO_AP_FOUND ) {
                printf("WiFi: AP not found\n\r");
                break;
            } else if( status == STATION_CONNECT_FAIL ) {
                printf("WiFi: connection failed\r\n");
                break;
            }
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            --retries;
        }
        if (status == STATION_GOT_IP) {
            printf("WiFi: Connected\n\r");
            xSemaphoreGive( wifi_alive );
            taskYIELD();
        }

        while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
            xSemaphoreGive( wifi_alive );
            taskYIELD();
        }
        printf("WiFi: disconnected\n\r");
        sdk_wifi_station_disconnect();
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

void user_init(void)
{
   uart_set_baud(0, BAUD_RATE);

   printf("mqtt_client\n");
   printf("SDK version:%s\n", sdk_system_get_sdk_version());

   vSemaphoreCreateBinary(wifi_alive);
   publish_queue = xQueueCreate(3, PUB_MSG_LEN);
   xTaskCreate(&wifi_task, "wifi_task",  256, NULL, 2, NULL);
   xTaskCreate(&beat_task, "beat_task", 256, NULL, 3, NULL);
   xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);
}
