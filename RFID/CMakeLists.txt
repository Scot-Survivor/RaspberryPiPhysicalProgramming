cmake_minimum_required(VERSION 3.16.3)
project(RFID-Reader)

set(CMAKE_PREFIX_PATH .)

find_package(WiringPi REQUIRED)
find_package(Threads REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${WIRINGPI_INCLUDE_DIRS})

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/dump/dump.c ${CMAKE_CURRENT_SOURCE_DIR}/src/AES_128/TI_aes_128.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mfrc522/mfrc522.c ${CMAKE_CURRENT_SOURCE_DIR}/src/mfrc522/mfrc522_debug.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mfrc522/mfrc522_hal_linux.c)

add_executable(${PROJECT_NAME} main.c ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${WIRINGPI_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
