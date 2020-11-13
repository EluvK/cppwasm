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
}

TEST(test_, littleendian) {
    byte_vec bv1{0x01, 0x00, 0x00, 0x00};  // 1
    byte_vec bv2{0x00, 0x00, 0x00, 0x80};  //-2147483648
    byte_vec bv3{0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00};
    byte_vec bv4{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    EXPECT_EQ(LittleEndian::i32(bv1), 1);
    EXPECT_EQ(LittleEndian::i32(bv2), -2147483648);
    EXPECT_EQ(LittleEndian::u32(bv2), 2147483648);
    EXPECT_EQ(LittleEndian::pack_i32(-2147483648), bv2);
    EXPECT_EQ(LittleEndian::pack_i32(1), bv1);
    EXPECT_EQ(LittleEndian::pack_i64(LittleEndian::i64(bv3)), bv3);
    EXPECT_EQ(LittleEndian::pack_u64(LittleEndian::u64(bv4)), bv4);
    EXPECT_EQ(LittleEndian::pack_i64(LittleEndian::i64(bv4)), bv4);

    float f = 1.234;
    double d = -123412.51253;
    EXPECT_EQ(LittleEndian::f32(LittleEndian::pack_f32(f)), f);
    EXPECT_EQ(LittleEndian::f64(LittleEndian::pack_f64(d)), d);
}