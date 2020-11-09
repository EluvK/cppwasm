// #include "../wasi-runtime.h"

// #include <gtest/gtest.h>
// class A {
//     int a;
//     float f;
//     int arr[10];
// };

// class func_print : public host_func_base {
// public:
//     Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
//         xdbg("cppwasm: print args.size(): %d", args.size());
//         for(auto v:args){
//             xdbg("[print:]%d", args[0].to_i32());
//         }
//         return{};
//     }
// };

// // unknown bug addr = -8? should not be negative number may be wrong wasm file compilered by WasmFiddle in C++?
// TEST(test_, struct_1) {
//     const char * file_path{"../example/struct.wasm"};

//     Module mod{file_path};

//     func_print print2;
//     host_func_base_ptr _print2 = std::make_shared<func_print>(print2);
//     std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;

//     imps["env"] = {{"print", _print2}};

//     Runtime runtime{mod, imps};

//     runtime.exec("test",{9,10});
// }

// TEST(test_, t_1) {
//     const char * file_path{"../example/t.wasm"};

//     Module mod{file_path};

//     // func_print print2;
//     // host_func_base_ptr _print2 = std::make_shared<func_print>(print2);
//     std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;

//     // imps["env"] = {{"print", _print2}};

//     Runtime runtime{mod, imps};

//     runtime.exec("test",{9,8.8});
// }

// TEST(test_, store_load_1) {
//     const char * file_path{"../example/store-load.wasm"};

//     Module mod{file_path};

//     func_print print;
//     host_func_base_ptr _print = std::make_shared<func_print>(print);
//     std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;

//     imps["env"] = {{"print", _print}};

//     Runtime runtime{mod, imps};

//     EXPECT_EQ(0, runtime.exec("show", {}).data[0].to_i32());
//     runtime.exec("set", {1234});
//     EXPECT_EQ(1234, runtime.exec("show", {}).data[0].to_i32());
//     runtime.exec("add", {});
//     EXPECT_EQ(1235, runtime.exec("show", {}).data[0].to_i32());
// }

// TEST(test_, fib_1) {
//     const char * file_path{"../example/fib.wasm"};

//     Module mod{file_path};
//     std::map<std::string, std::map<std::string, host_func_base_ptr>> imps;
//     Runtime runtime{mod, imps};
//     ASSERT_EQ(runtime.exec("fib", {5}).data[0].to_i64(), 5);
// }