#
# Copyright (c) 2017, 2020 ADLINK Technology Inc.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
# which is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
#
# Contributors:
#   ADLINK zenoh team, <zenoh@adlink-labs.tech>
#
cmake_minimum_required(VERSION 3.0)

project(zenoh)
set (zenoh_c_VERSION_MAJOR 0)
set (zenoh_c_VERSION_MINOR 1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option (UNIX_TARGET "Use this for Linux, Macos, etc." ON)
option (CONTIKI "Use this for Contiki Cross compilation." OFF)
option (TESTS "Use this to also build the tests." ON)
option (EXAMPLES "Use this to also build the examples." ON)
option (SWIG_JAVA "Use this to build the zenohc_java library, assuming the Swig generated C files in zenoh-c/src/ ." OFF)

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

if (DEFINED ZENOH_DEBUG)
  set (ZENOH_DEBUG_OPT "-DZENOH_DEBUG=${ZENOH_DEBUG}")
else()
  set(ZENOH_DEBUG_OPT "-DZENOH_DEBUG=1") 
endif() 

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Configuring on/for Linux")
  set(ZENOH_PLATFORM_OPT "-DZENOH_LINUX=1")  
  set(INSTALL_RPATH "/usr/local/lib")
  set(INSTALL_NAME_DIR "/usr/local/lib")
  set(INSTALL_INCLUDE_NAME_DIR "/usr/local/include")
  set(JNI_PLATFORM_NAME "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message(STATUS "Configuring on/for macOS")
  set(ZENOH_PLATFORM_OPT "-DZENOH_MACOS=1")
  set(MACOSX_RPATH "ON")  
  set(INSTALL_NAME_DIR "/usr/local/lib")
  set(INSTALL_INCLUDE_NAME_DIR "/usr/local/include")
  set(JNI_PLATFORM_NAME "darwin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "Configuring on/for Windows")
  set(ZENOH_PLATFORM_OPT "-DZENOH_WIN=1")
  set(JNI_PLATFORM_NAME "win32")
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
set(CMAKE_C_FLAGS_DEBUG "${ZENOH_DEBUG_OPT} ${ZENOH_PLATFORM_OPT} -Wall -Wextra -Werror -Wpedantic -Wunused -pipe -g -O0")
set(CMAKE_C_FLAGS_GCOV "${ZENOH_DEBUG_OPT} ${ZENOH_PLATFORM_OPT} -fprofile-arcs -ftest-coverage -Wall -Wextra -Werror -Wpedantic -Wunused -pipe -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${ZENOH_DEBUG_OPT} ${ZENOH_PLATFORM_OPT} -DNDEBUG -pipe -O3")


if (NOT SWIG_JAVA)
  set(Libname "zenohc")
  file(GLOB PublicHeaders "include/*.h" "include/zenoh/*.h" "include/zenoh/net/*.h")
  include_directories(
      "${PROJECT_SOURCE_DIR}/include"
      "${PROJECT_SOURCE_DIR}/include/net"
    )
else()
  set(Libname "zenohc_java")

  if (NOT JNI_INCLUDE_HOME)
    message(STATUS "Looking for local JDK with JNI")
    find_package(JNI REQUIRED)
  else()
    message(STATUS "Using JNI headers from ${JNI_INCLUDE_HOME}")
    get_filename_component(JNI_INCL_PATH1 "${JNI_INCLUDE_HOME}" ABSOLUTE)
    get_filename_component(JNI_INCL_PATH2 "${JNI_INCLUDE_HOME}/${JNI_PLATFORM_NAME}" ABSOLUTE)
    set(JNI_INCLUDE_DIRS "${JNI_INCL_PATH1};${JNI_INCL_PATH2}")
  endif()
  message(STATUS "Using JNI include paths: ${JNI_INCLUDE_DIRS}")

  include_directories(
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/net"
    "${JNI_INCLUDE_DIRS}"
  )
endif()


if (UNIX_TARGET)
  file(GLOB Sources "src/*.c" "src/net/*.c" "src/net/unix/*.c")
endif()

link_directories("${PROJECT_SOURCE_DIR}/build")

add_library(${Libname} SHARED ${Sources})

if (UNIX_TARGET)
  target_link_libraries(${Libname} pthread)
endif()

target_link_libraries(${Libname})

if (NOT SWIG_JAVA)
  install (TARGETS ${Libname}  DESTINATION ${INSTALL_NAME_DIR})
  # FILE (GLOB_RECURSE INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
   install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/zenoh.h DESTINATION ${INSTALL_INCLUDE_NAME_DIR})
   install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/zenoh DESTINATION ${INSTALL_INCLUDE_NAME_DIR})
  endif()


if (EXAMPLES)
  add_executable(zn_write example/net/zn_write.c)
  add_executable(zn_stream example/net/zn_stream.c)
  add_executable(zn_sub example/net/zn_sub.c)
  add_executable(zn_pull example/net/zn_pull.c)
  add_executable(zn_query example/net/zn_query.c)
  add_executable(zn_storage example/net/zn_storage.c)
  add_executable(zn_eval example/net/zn_eval.c)
  add_executable(zn_info example/net/zn_info.c)
  add_executable(zn_pub_thr example/net/zn_pub_thr.c)
  add_executable(zn_sub_thr example/net/zn_sub_thr.c)
  add_executable(zn_scout example/net/zn_scout.c)

  target_link_libraries(zn_write ${Libname})
  target_link_libraries(zn_stream ${Libname})
  target_link_libraries(zn_sub ${Libname})
  target_link_libraries(zn_pull ${Libname})
  target_link_libraries(zn_query ${Libname})
  target_link_libraries(zn_storage ${Libname})
  target_link_libraries(zn_eval ${Libname})
  target_link_libraries(zn_info ${Libname})
  target_link_libraries(zn_pub_thr ${Libname})
  target_link_libraries(zn_sub_thr ${Libname})
  target_link_libraries(zn_scout ${Libname})
endif(EXAMPLES)

if (TESTS)
  add_executable(z_data_struct test/z_data_struct.c)
  add_executable(z_mvar_test test/z_mvar_test.c)
  add_executable(z_rname_test test/z_rname_test.c)
  add_executable(zn_client_test test/zn_client_test.c)
  add_executable(zn_large_data_test test/zn_large_data_test.c)

  target_link_libraries(z_data_struct ${Libname})
  target_link_libraries(z_mvar_test ${Libname})
  target_link_libraries(z_rname_test ${Libname})
  target_link_libraries(zn_client_test ${Libname})
  target_link_libraries(zn_large_data_test ${Libname})

  configure_file(test/routed.sh routed.sh COPYONLY)

  enable_testing()
  add_test(z_data_struct z_data_struct)
  add_test(z_mvar_test z_mvar_test)
  add_test(z_rname_test z_rname_test)
  add_test(zn_client_test bash routed.sh zn_client_test)
  add_test(zn_large_data_test bash routed.sh zn_large_data_test)
endif(TESTS)
