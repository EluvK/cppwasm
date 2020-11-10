// #define private public
#include "../wasi-leb128.h"

#include <gtest/gtest.h>

TEST(test_, f32f64_1) {
    const byte_vec f64_1{0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xf1, 0x3f};  // double 1.1
    const byte_vec f32_1{0xcd, 0xcc, 0x8c, 0x3f};                          // float 1.1
    byte_IO f32_io{f32_1};
    byte_IO f64_io{f64_1};
    EXPECT_EQ(F32_decode(f32_1), (float)1.1);
    EXPECT_EQ(F32_decode_reader(f32_io), (float)1.1);
    EXPECT_EQ(f32_io.empty(), true);

    EXPECT_EQ(F64_decode(f64_1), (double)1.1);
    EXPECT_EQ(F64_decode_reader(f64_io), (double)1.1);
    EXPECT_EQ(f64_io.empty(), true);

    EXPECT_EQ(F32_encode((float)1.1), f32_1);
    EXPECT_EQ(F64_encode((double)1.1), f64_1);
}
