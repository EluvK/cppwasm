#include "../wasi-runtime.h"

#include <gtest/gtest.h>

TEST(test_, value_1) {
    int32_t i32 = 1234;
    uint32_t u32 = 12345;
    int64_t i64 = 323415;
    uint64_t u64 = 12312512;
    float f32 = 1.2345;
    double f64 = -1234.513;
    EXPECT_TRUE(Value{i32}.to_i32() == i32);
    EXPECT_TRUE(Value{u32}.to_u32() == u32);
    EXPECT_TRUE(Value{i64}.to_i64() == i64);
    EXPECT_TRUE(Value{u64}.to_u64() == u64);
    EXPECT_TRUE(Value(-1234).to_i32() == -1234);
    EXPECT_TRUE(Value{i32}.to_u32() == i32);

    EXPECT_TRUE(Value{f32}.to_f32() == f32);
    EXPECT_TRUE(Value{f64}.to_f64() == f64);
    EXPECT_TRUE(Value{"12341235qwertyuioplkjhgfdsazxcvbn"}.to_string() == "12341235qwertyuioplkjhgfdsazxcvbn");

    for (int i = 0; i < 10; ++i) {
        double b = 123.1234 + i;
        byte_vec res = F_encode(b);
        double f = F_decode(res);
        xdbg("%f", f);
    }
}