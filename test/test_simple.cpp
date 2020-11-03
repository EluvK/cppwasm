#include "../wasi-runtime.h"

#include <gtest/gtest.h>
class A {
    int a;
    float f;
    int arr[10];
};

class func_print : public host_func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: print args.size(): %d", args.size());
        for(auto v:args){
            xdbg("[print:]%d", args[0].to_i32());
        }
        return{};
    }
};

TEST(test_, store_load_1) {
    const char * file_path{"../example/store-load.wasm"};

    Module mod{file_path};

    func_print print;
    host_func_base_ptr _print = std::make_shared<func_print>(print);
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;

    imps["env"] = {{"print", _print}};

    Runtime runtime{mod, imps};

    // runtime.exec("show",{});
    // runtime.exec("set", {12});
    // runtime.exec("add", {});
    // runtime.exec("show",{});
}

