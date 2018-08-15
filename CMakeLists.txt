cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)

#-------------------------------------------------------------------------------
# User configuration
#-------------------------------------------------------------------------------
# Set Arduino sketchbook directory
if(WIN32)
    set(SKETCHBOOK_DIR  C:/Users/$ENV{USERNAME}/Documents/Arduino)
elseif(UNIX)
    set(SKETCHBOOK_DIR  $ENV{HOME}/Arduino)
endif()

# Configure one sketch
set(SKETCH sketches/DHT22/DHT22.ino)

# Configure board, default Arduino UNO
set(BOARD       uno)
#set(BOARD       nano)

if(${BOARD} STREQUAL nano)
    set(nano.upload.maximum_size 30720)
    set(nano.upload.maximum_data_size 2048)
    set(nano.upload.speed 57600)
    set(nano.build.mcu atmega328p)
endif()

# Configure serial port:
if(WIN32)
    set(SERIAL_PORT     COM1)
elseif(UNIX)
    set(SERIAL_PORT     /dev/ttyUSB0)
endif()

#-------------------------------------------------------------------------------
# Project configuation
#-------------------------------------------------------------------------------
# Project name
set(PROJECT_NAME        ESP8266Ubidots)

add_definitions(-DARDUINO_ESP8266_NODEMCU)

# Set library directory
set(TIMESTAMP_SRC_DIR      ${SKETCHBOOK_DIR}/libraries/ErriezTimestamp/src)
set(DHT22_SRC_DIR          ${SKETCHBOOK_DIR}/libraries/ErriezDHT22/src)
set(UBIDOTS_SRC_DIR        ${SKETCHBOOK_DIR}/libraries/ubidots-esp8266)
set(ESP_SRC_DIR            ~/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/cores/esp8266)

# Configure project
project(${PROJECT_NAME})

# Configure include directories
include_directories(${TIMESTAMP_SRC_DIR} ${DHT22_SRC_DIR} ${UBIDOTS_SRC_DIR} ${ESP_SRC_DIR})

# Configure project
set(${CMAKE_PROJECT_NAME}_SKETCH    ${SKETCH})
set(${CMAKE_PROJECT_NAME}_BOARD     ${BOARD})
set(${CMAKE_PROJECT_NAME}_PORT      ${SERIAL_PORT})
generate_arduino_firmware(${CMAKE_PROJECT_NAME})
