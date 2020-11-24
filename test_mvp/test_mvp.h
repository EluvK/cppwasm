#pragma once
#include "../wasi-runtime.h"
#include "json.hpp"

#include <gtest/gtest.h>

#include <string>
using json = nlohmann::json;

class empty_host_func : public host_func_base {
public:
    Result operator()(std::vector<Value> args, std::vector<ValueType> types) override {
        xdbg("cppwasm: empty_host_func");
        return {};
    }
};

class cppwasm_test_mvp : public testing::Test {
public:
    inline std::string get_case_name(std::string dir) {
        std::string res;
        for (auto index = dir.size() - 1; index >= 0; --index) {
            if (dir[index] == '/')
                break;
            res = dir[index] + res;
        }
        return res;
    }

    void assert_value(Value & a, Value & b) {
        if (b.type() == TYPE_i32 || b.type() == TYPE_i64) {
            assert(a.type() == b.type());
            assert(a.raw() == b.raw());
        } else if (b.type() == TYPE_f32) {
            assert(a.type() == TYPE_f32);
            if (isnanf(b.to_f32())) {
                if (b.to_i32() == f32_nan_canonical) {
                    assert(a.to_u32() == f32_nan_canonical || a.to_u32() == (f32_nan_canonical | 1 << 31));
                } else {
                    assert(isnanf(a.to_f32()));
                }
                return;
            }
            assert(a.raw() == b.raw());
        } else if (b.type() == TYPE_f64) {
            assert(a.type() == TYPE_f64);
            if (isnan(b.to_f64())) {
                if (b.to_i64() == f64_nan_canonical) {
                    assert(a.to_u64() == f64_nan_canonical || a.to_u64() == (f64_nan_canonical | 0x8000000000000000));
                } else {
                    assert(isnan(a.to_f64()));
                }
                return;
            }
            assert(a.raw() == b.raw());
        }
    }

    void assert_result(Result & expect_res, Result & real_res) {
        std::printf("exp: ");
        if (expect_res.data.empty()) {
            std::printf(" empty expect_res");
        } else {
            for (auto index = 0; index < expect_res.data[0].raw().size(); index++) {
                std::printf("0x%02x  ", expect_res.data[0].raw()[index]);
            }
        }
        std::printf("\n------\nact: ");
        if (real_res.data.empty()) {
            std::printf(" empty real_res");
        } else {
            for (auto index = 0; index < real_res.data[0].raw().size(); index++) {
                std::printf("0x%02x  ", real_res.data[0].raw()[index]);
            }
        }
        std::printf("\n");
        assert(expect_res.data.size() == real_res.data.size());
        for (auto index = 0; index < expect_res.data.size(); index++) {
            assert_value(real_res.data[index], expect_res.data[index]);
        }
    }

    uint64_t stringToUINT64(const std::string s) {
        std::stringstream a;
        a << s;
        uint64_t ret = 0;
        a >> ret;
        return ret;
    }

    std::vector<Value> parse_val(nlohmann::json const & json) {
        std::vector<Value> res;
        std::cout << json << std::endl;
        for (auto & p : json) {
            std::cout << "each pair: " << p << std::endl;
            std::string str = p["value"];
            if (p["type"] == "i32") {
                int32_t val = atoi(str.c_str());
                xdbg("res :% " PRId32, val);
                res.push_back(Value(val));
            } else if (p["type"] == "i64") {
                uint64_t ua = stringToUINT64(str.c_str());
                xdbg("res ua :% " PRIu64, ua);
                int64_t u2i = static_cast<int64_t>(ua);
                xdbg("res u2i:% " PRId64, u2i);
                res.push_back(Value(u2i));
            } else if (p["type"] == "f32") {
                uint32_t val{};
                if (str == "nan:canonical") {
                    val = f32_nan_canonical;
                    res.push_back(Value::from_f32_u32(val));
                    xdbg("nan:canonical val :% " PRIu32, val);
                } else if (str == "nan:arithmetic") {
                    val = f32_nan_canonical + 1;
                    res.push_back(Value::from_f32_u32(val));
                    xdbg("nan:arithmetic val :% " PRIu32, val);
                } else {
                    val = atol(str.c_str());
                    res.push_back(Value::from_f32_u32(val));
                    xdbg("nomal val :% " PRIu32, val);
                }
            } else if (p["type"] == "f64") {
                uint64_t val{};
                if (str == "nan:canonical") {
                    val = f64_nan_canonical;
                    res.push_back(Value::from_f64_u64(val));
                    xdbg("nan:canonical val :% " PRIu64, val);
                } else if (str == "nan:arithmetic") {
                    val = f64_nan_canonical + 1;
                    res.push_back(Value::from_f64_u64(val));
                    xdbg("nan:arithmetic val :% " PRIu64, val);
                } else {
                    val = stringToUINT64(str.c_str());
                    res.push_back(Value::from_f64_u64(val));
                    xdbg("nomal val :% " PRIu64, val);
                }
            }
        }
        return res;
    }

