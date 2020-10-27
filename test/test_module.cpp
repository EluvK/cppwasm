#include "../wasi-execution.h"

#include <gtest/gtest.h>
#include <fstream>

TEST(test_, module_1) {
    const char * file_path{"../example/add.wasm"};
    
    unsigned char * bytes;
    uint32_t bytes_size;

    std::ifstream file_size(file_path, std::ifstream::ate | std::ifstream::binary);
    bytes_size = file_size.tellg();
    file_size.close();

    byte_vec data{};
    data.reserve(bytes_size);
    if (bytes_size == -1)
        assert(false);
    std::ifstream in(file_path, std::ifstream::binary);
    bytes = (uint8_t *)malloc(bytes_size);
    in.read(reinterpret_cast<char *>(bytes), bytes_size);
    xdbg("size: %d",bytes_size);
    for(auto index = 0;index<bytes_size;++index){
        data.push_back(*(bytes + index));
        xdbg("byte:%02x", *(bytes + index));
    }
    in.close();

    Module(byte_IO(data));
}