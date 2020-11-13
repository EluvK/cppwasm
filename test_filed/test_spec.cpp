#include "../test_mvp/test_mvp.h"

void show(byte_vec bv){
    for(auto b:bv){
        std::printf("0x%02x ",b);
    }
    std::printf("\n");
}

TEST(tmp,_1){
    auto bv = LittleEndian::pack_i32(-4242);
    show(bv);
    auto bv2 = I_encode(4294963054);
    show(bv2);

    uint32_t val = atol("3212836864");
    xdbg("expect_res ua :% " PRIu32, val);
    float f2 = reinterpret_cast<float &>(val);
    xdbg("float: %f",f2);

}

TEST_F(cppwasm_test_mvp, test_tmp){
    test_case("../spectest/endianness");

}