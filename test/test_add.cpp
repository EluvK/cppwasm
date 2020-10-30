#include "../wasi-runtime.h"

#include <gtest/gtest.h>

TEST(test_, add_1) {
    const char * file_path{"../example/add.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, bool>> imps;
    Runtime runtime{mod, imps};
    runtime.exec("add", {1,2});
}

TEST(test_, fib_1) {
    const char * file_path{"../example/fib.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, bool>> imps;
    Runtime runtime{mod, imps};
    runtime.exec("fib", {20});
}