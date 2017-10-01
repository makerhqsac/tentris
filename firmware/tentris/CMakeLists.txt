cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME tentris)
project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH tentris.ino)

link_directories($ENV{HOME}/Documents/Arduino/libraries)
link_directories($ENV{HOME}/Arduino/libraries)


#### Uncomment below additional settings as needed.
set(${CMAKE_PROJECT_NAME}_BOARD diecimila)
set(${CMAKE_PROJECT_NAME}_PORT /dev/ttyUSB0)
set(diecimila.build.mcu atmega328p)
set(${TARGET_NAME}_PROGRAMMER avrispmkii)
# set(mega.upload.protocol wiring)
set(diecimila.upload.speed 57600)

generate_arduino_firmware(${CMAKE_PROJECT_NAME})
