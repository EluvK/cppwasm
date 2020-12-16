#include "../test_mvp/test_mvp.h"

void show(byte_vec bv){
    for(auto b:bv){
        std::printf("0x%02x ",b);
    }
    std::printf("\n");
}

TEST(tmp,_1){
    // auto bv = LittleEndian::pack_i32(-4242);
    // show(bv);
    // auto bv2 = I_encode(4294963054);
    // show(bv2);

    // uint32_t val = atol("3212836864");
    // xdbg("expect_res ua :% " PRIu32, val);
    // float f2 = reinterpret_cast<float &>(val);
    // xdbg("float: %f",f2);
    
    // std::vector<int> t_l(10,3);
    // xdbg("%d",t_l.size());
    // if (t_l.size() < 12){
    //     xdbg("reserve");
    //     t_l.resize(12);
    // }
    // t_l[12]=1;

       // byte_vec bv = {0xfb,0x08,0x04,0x63};
    // auto bio = byte_IO{bv};
    // auto i= U_decode_reader(bio);
    // xdbg("%d",i);
    // for(auto b :bio.data){
    //     xdbg("0x%02x", b);
    // }
}

// TEST(nan_,test){
    //    uint32_t nann = 0x7f800001;
    // Value v(nann);
    // xdbg("%d %d %f", v.type(), v.to_u32(), v.to_f32());

    // uint32_t u32 = 0x80000000;
    // Value vv(u32);
    // xdbg("%d %d %f", vv.type(), vv.to_u32(), vv.to_f32());

    // xdbg("%f", std::min(v.to_f32(), vv.to_f32()));
    // xdbg("%f", std::max(v.to_f32(), vv.to_f32()));
    // xdbg("%s",v.to_f32()<vv.to_f32()?"true":"false");
    // float a = -0.0;
    // float b = nanf("1");
    // xdbg("%f =? %f", v.to_f32(), b);
    // xdbg("%f", std::min(a, b));
    // xdbg("%f", std::max(a, b));
    // xdbg("%f", std::min(b, a));
    // xdbg("%f", std::max(b, a));
    // xdbg("%s", a == b ? "true" : "false");
    // xdbg("%d",isnanf(a));
    // xdbg("%d",isnanf(b));
// }

// TEST(decode32,test){
//     byte_vec bv1{0xff,0xff,0xff,0xff,0x07};
//     auto res1 = I_decode(bv1);
//     xdbg("res1: % " PRId64, res1);
//     byte_vec bv2{0x81,0x80,0x80,0x80,0x78};
//     auto res2 = I_decode(bv2);
//     xdbg("res2: % " PRId64, res2);
//     int32_t a = -2147483647;
//     auto bva = I_encode(a);
//     XDEBUG_BV(bva);
//     auto resa = I_decode(bva);
//     xdbg("resa: % " PRId64, resa);
    
// }

// TEST(decode64,test){
//     byte_vec bv1{0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f};
//     auto res1 = I_decode(bv1);
//     xdbg("res1: % " PRId64, res1);
//     byte_vec bv2{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f};
//     auto res2 = I_decode(bv2);
//     xdbg("res2: % " PRId64, res2);
//     int64_t a = -9223372036854775807;
//     auto bva = I_encode(a);
//     XDEBUG_BV(bva);
//     auto resa = I_decode(bva);
//     xdbg("resa: % " PRId64, resa);
    
// }


TEST_F(cppwasm_test_mvp, test_tmp){

    // test_case("../spectest/table");
    // test_case("../spectest/token");
    // test_case("../spectest/traps");
    // test_case("../spectest/type");
    // test_case("../spectest/typecheck");
    // test_case("../spectest/unreachable");
    // test_case("../spectest/unreached-invalid");
    // test_case("../spectest/unwind");
    // test_case("../spectest/utf8-custom-section-id");
    // test_case("../spectest/utf8-import-field");
    // test_case("../spectest/utf8-import-module");
    // test_case("../spectest/utf8-invalid-encoding");
 
}