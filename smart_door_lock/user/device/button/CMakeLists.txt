set(app_src button.c)
set(pri_req)
idf_component_register(SRCS ${app_src}
                        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/driver/gpio/include"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES ${pri_req}
                        )