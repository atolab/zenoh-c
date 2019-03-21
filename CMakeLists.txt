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
        "${LIB_UV_PATH}/include"
        "${LIB_CRITERION_PATH}/include"
)

link_directories(
        "/usr/local/Cellar/libuv/1.15.0/lib"
        "/usr/local/Cellar/criterion/2.3.2/lib"
)

link_libraries(
        "uv"
        "criterion"
)


file(GLOB Sources "src/*.c")

# file(GLOB Tests "test/test_buffer.cxx" "test/test_locator.cxx")
#$add_executable(zenoh app/broker.cpp ${Sources} include/zenoh/stack.hxx include/zenoh/bound_stack.hxx)

# $add_executable(client app/client.cpp ${Sources} include/zenoh/stack.hxx include/zenoh/functional.hxx include/zenoh/bound_stack.hxx)

#$add_executable(run-test ${Tests} ${Sources} test/test_stack.cxx include/zenoh/bound_stack.hxx)

add_library(zenohc ${Sources})

install (TARGETS zenohc DESTINATION ${PROJECT_SOURCE_DIR}/bin)
