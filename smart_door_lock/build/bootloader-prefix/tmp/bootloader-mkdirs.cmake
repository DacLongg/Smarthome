# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/App/ESP32/Espressif/frameworks/esp-idf-v5.2.1/components/bootloader/subproject"
  "D:/App/ESP32/smart_door_lock/build/bootloader"
  "D:/App/ESP32/smart_door_lock/build/bootloader-prefix"
  "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/tmp"
  "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/src/bootloader-stamp"
  "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/src"
  "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/App/ESP32/smart_door_lock/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
