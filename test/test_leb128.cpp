// #define private public
#include "../wasi-leb128.h"

#include <gtest/gtest.h>

#if 0
TEST(test_, leb128) {
    const byte_vec bv1{0xb0, 0x98, 0xc0, 0x02};
    const byte_vec bv2{0xa4, 0x18};
    EXPECT_EQ(U_decode(bv1), 5246000);
    EXPECT_EQ(U_encode(5246000), bv1);

    EXPECT_EQ(I_decode(bv1), 5246000);
    EXPECT_EQ(I_encode(5246000), bv1);

    EXPECT_EQ(U_decode(bv2), 3108);
    EXPECT_EQ(U_encode(3108), bv2);

    EXPECT_EQ(I_decode(bv2), 3108);
    EXPECT_EQ(I_encode(3108), bv2);

    const byte_vec bv3{0xc7, 0x9f, 0x7f};
    EXPECT_EQ(I_decode(bv3), -12345);
    EXPECT_EQ(I_encode(-12345), bv3);

    byte_IO bio1(bv1), bio2 = bio1;
    EXPECT_EQ(U_decode_reader(bio1), 5246000);
    EXPECT_EQ(I_decode_reader(bio2), 5246000);
    EXPECT_TRUE(bio1.data.empty() && bio2.data.empty());
}
#endif