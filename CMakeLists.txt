cmake_minimum_required(VERSION 3.9)

project(zenoh)
set (zenoh_c_VERSION_MAJOR 0)
set (zenoh_c_VERSION_MINOR 1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option (UNIX_TARGET "Use this for Linux, Macos, etc." ON)
option (CONTIKI "Use this for Contiki Cross compilation." OFF)

# Configure the transport to use with the following variables
set (ZENOH_TRANSPORT_TCP "ON")
set (ZENOH_TRANSPORT_UDP "OFF")
set (ZENOH_TRANSPORT_BLE "OFF")
set (ZENOH_TRANSPORT_I2C "OFF")


# Use cmake .. -DCMAKE_BUILD_TYPE=Debug for debug / Release

# Configure the debug level
#
# ZENOH_DEBUG :
#   - 2 : ERROR + DEBUG
#   - 1 : ERROR
#   - 0 : NOTHING

set(ZENOH_DEBUG_OPT "-DZENOH_DEBUG=1") 

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Configuring on/for Linux")
  set(ZENOH_PLATFORM_OPT "-DZENOH_LINUX=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message(STATUS "Configuring on/for macOS")
  set(ZENOH_PLATFORM_OPT "-DZENOH_MACOS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "Configuring on/for Windows")
  set(ZENOH_PLATFORM_OPT "-DZENOH_WIN=1")
else()
  message(STATUS "Configuring on/for ${CMAKE_SYSTEM_NAME}")
endif()

if (ZENOH_TRANSPORT_TCP STREQUAL "ON")
  set (ZENOH_TRANSPORT_OPT "-DZENOH_TRANSPORT_TCP_IP=1")
endif()
if (ZENOH_TRANSPORT_UDP STREQUAL "ON")
  set (ZENOH_TRANSPORT_OPT "${ZENOH_TRANSPORT_OPT} -DZENOH_TRANSPORT_UDP_IP=1")
endif()
if (ZENOH_TRANSPORT_BLE STREQUAL "ON")
  set (ZENOH_TRANSPORT_OPT "${ZENOH_TRANSPORT_OPT} -DZENOH_TRANSPORT_BLE=1")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)



set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_DEBUG "${ZENOH_DEBUG_OPT} ${ZENOH_PLATFORM_OPT} -Wall -Wunused -pipe -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${ZENOH_DEBUG_OPT} ${ZENOH_PLATFORM_OPT} -Wall -Wunused -pipe -O3")

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
add_executable(y_put example/y_put.c)
add_executable(y_put_thr example/y_put_thr.c)
add_executable(z_wd_pub example/z_wd_pub.c)
add_executable(z_sub example/z_sub.c)
add_executable(y_sub example/y_sub.c)
add_executable(y_sub_thr example/y_sub_thr.c)
add_executable(z_pub_thr example/z_pub_thr.c)
add_executable(z_sub_thr example/z_sub_thr.c)

add_executable(z_data_struct example/z_data_struct.c)

target_link_libraries(z_pub zenohc)
target_link_libraries(y_put zenohc)
target_link_libraries(y_put_thr zenohc)
target_link_libraries(z_wd_pub zenohc)
target_link_libraries(z_sub zenohc)
target_link_libraries(y_sub zenohc)
target_link_libraries(y_sub_thr zenohc)
target_link_libraries(z_pub_thr zenohc)
target_link_libraries(z_sub_thr zenohc)
target_link_libraries(z_data_struct zenohc)

install (TARGETS zenohc  DESTINATION /usr/local/lib)
install (TARGETS z_pub  DESTINATION ${PROJECT_SOURCE_DIR}/bin)
