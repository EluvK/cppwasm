cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
# add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -Wall -fstack-protector-all -march=native -fnon-call-exceptions)
add_compile_options(-std=c++11 -pthread -g  -ggdb -O0 -w -fstack-protector-all -march=native -fnon-call-exceptions)

aux_source_directory(. wasm_analyzer_src)

add_executable(wasm_analyzer ${wasm_analyzer_src})

add_dependencies(wasm_analyzer wasi)
target_link_libraries(wasm_analyzer PRIVATE wasi)

