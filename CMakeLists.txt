cmake_minimum_required(VERSION 3.9)

project(zenoh)
set (zenoh_c_VERSION_MAJOR 0)
set (zenoh_c_VERSION_MINOR 1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Use cmake .. -DCMAKE_BUILD_TYPE=Debug for debug / Release
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

option (UNIX_TARGET "Use this for Linux, Macos, etc." ON)

set(CMAKE_C_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wunused -pipe -g")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wunused -pipe -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wunused -pipe -g")

include_directories(
        "${PROJECT_SOURCE_DIR}/include"                
)

if (UNIX_TARGET)
file(GLOB Sources "src/*.c" "src/unix/*.c")

endif(UNIX_TARGET)

link_directories("${PROJECT_SOURCE_DIR}/build")

add_library(zenohc SHARED ${Sources})
target_link_libraries(zenohc Threads::Threads)

target_link_libraries(zenohc)
add_executable(z_pub example/z_pub.c)
add_executable(z_sub example/z_sub.c)
add_executable(z_pub_thr example/z_pub_thr.c)
add_executable(z_sub_thr example/z_sub_thr.c)

add_executable(z_data_struct example/z_data_struct.c)
# target_link_directories(driver "${PROJECT_SOURCE_DIR}/build")

target_link_libraries(z_pub zenohc)
target_link_libraries(z_sub zenohc)
target_link_libraries(z_pub_thr zenohc)
target_link_libraries(z_sub_thr zenohc)
target_link_libraries(z_data_struct zenohc)

install (TARGETS zenohc  DESTINATION ${PROJECT_SOURCE_DIR}/lib)
install (TARGETS z_pub  DESTINATION ${PROJECT_SOURCE_DIR}/bin)
