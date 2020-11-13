cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
# add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)
add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -w -fstack-protector-all -march=native -fnon-call-exceptions)

# project(wasi-test)
aux_source_directory(. cppwasm_mvp_test_src)

add_executable(cppwasm_mvp_test ${cppwasm_mvp_test_src})

add_dependencies(cppwasm_mvp_test wasi)
target_link_libraries(cppwasm_mvp_test PRIVATE wasi gtest gtest_main pthread)

