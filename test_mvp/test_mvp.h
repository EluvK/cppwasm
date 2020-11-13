#pragma once
#include "../wasi-runtime.h"
#include "json.hpp"

#include <gtest/gtest.h>

#include <string>
using json = nlohmann::json;

class cppwasm_test_mvp : public testing::Test{
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

    void assert_result(Result & expect_res, Result & real_res) {
        std::printf("exp: ");
        for (auto index = 0; index < expect_res.data[0].raw().size(); index++) {
            std::printf("0x%02x  ", expect_res.data[0].raw()[index]);
        }
        std::printf("\n------\nact: ");
        for (auto index = 0; index < real_res.data[0].raw().size(); index++) {
            std::printf("0x%02x  ", real_res.data[0].raw()[index]);
        }
        std::printf("\n");
        assert(expect_res.data.size() == real_res.data.size());
        for (auto index = 0; index < expect_res.data.size(); index++) {
            for (auto index1 = 0; index1 < expect_res.data[index].raw().size(); index1++) {
                assert(expect_res.data[index].raw()[index1] == real_res.data[index].raw()[index1]);
            }
        }
    }

    uint64_t stringToUINT64(const std::string s) {
        std::stringstream a;
        a << s;
        uint64_t ret = 0;
        a >> ret;
        return ret;
    }

    Result parse_exp(nlohmann::json const & expect_json) {
        Result exp_res;
        std::cout << "expect_json " << expect_json << std::endl;
        for (auto & p : expect_json) {
            std::cout << "each pair: " << p << std::endl;
            std::string str = p["value"];
            if (p["type"] == "i32") {
                int32_t val = atoi(str.c_str());
                xdbg("expect_res :% " PRId32, val);
                exp_res.data.push_back(Value(val));
            } else if (p["type"] == "i64") {
                uint64_t ua = stringToUINT64(str.c_str());
                xdbg("expect_res ua :% " PRIu64, ua);
                int64_t u2i = static_cast<int64_t>(ua);
                xdbg("expect_res u2i:% " PRId64, u2i);
                // int64_t a = atoll(str.c_str());
                // xdbg("expect_res :% " PRId64, a);
                exp_res.data.push_back(Value(u2i));
            } else if (p["type"] == "f32") {
                uint32_t val = atol(str.c_str());
                float f32 = reinterpret_cast<float &>(val);
                xdbg("expect_res f32:%f", f32);
                exp_res.data.push_back(Value(f32));
            } else if (p["type"] == "f64") {
                
                uint64_t val = stringToUINT64(str.c_str());
                double f64 = reinterpret_cast<double &>(val);
                xdbg("expect_res f64:%lf", f64);
                exp_res.data.push_back(Value(f64));

            }
        }
        return exp_res;
    }
    std::vector<InputType> parse_arg(nlohmann::json const & args_json) {
        std::vector<InputType> res;
        std::cout << "args_json " << args_json << std::endl;
        for (auto & p : args_json) {
            std::cout << "each pair: " << p << std::endl;
            std::string str = p["value"];
            if (p["type"] == "i32") {
                int32_t a = atoi(str.c_str());
                res.push_back(a);
                xdbg("args_input :% " PRId32, a);
            } else if (p["type"] == "i64") {
                uint64_t ua = stringToUINT64(str.c_str());
                xdbg("inputargs ua :% " PRIu64, ua);
                int64_t u2i = static_cast<int64_t>(ua);
                xdbg("inputargs u2i:% " PRId64, u2i);
                res.push_back(u2i);
            } else if (p["type"] == "f32") {
                uint32_t val = atol(str.c_str());
                float f32 = reinterpret_cast<float &>(val);
                xdbg("inputargs f32:%f", f32);
                res.push_back(f32);
            } else if (p["type"] == "f64") {
                uint64_t val = stringToUINT64(str.c_str());
                double f64 = reinterpret_cast<double &>(val);
                xdbg("inputargs f64:%lf", f64);
                res.push_back(f64);
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
        std::map<std::string, std::map<std::string, host_func_base_ptr>> imps{};
        for (auto & command : case_data["commands"]) {
            std::cout << "[------ DEBUG line:" << command["line"] << " : ------]" << std::endl << command << std::endl;
            if (command["type"] == "module") {
                file_name = command["filename"];
                auto wasm_file_path = directory + "/" + file_name;
                xdbg("wasm_file_path: %s", wasm_file_path.c_str());
                mod = Module{wasm_file_path.c_str()};
                rt = Runtime{mod, imps};
            } else if (command["type"] == "assert_return") {
                // {"type": "assert_return", "line": 104, "action": {"type": "invoke", "field": "8u_good1", "args": [{"type": "i32", "value": "0"}]}, "expected": [{"type": "i32",
                // "value": "97"}]},
                if (command["action"]["type"] == "invoke") {
                    std::string function_name = command["action"]["field"];
                    // xdbg("function_name: %s", function_name.c_str());
                    std::vector<InputType> args = parse_arg(command["action"]["args"]);
                    // xdbg("args.size:%d", args.size());
                    Result expect_res = parse_exp(command["expected"]);
                    // xdbg("expect_res.size:%d", expect_res.data.size());
                    Result real_res = rt.exec(function_name, args);

                    assert_result(expect_res, real_res);

                } else {
                    xdbg("no implement");
                    assert(false);
                }
            } else if (command["type"] == "assert_trap" || command["type"] == "assert_exhaustion") {
                if (command["action"]["type"] == "invoke") {
                    std::string function_name = command["action"]["field"];
                    // xdbg("function_name: %s", function_name.c_str());
                    std::vector<InputType> args = parse_arg(command["action"]["args"]);
                    // xdbg("args.size:%d", args.size());
                    try {
                        rt.exec(function_name, args);
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
            }else if (command["type"]=="assert_invalid"){
                continue; //alignment must not be larger than natural? what's this for.?
            } else {
                assert(false);
            }
            // else if (command["type"] == "") {
            // } else if (command["type"] == "assert_malformed") {
            //     continue;
            // } else if (command["type"] == "assert_invalid") {
            //     continue;
            // } else if (command["type"] == "assert_unlinkable") {
            //     continue;
            // } else if (command["type"] == "register") {
            //     continue;
            // }
        }
    }

protected:
    void SetUp() override{}
    void TearDown() override{}

};