    void test_case(std::string directory) {
        auto case_name = get_case_name(directory);
        xdbg("case_name: %s", case_name.c_str());
        auto json_path = directory + "/" + case_name + ".json";
        xdbg("json_path: %s", json_path.c_str());
        std::ifstream ifs(json_path);

        json case_data;
        ifs >> case_data;

        std::string file_name;
        Module mod;
        Runtime rt;

        empty_host_func e_host_func;
        host_func_base_ptr e_func = std::make_shared<empty_host_func>(e_host_func);

        Limits _limits;
        _limits.n = 1;
        _limits.m = 2;
        MemoryType _mem_type{};
        _mem_type.limits = _limits;
        MemoryInstance _mem_ins = MemoryInstance{_mem_type};

        GlobalInstance _global_ins{Value{666}, Mut{0}};

        Limits _limits2;
        _limits2.n = 10;
        _limits2.m = 20;

        TableInstance _table_ins{FunctionAddress{}, _limits2};

        std::map<std::string, std::map<std::string, imp_variant>> test_imps{};
        test_imps["spectest"]["print_i32"] = e_func;
        test_imps["spectest"]["print"] = e_func;
        test_imps["spectest"]["memory"] = _mem_ins;
        test_imps["spectest"]["global_i32"] = _global_ins;
        test_imps["spectest"]["table"] = _table_ins;

        for (auto & command : case_data["commands"]) {
            std::cout << "[------ DEBUG line:" << command["line"] << " : ------]" << std::endl << command << std::endl;
            if (command["type"] == "module") {
                file_name = command["filename"];
                auto wasm_file_path = directory + "/" + file_name;
                xdbg("wasm_file_path: %s", wasm_file_path.c_str());
                mod = Module{wasm_file_path.c_str()};
                rt = Runtime{mod, test_imps};
            } else if (command["type"] == "assert_return") {
                // {"type": "assert_return", "line": 104, "action": {"type": "invoke", "field": "8u_good1", "args": [{"type": "i32", "value": "0"}]}, "expected": [{"type": "i32",
                // "value": "97"}]},
                if (command["action"]["type"] == "invoke") {
                    std::string function_name = command["action"]["field"];
                    // xdbg("function_name: %s", function_name.c_str());
                    std::vector<Value> args = parse_val(command["action"]["args"]);
                    // xdbg("args.size:%d", args.size());
                    Result expect_res;
                    expect_res.data = parse_val(command["expected"]);
                    // xdbg("expect_res.size:%d", expect_res.data.size());
                    Result real_res = rt.exec_accu(function_name, args);

                    assert_result(expect_res, real_res);

                } else {
                    xdbg("no implement");
                    assert(false);
                }
            } else if (command["type"] == "assert_trap" || command["type"] == "assert_exhaustion") {
                if (command["action"]["type"] == "invoke") {
                    std::string function_name = command["action"]["field"];
                    // xdbg("function_name: %s", function_name.c_str());
                    std::vector<Value> args = parse_val(command["action"]["args"]);
                    // xdbg("args.size:%d", args.size());
                    try {
                        rt.exec_accu(function_name, args);
                    } catch (const char * str) {
                        std::string s{str};
                        std::string expect = command["text"];
                        s = s.substr(9);
                        assert(s == expect);
                        continue;
                    } catch (...) {
                        assert(false);
                        continue;
                    }
                    xdbg("should has throw()");
                    assert(false);
                } else {
                    xdbg("no implement");
                    assert(false);
                }
            } else if (command["type"] == "assert_malformed") {
                continue;  // wat file.
            } else if (command["type"] == "assert_invalid") {
                continue;  // alignment must not be larger than natural? what's this for.?
            } else if (command["type"] == "assert_unlinkable") {
                continue;
                // } else if (command["type"] == "register") {
                //     continue;
            } else if (command["type"] == "assert_uninstantiable") {
                continue;
            } else if (command["type"] == "action") {
                if (command["action"]["type"] == "invoke") {
                    std::string function_name = command["action"]["field"];
                    std::vector<Value> args = parse_val(command["action"]["args"]);
                    rt.exec_accu(function_name, args);
                } else {
                    xdbg("no implement");
                    assert(false);
                }
                // if(command["line"]=="458") assert(false);
                // else if (command["type"] == "") {
            } else if (command["type"] == "register") {  // todo check why return .
                return;
            } else {
                assert(false);
            }
        }
    }

protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};
