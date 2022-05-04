set(app_src esp_mqtt.c)
set(pri_req mqtt)

idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

