#include "../wasi-runtime.h"

#include <gtest/gtest.h>

TEST(test_, add_1) {
    const char * file_path{"../example/add.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    ASSERT_EQ(runtime.exec("add", {1, 2}).data[0].to_i64(),3);
    ASSERT_EQ(runtime.exec("add", {100, 200}).data[0].to_i64(),300);
}

TEST(test_, fib_1) {
    const char * file_path{"../example/fib.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    ASSERT_EQ(runtime.exec("fib", {10}).data[0].to_i64(), 55);
}

TEST(test_, mul_1) {
    const char * file_path{"../example/mul.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    ASSERT_EQ(runtime.exec("add1", {10}).data[0].to_i32(), 11);
    ASSERT_EQ(runtime.exec("add", {102, 324}).data[0].to_i32(), 426);
    ASSERT_EQ(runtime.exec("add", {-12, 20012}).data[0].to_i32(), 20000);
    ASSERT_EQ(runtime.exec("zero", {}).data[0].to_i32(), 0);
    ASSERT_EQ(runtime.exec("fib", {10}).data[0].to_i32(), 55);
}
