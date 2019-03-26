cmake_minimum_required(VERSION 3.9)
project(zenoh)
set (zenoh_c_VERSION_MAJOR 0)
set (zenoh_c_VERSION_MINOR 1)

set(CMAKE_C_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wunused -pipe -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wunused -pipe -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wunused -pipe -g")

include_directories(
        "${PROJECT_SOURCE_DIR}/include"        
)

file(GLOB Sources "src/*.c")
link_directories("${PROJECT_SOURCE_DIR}/build")

add_library(zenohc SHARED ${Sources})

add_executable(driver example/driver.c)
# target_link_directories(driver "${PROJECT_SOURCE_DIR}/build")

target_link_libraries(driver zenohc)

install (TARGETS zenohc  DESTINATION ${PROJECT_SOURCE_DIR}/lib)
install (TARGETS driver  DESTINATION ${PROJECT_SOURCE_DIR}/bin)
