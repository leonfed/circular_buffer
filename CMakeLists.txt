cmake_minimum_required(VERSION 3.10)
project(circular_buffer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-pthread -Wall -std=c++14 -pedantic ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address,undefined -D_GLIBCXX_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_COVERAGE "-g --coverage")
set(LINK_FLAGS "-pthread ${LINK_FLAGS}")

include_directories(.)
add_subdirectory(gtest)

add_executable(main main.cpp circular_buffer.h)

add_library(counted counted.h counted.cpp fault_injection.h fault_injection.cpp)

add_executable(circular_buffer_testing circular_buffer_testing.cpp circular_buffer.h)
target_link_libraries(circular_buffer_testing counted gtest)