cmake_minimum_required(VERSION 3.8)

aux_source_directory(./src wasi_src)

# add_library(wasi STATIC ${wasi_src})
add_library(wasi STATIC ${wasi_src})