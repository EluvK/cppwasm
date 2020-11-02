#include "../wasi-runtime.h"

#include <gtest/gtest.h>
#include <string>

class func_say_hello : public host_func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: func_say_hello");
        std::printf("Hi: %s\n", args[0].to_string().c_str());
        return{};
    }
};

TEST(test_, show_string_1) {
    const char * file_path{"../example/show_string.wasm"};

    Module mod{file_path};
    func_say_hello print;
    host_func_base_ptr _print = std::make_shared<func_say_hello>(print);
    std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
    imps["env"] = {{"print", _print}};
    Runtime runtime{mod, imps};
    runtime.exec("show", {"Charles"});
}