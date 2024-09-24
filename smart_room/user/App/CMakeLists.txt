set(app_src mainApp.c sesion_communication.c)
set(pri_req)
idf_component_register(SRCS ${app_src}
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/esp_wifi/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/lcd"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/mq2"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/dht22"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/user_mqtt"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/sim"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/mqtt/esp-mqtt/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/gpio/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/i2c/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/freertos/FreeRTOS-Kernel-SMP/include/freertos"
                        INCLUDE_DIRS "."
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_ringbuf/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_netif/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_partition/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/nvs_flash/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/driver/uart/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/esp_event/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/tcp_transport/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/hal/include"
                        # INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/hal/include/hal"
                        PRIV_REQUIRES ${pri_req}
                        )