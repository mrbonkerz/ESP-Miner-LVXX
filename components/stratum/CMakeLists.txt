idf_component_register(
SRCS
    "utils.c"
    "mining.c"
   
                    
INCLUDE_DIRS
    "include"

REQUIRES
    "json"
    "mbedtls"
    "app_update"
    "esp_timer"
)