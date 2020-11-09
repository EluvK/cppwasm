#include "../cppwasm/base/Variant.h"
#include "../cppwasm/base/wasi-define.h"
#include "../cppwasm/wasi-binary.h"

#include <assert.h>

#include <iostream>

using InputType = Variant<int64_t, double, std::string>;
#define TYPE_I64 1
#define TYPE_F64 2
#define TYPE_STR 3

static byte_vec F_encode(double f64) {
    byte_vec res{};
    char * p = (char *)&f64;
    for (int index = 0; index < 8; ++index) {
        res.push_back(*p++);
    }
    return res;
}
static double F_decode(byte_vec bv) {
    char p[8] = {};
    for (auto index = 0; index < bv.size(); index++) {
        p[index] = bv[index];
    }
    double res = *(double *)p;
    return res;
}

static byte_vec S_encode(std::string str) {
    byte_vec res{};
    for (auto c : str) {
        res.push_back(c);
    }
    return res;
}
static std::string S_decode(byte_vec bv) {
    std::string res;
    for (auto b : bv) {
        res += b;
    }
    return res;
}

class Value {
public:
    Value() {
    }
    Value(byte_vec bv) : raw_data{bv} {
    }
    Value(int32_t i32) : raw_data{I_encode(i32)} {
    }
    Value(int64_t i64) : raw_data{I_encode(i64)} {
    }
    Value(uint32_t u32) : raw_data{U_encode(u32)} {
    }
    Value(uint64_t u64) : raw_data{U_encode(u64)} {
    }
    Value(float f32) : raw_data{F_encode(static_cast<double>(f32))} {
    }
    Value(double f64) : raw_data{F_encode(f64)} {
    }
    Value(std::string str) : raw_data{S_encode(str)} {
    }

    int32_t to_i32() {
        return static_cast<int32_t>(I_decode(raw_data));
    }
    int64_t to_i64() {
        return I_decode(raw_data);
    }
    uint32_t to_u32() {
        return static_cast<uint32_t>(U_decode(raw_data));
    }
    uint64_t to_u64() {
        return U_decode(raw_data);
    }
    float to_f32() {
        return static_cast<float>(F_decode(raw_data));
    }
    double to_f64() {
        return F_decode(raw_data);
    }
    std::string to_string() {
        return S_decode(raw_data);
    }

    static Value newValue(ValueType type, InputType data) {
        switch (data.GetType()) {
        case TYPE_I64:
            return Value(data.GetConstRef<int64_t>());
        case TYPE_F64:
            return Value(data.GetConstRef<double>());
        case TYPE_STR:
            return Value(data.GetConstRef<std::string>());
        default:
            xerror("cppwasm unknow input type");
        }
    }

private:
    byte_vec raw_data;
};

int main() {
    int32_t i32 = 1234;
    uint32_t u32 = 12345;
    int64_t i64 = 323415;
    uint64_t u64 = 12312512;
    float f32 = 1.2345;
    double f64 = -1234.513;
    assert(Value{i32}.to_i32() == i32);
    assert(Value{u32}.to_u32() == u32);
    assert(Value{i64}.to_i64() == i64);
    assert(Value{u64}.to_u64() == u64);
    assert(Value(-1234).to_i32() == -1234);
    assert(Value{i32}.to_u32() == i32);

    assert(Value{f32}.to_f32() == f32);
    assert(Value{f64}.to_f64() == f64);

    assert(Value{"12341235qwertyuioplkjhgfdsazxcvbn"}.to_string() == "12341235qwertyuioplkjhgfdsazxcvbn");

    for (int i = 0; i < 10; ++i) {
        double b = 123.1234 + i;
        byte_vec res = F_encode(b);
        double f = F_decode(res);
        xdbg("%f", f);
    }
    getchar();
}