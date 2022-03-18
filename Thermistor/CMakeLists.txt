cmake_minimum_required(VERSION 3.16.3)
project(Thermistor)

set(CMAKE_PREFIX_PATH .)

find_package(WiringPi REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS
        iostreams system filesystem
        )

configure_file(CMakeLists.txt.in gnuplot-iostream-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gnuplot-iostream-download)
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gnuplot-iostream-download)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/gnuplot-iostream-src/)
include_directories(${WIRINGPI_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})


add_library(ADCLib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/ADCDevice.cpp)


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ADCLib ${WIRINGPI_LIBRARIES} ${THREADS_LIBRARIES} gnuplot_iostream
        ${Boost_LIBRARIES})
