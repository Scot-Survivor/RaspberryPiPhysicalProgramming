cmake_minimum_required(VERSION 3.16.3)
project(Thermistor)

set(CMAKE_PREFIX_PATH .)

find_package(WiringPi REQUIRED)
find_package(Threads REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${WIRINGPI_INCLUDE_DIRS})


add_library(ADCLib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ADCDevice.cpp)


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ADCLib ${WIRINGPI_LIBRARIES} ${THREADS_LIBRARIES})
