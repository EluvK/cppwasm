cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
# add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)
add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -w -fstack-protector-all -march=native -fnon-call-exceptions)
# add_compile_options(-std=c++11 -pthread -O3 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)

project(cppwasm)

include_directories(.)
include_directories(cppwasm/base)
include_directories(cppwasm)
# include_directories(metrics)

add_subdirectory(cppwasm)
add_subdirectory(test)
add_subdirectory(test_filed)
add_subdirectory(test_mvp)
add_subdirectory(analyzer)
