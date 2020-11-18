#include "../wasi-runtime.h"

#include <gtest/gtest.h>

#if 0
int cfib(int n) {
    if (n <= 1) {
        return n;
    }
    return cfib(n - 1) + cfib(n - 2);
}

// fib(10)
// c: 0ms
// pywasm: 54ms
// cppwasm: 12ms

// fib(20)
// c:0ms
// pywasm: 6346ms
// cppwasm: 1337ms

// update: 11.16
// fib(20)
// cppwasm: 970ms
// fib(30)
// c: 7ms
// cppwasm: 119269 ms 2min.

TEST(test_, fib_standard) {
    // EXPECT_EQ(cfib(20), 6765);
    EXPECT_EQ(cfib(30), 832040);
}

TEST(test_, fib_benchmark) {
    const char * file_path{"../example/fib.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, imp_variant>>  imps;
    Runtime runtime{mod, imps};
    ASSERT_EQ(runtime.exec("fib", {30}).data[0].to_i64(), 832040);
}
#endif