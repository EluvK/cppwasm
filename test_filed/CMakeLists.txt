cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
# add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)
add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -w -fstack-protector-all -march=native -fnon-call-exceptions)

# project(wasi-test)
aux_source_directory(. wasi_test_on_work_src)

add_executable(wasi_test_on_work ${wasi_test_on_work_src})

add_dependencies(wasi_test_on_work wasi)
target_link_libraries(wasi_test_on_work PRIVATE wasi gtest gtest_main pthread)


# include_directories(.)

# include_directories(metrics)

# add_subdirectory(cppwasm)
