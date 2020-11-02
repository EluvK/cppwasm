#include "../wasi-runtime.h"

#include <gtest/gtest.h>

TEST(test_, add_1) {
    const char * file_path{"../example/add.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    runtime.exec("add", {1, 2});
}

TEST(test_, fib_1) {
    const char * file_path{"../example/fib.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    runtime.exec("fib", {10});
}

TEST(test_, mul_1) {
    const char * file_path{"../example/mul.wasm"};

    Module mod{file_path};
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    Runtime runtime{mod, imps};
    runtime.exec("add1", {10});
    runtime.exec("add", {102, 324});
    runtime.exec("zero", {});
    // more input than normal
    runtime.exec("zero", {1, 2});
    runtime.exec("add", {3, 4});

    // todo missing param -> get_local none -> bad_alloc -> how?
    // runtime.exec("add", {3});
}
