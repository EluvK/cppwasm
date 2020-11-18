#include "../wasi-runtime.h"

#include <gtest/gtest.h>

TEST(test_, struct_1) {
    const char * file_path{"../example/struct.wasm"};

    Module mod{file_path};

    std::map<std::string, std::map<std::string, imp_variant>>  imps;

    Runtime runtime{mod, imps};
    
    int32_t test_a = 1234;
    float test_f =9.9;
    runtime.exec("set", {test_a, test_f});
    EXPECT_EQ(runtime.exec("geta",{}).data[0].to_i32(),test_a);
    EXPECT_EQ(runtime.exec("getf",{}).data[0].to_f32(),test_f);
    
}