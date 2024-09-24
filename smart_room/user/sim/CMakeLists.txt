set(app_src sim.c)
set(pri_req)
idf_component_register(SRCS ${app_src}
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/esp_wifi/include"
                        INCLUDE_DIRS "."
                        INCLUDE_DIRS "D:/App/ESP32/smart_room/user/App"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_room/user/user_mqtt"
                        INCLUDE_DIRS "D:/App/ESP32/smart_room/user/user_mqtt"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_partition/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/nvs_flash/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/mqtt/esp-mqtt/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/esp_event/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/tcp_transport/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/freertos/FreeRTOS-Kernel-SMP/include/freertos"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_netif/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/driver/uart/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.1.3/components/esp_ringbuf/include"
                        PRIV_REQUIRES ${pri_req}
                        )