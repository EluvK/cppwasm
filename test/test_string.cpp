#include "../wasi-runtime.h"

#include <gtest/gtest.h>
#include <string>

class func_print_char : public func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: printchar");
        for(auto v:args){
            std::printf("%d", args[0].to_i32());
        }
        return{};
    }
};

TEST(test_, show_string_1) {
    const char * file_path{"../example/show_string.wasm"};

    Module mod{file_path};
    func_print_char print;
    func_base_ptr _print = std::make_shared<func_print_char>(print);
    std::map<std::string, std::map<std::string, func_base_ptr>> imps;
    imps["env"] = {{"print", _print}};
    Runtime runtime{mod, imps};
    runtime.exec("show", {10});
}