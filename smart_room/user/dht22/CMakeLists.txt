set(app_src dht22.c)
set(pri_req)
idf_component_register(SRCS ${app_src}
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/gpio/include"
                        INCLUDE_DIRS "."
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/deprecated"
                        INCLUDE_DIRS "../lcd"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/i2c/include"
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/freertos/FreeRTOS-Kernel-SMP/include/freertos"
                        PRIV_REQUIRES ${pri_req}
                        )