#include "app_nvs_flash.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"
#include "esp_err.h"

nvs_handle nvs_handle_user;
static uint8_t mode = 0;

#define NAME   "__nvs_user"
#define KEY   "provision_mode"
#define KEY_HOMEID  "homeid"
#define KEY_USERID    "userid"

void app_flash_switch_mode_provision(void)
{
    nvs_open(NAME, NVS_READWRITE, &nvs_handle_user);
    nvs_get_u8(nvs_handle_user,KEY,&mode);
    printf("get mode = %d\n", mode);
    if(mode == 0 || mode == 1)
        mode = 1 - mode;
    else
    {
        mode = 0;
    }
    printf("set mode = %d\n", mode);
    nvs_set_u8(nvs_handle_user,KEY,mode);
}

uint8_t app_flash_get_mode_provision(void)
{
    nvs_open(NAME, NVS_READONLY, &nvs_handle_user);
    nvs_get_u8(nvs_handle_user,KEY,&mode);    
    printf("get mode = %d\n", mode);
    return mode;
}

static void app_flash_get_homeid(char *homeid)
{
    size_t length;
    nvs_open(NAME, NVS_READONLY, &nvs_handle_user);
    nvs_get_str(nvs_handle_user,KEY_HOMEID, NULL, &length);
    nvs_get_str(nvs_handle_user,KEY_HOMEID, homeid, &length);
    printf("get homeid: %s", homeid);  
}

static void app_flash_get_userid(char *userid)
{
    size_t length;
    nvs_open(NAME, NVS_READONLY, &nvs_handle_user);
    nvs_get_str(nvs_handle_user,KEY_USERID, NULL, &length);
    nvs_get_str(nvs_handle_user,KEY_USERID, userid, &length);
    printf("get userid: %s", userid);  
}

static void app_flash_get_clientid(char *clientid)
{
    const esp_partition_t* partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "clientid");
    assert(partition);
    ESP_ERROR_CHECK(esp_partition_read(partition, 0, clientid, 15));
    printf("get clientid: %s", clientid);  
}


void app_flash_set_homeid(char *homeid)
{
    nvs_open(NAME, NVS_READWRITE, &nvs_handle_user);
    nvs_set_str(nvs_handle_user, KEY_HOMEID, homeid);
}

void app_flash_set_userid(char *userid)
{
    nvs_open(NAME, NVS_READWRITE, &nvs_handle_user);
    nvs_set_str(nvs_handle_user, KEY_USERID, userid);
}

void app_flash_erase_all(void)
{
    ESP_ERROR_CHECK(nvs_flash_erase());
}

void app_flash_get_device_info(char *clientid, char *userid, char *homeid)
{
    // app_flash_get_userid(userid);
    // app_flash_get_homeid(homeid);
    app_flash_get_clientid(clientid);
}