
idf_component_register(SRCS "tm_onejson.c" "dev_token.c" "tm_data.c" "tm_mqtt.c" "tm_api.c" "time_linux.c" "tm_user.c" 
                    INCLUDE_DIRS "include"
                    REQUIRES "esp-wolfssl" "json" "mbedtls" "mqtt" "app_water_back" "app_ota")