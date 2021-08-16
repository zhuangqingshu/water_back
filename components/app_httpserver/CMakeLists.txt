
idf_component_register(SRCS "user_httpserver.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_http_server http_parser app_event)