#include "../wasi-runtime.h"

#include <gtest/gtest.h>

// int64_t cal_fib(int64_t n) {
//     if (n <= 1) {
//         return n;
//     }
//     return cal_fib(n - 1) + cal_fib(n - 2);
// }

// class func_fib : public host_func_base {
// public:
//     Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
//         xdbg("cppwasm: fib");
//         Value res{cal_fib(args[0].to_i64())};
//         Result r;
//         r.data.push_back(res);
//         return r;
//     }
// };
// class func_print : public host_func_base {
// public:
//     Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
//         xdbg("cppwasm: print");
//         for(auto v:args){
//             std::printf("%d", args[0].to_i32());
//         }
//         return{};
//     }
// };

// TEST(test_, etx_1) {
//     const char * file_path{"../example/ext.wasm"};

//     Module mod{file_path};
//     func_fib fib;
//     host_func_base_ptr _fib = std::make_shared<func_fib>(fib);
//     func_print print;
//     host_func_base_ptr _print = std::make_shared<func_print>(print);
//     std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
//     imps["env"] = {{"fib", _fib}, {"print", _print}};
//     Runtime runtime{mod, imps};
//     runtime.exec("cal", {10});
// }