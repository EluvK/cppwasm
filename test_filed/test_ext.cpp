#include "../wasi-runtime.h"

#include <gtest/gtest.h>

int64_t cal_fib(int64_t n) {
    if (n <= 1) {
        return n;
    }
    return cal_fib(n - 1) + cal_fib(n - 2);
}

class func_save : public host_func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: save");
        return {};
        // Value res{cal_fib(args[0].to_i64())};
        // Result r;
        // r.data.push_back(res);
        // return r;
    }
};
class func_print : public host_func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: print");
        for(auto v:args){
            std::printf("%d", args[0].to_i32());
        }
        return{};
    }
};

TEST(test_, x_1) {
    const char * file_path{"../example/llvm_test/climit.wasm"};

    Module mod{file_path};
    func_save save;
    host_func_base_ptr _save = std::make_shared<func_save>(save);
    func_print print;
    host_func_base_ptr _print = std::make_shared<func_print>(print);

    std::map<std::string, std::map<std::string, imp_variant>>  imps;
    imps["env"] = {{"save", _save}, {"print", _print}};
    Runtime runtime{mod, imps};
    EXPECT_EQ(runtime.exec("_Z4testi", {1234}).data[0].to_i32(), 4321);
    EXPECT_EQ(runtime.exec("_Z4testi", {789456123}).data[0].to_i32(), 321654987);
    // EXPECT_EQ(runtime.exec("geta", {}).data[0].to_i32(),20);
    // EXPECT_EQ(runtime.exec("getf", {}).data[0].to_f32(), (float)1.2);
}