cmake_minimum_required(VERSION 3.9)

project(zenoh)
set (zenoh_c_VERSION_MAJOR 0)
set (zenoh_c_VERSION_MINOR 1)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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
target_link_libraries(zenohc Threads::Threads)

target_link_libraries(zenohc)
add_executable(z_pub example/z_pub.c)
add_executable(z_data_struct example/z_data_struct.c)
# target_link_directories(driver "${PROJECT_SOURCE_DIR}/build")

target_link_libraries(z_pub zenohc)
target_link_libraries(z_data_struct zenohc)

install (TARGETS zenohc  DESTINATION ${PROJECT_SOURCE_DIR}/lib)
install (TARGETS z_pub  DESTINATION ${PROJECT_SOURCE_DIR}/bin)
