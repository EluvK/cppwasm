#include "../wasi-execution.h"

#include <gtest/gtest.h>

#include <fstream>

byte_vec read_file(const char * file_path) {
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
    xdbg("file:%s size: %d", file_path, bytes_size);
    int cnt = 0;
    for (auto index = 0; index < bytes_size; ++index) {
        data.push_back(*(bytes + index));
        // std::printf("%02x ", *(bytes + index));
        // fflush(stdout);
        // if (++cnt == 4) {
            // std::printf("\n");fflush(stdout);
            // cnt = 0;
        // }
    }
    in.close();
    return data;
}

TEST(test_, module_1) {
    const char * file_path{"../example/ext.wasm"};

    auto data = read_file(file_path);

    Module mod{byte_IO(data)};
}