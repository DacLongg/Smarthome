idf_component_register(SRCS "mainApp.c"
                       INCLUDE_DIRS "."
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../Espressif/frameworks/esp-idf-v5.2.1/components/freertos/FreeRTOS-Kernel-SMP/include/freertos"
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_door_lock/user/device/lcd"
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_door_lock/user/device/button"
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_door_lock/user/device/fingerprint"
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_door_lock/user/device/wifi"
                       INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../smart_door_lock/user/device/password"
                       )
