#pragma once
#include "xmacro.h"
#include <assert.h>
// #include "xmemory.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using byte = uint8_t;
using byte_vec = std::vector<byte>;

class byte_IO {
public:
    byte_IO(byte_vec _data) : data{_data} {
    }

    byte_vec read(uint32_t size) {
        byte_vec res{};
        ASSERT(size <= data.size(), "byte_IO read overflow");
        while (size--) {
            res.push_back(data[0]);
            data.erase(data.begin());
        }
        return res;
    }
    byte read_one() {
        ASSERT(!data.empty(), "byte_IO empty");
        byte f = data[0];
        data.erase(data.begin());
        return f;
    }
    bool empty() {
        return data.empty();
    }
    std::size_t size() {
        return data.size();
    }

    byte_vec data;
};

#define XDEBUG_BV(bv)                                                                                                                                                              \
    for (auto & b : bv)                                                                                                                                                            \
        std::printf("0x%02x ", b);                                                                                                                                                 \
    std::printf("\n");

static const byte TYPE_i32 = 0x7f;
static const byte TYPE_i64 = 0x7e;
static const byte TYPE_f32 = 0x7d;
static const byte TYPE_f64 = 0x7c;

#define f32_nan_canonical 0x7fc00000
#define f64_nan_canonical 0x7ff8000000000000

#define f32_positive_zero 0x00000000
#define f32_negative_zero 0x80000000

#define f64_positive_zero 0x0000000000000000
#define f64_negative_zero 0x8000000000000000

// IMPORTANT
#define CUSTOM_SECTION_INDEX 0x00
#define TYPE_SECTION_INDEX 0x01
#define IMPORT_SECTION_INDEX 0x02
#define FUNCTION_SECTION_INDEX 0x03
#define TABLE_SECTION_INDEX 0x04
#define MEMORY_SECTION_INDEX 0x05
#define GLOBAL_SECTION_INDEX 0x06
#define EXPORT_SECTION_INDEX 0x07
#define START_SECTION_INDEX 0x08
#define ELEMENT_SECTION_INDEX 0x09
#define CODE_SECTION_INDEX 0x0a
#define DATA_SECTION_INDEX 0x0b

// doc:
#define WA_MAGIC 0x6d736100
#define WA_VERSION 0x01

namespace instruction {
static const byte unreachable = 0x00;
static const byte nop = 0x01;
static const byte block = 0x02;
static const byte loop = 0x03;
static const byte if_ = 0x04;
static const byte else_ = 0x05;
static const byte end = 0x0b;
static const byte br = 0x0c;
static const byte br_if = 0x0d;
static const byte br_table = 0x0e;
static const byte return_ = 0x0f;
static const byte call = 0x10;
static const byte call_indirect = 0x11;
static const byte drop = 0x1a;
static const byte select = 0x1b;
static const byte get_local = 0x20;
static const byte set_local = 0x21;
static const byte tee_local = 0x22;
static const byte get_global = 0x23;
static const byte set_global = 0x24;
static const byte i32_load = 0x28;
static const byte i64_load = 0x29;
static const byte f32_load = 0x2a;
static const byte f64_load = 0x2b;
static const byte i32_load8_s = 0x2c;
static const byte i32_load8_u = 0x2d;
static const byte i32_load16_s = 0x2e;
static const byte i32_load16_u = 0x2f;
static const byte i64_load8_s = 0x30;
static const byte i64_load8_u = 0x31;
static const byte i64_load16_s = 0x32;
static const byte i64_load16_u = 0x33;
static const byte i64_load32_s = 0x34;
static const byte i64_load32_u = 0x35;
static const byte i32_store = 0x36;
static const byte i64_store = 0x37;
static const byte f32_store = 0x38;
static const byte f64_store = 0x39;
static const byte i32_store8 = 0x3a;
static const byte i32_store16 = 0x3b;
static const byte i64_store8 = 0x3c;
static const byte i64_store16 = 0x3d;
static const byte i64_store32 = 0x3e;
static const byte current_memory = 0x3f;
static const byte grow_memory = 0x40;
static const byte i32_const = 0x41;
static const byte i64_const = 0x42;
static const byte f32_const = 0x43;
static const byte f64_const = 0x44;
static const byte i32_eqz = 0x45;
static const byte i32_eq = 0x46;
static const byte i32_ne = 0x47;
static const byte i32_lts = 0x48;
static const byte i32_ltu = 0x49;
static const byte i32_gts = 0x4a;
static const byte i32_gtu = 0x4b;
static const byte i32_les = 0x4c;
static const byte i32_leu = 0x4d;
static const byte i32_ges = 0x4e;
static const byte i32_geu = 0x4f;
static const byte i64_eqz = 0x50;
static const byte i64_eq = 0x51;
static const byte i64_ne = 0x52;
static const byte i64_lts = 0x53;
static const byte i64_ltu = 0x54;
static const byte i64_gts = 0x55;
static const byte i64_gtu = 0x56;
static const byte i64_les = 0x57;
static const byte i64_leu = 0x58;
static const byte i64_ges = 0x59;
static const byte i64_geu = 0x5a;
static const byte f32_eq = 0x5b;
static const byte f32_ne = 0x5c;
static const byte f32_lt = 0x5d;
static const byte f32_gt = 0x5e;
static const byte f32_le = 0x5f;
static const byte f32_ge = 0x60;
static const byte f64_eq = 0x61;
static const byte f64_ne = 0x62;
static const byte f64_lt = 0x63;
static const byte f64_gt = 0x64;
static const byte f64_le = 0x65;
static const byte f64_ge = 0x66;
static const byte i32_clz = 0x67;
static const byte i32_ctz = 0x68;
static const byte i32_popcnt = 0x69;
static const byte i32_add = 0x6a;
static const byte i32_sub = 0x6b;
static const byte i32_mul = 0x6c;
static const byte i32_divs = 0x6d;
static const byte i32_divu = 0x6e;
static const byte i32_rems = 0x6f;
static const byte i32_remu = 0x70;
static const byte i32_and = 0x71;
static const byte i32_or = 0x72;
static const byte i32_xor = 0x73;
static const byte i32_shl = 0x74;
static const byte i32_shrs = 0x75;
static const byte i32_shru = 0x76;
static const byte i32_rotl = 0x77;
static const byte i32_rotr = 0x78;
static const byte i64_clz = 0x79;
static const byte i64_ctz = 0x7a;
static const byte i64_popcnt = 0x7b;
static const byte i64_add = 0x7c;
static const byte i64_sub = 0x7d;
static const byte i64_mul = 0x7e;
static const byte i64_divs = 0x7f;
static const byte i64_divu = 0x80;
static const byte i64_rems = 0x81;
static const byte i64_remu = 0x82;
static const byte i64_and = 0x83;
static const byte i64_or = 0x84;
static const byte i64_xor = 0x85;
static const byte i64_shl = 0x86;
static const byte i64_shrs = 0x87;
static const byte i64_shru = 0x88;
static const byte i64_rotl = 0x89;
static const byte i64_rotr = 0x8a;
static const byte f32_abs = 0x8b;
static const byte f32_neg = 0x8c;
static const byte f32_ceil = 0x8d;
static const byte f32_floor = 0x8e;
static const byte f32_trunc = 0x8f;
static const byte f32_nearest = 0x90;
static const byte f32_sqrt = 0x91;
static const byte f32_add = 0x92;
static const byte f32_sub = 0x93;
static const byte f32_mul = 0x94;
static const byte f32_div = 0x95;
static const byte f32_min = 0x96;
static const byte f32_max = 0x97;
static const byte f32_copysign = 0x98;
static const byte f64_abs = 0x99;
static const byte f64_neg = 0x9a;
static const byte f64_ceil = 0x9b;
static const byte f64_floor = 0x9c;
static const byte f64_trunc = 0x9d;
static const byte f64_nearest = 0x9e;
static const byte f64_sqrt = 0x9f;
static const byte f64_add = 0xa0;
static const byte f64_sub = 0xa1;
static const byte f64_mul = 0xa2;
static const byte f64_div = 0xa3;
static const byte f64_min = 0xa4;
static const byte f64_max = 0xa5;
static const byte f64_copysign = 0xa6;
static const byte i32_wrap_i64 = 0xa7;
static const byte i32_trunc_sf32 = 0xa8;
static const byte i32_trunc_uf32 = 0xa9;
static const byte i32_trunc_sf64 = 0xaa;
static const byte i32_trunc_uf64 = 0xab;
static const byte i64_extend_si32 = 0xac;
static const byte i64_extend_ui32 = 0xad;
static const byte i64_trunc_sf32 = 0xae;
static const byte i64_trunc_uf32 = 0xaf;
static const byte i64_trunc_sf64 = 0xb0;
static const byte i64_trunc_uf64 = 0xb1;
static const byte f32_convert_si32 = 0xb2;
static const byte f32_convert_ui32 = 0xb3;
static const byte f32_convert_si64 = 0xb4;
static const byte f32_convert_ui64 = 0xb5;
static const byte f32_demote_f64 = 0xb6;
static const byte f64_convert_si32 = 0xb7;
static const byte f64_convert_ui32 = 0xb8;
static const byte f64_convert_si64 = 0xb9;
static const byte f64_convert_ui64 = 0xba;
static const byte f64_promote_f32 = 0xbb;
static const byte i32_reinterpret_f32 = 0xbc;
static const byte i64_reinterpret_f64 = 0xbd;
static const byte f32_reinterpret_i32 = 0xbe;
static const byte f64_reinterpret_i64 = 0xbf;

}  // namespace instruction

#define INS_TO_STR(name) case instruction::name: return #name;

static std::string instruction_to_string(byte _b) {
    assert(_b <= 0xbf && _b >= 0x00);
    switch (_b) {
        INS_TO_STR(unreachable);
        INS_TO_STR(nop);
        INS_TO_STR(block);
        INS_TO_STR(loop);
        INS_TO_STR(if_);
        INS_TO_STR(else_);
        INS_TO_STR(end);
        INS_TO_STR(br);
        INS_TO_STR(br_if);
        INS_TO_STR(br_table);
        INS_TO_STR(return_);
        INS_TO_STR(call);
        INS_TO_STR(call_indirect);
        INS_TO_STR(drop);
        INS_TO_STR(select);
        INS_TO_STR(get_local);
        INS_TO_STR(set_local);
        INS_TO_STR(tee_local);
        INS_TO_STR(get_global);
        INS_TO_STR(set_global);
        INS_TO_STR(i32_load);
        INS_TO_STR(i64_load);
        INS_TO_STR(f32_load);
        INS_TO_STR(f64_load);
        INS_TO_STR(i32_load8_s);
        INS_TO_STR(i32_load8_u);
        INS_TO_STR(i32_load16_s);
        INS_TO_STR(i32_load16_u);
        INS_TO_STR(i64_load8_s);
        INS_TO_STR(i64_load8_u);
        INS_TO_STR(i64_load16_s);
        INS_TO_STR(i64_load16_u);
        INS_TO_STR(i64_load32_s);
        INS_TO_STR(i64_load32_u);
        INS_TO_STR(i32_store);
        INS_TO_STR(i64_store);
        INS_TO_STR(f32_store);
        INS_TO_STR(f64_store);
        INS_TO_STR(i32_store8);
        INS_TO_STR(i32_store16);
        INS_TO_STR(i64_store8);
        INS_TO_STR(i64_store16);
        INS_TO_STR(i64_store32);
        INS_TO_STR(current_memory);
        INS_TO_STR(grow_memory);
        INS_TO_STR(i32_const);
        INS_TO_STR(i64_const);
        INS_TO_STR(f32_const);
        INS_TO_STR(f64_const);
        INS_TO_STR(i32_eqz);
        INS_TO_STR(i32_eq);
        INS_TO_STR(i32_ne);
        INS_TO_STR(i32_lts);
        INS_TO_STR(i32_ltu);
        INS_TO_STR(i32_gts);
        INS_TO_STR(i32_gtu);
        INS_TO_STR(i32_les);
        INS_TO_STR(i32_leu);
        INS_TO_STR(i32_ges);
        INS_TO_STR(i32_geu);
        INS_TO_STR(i64_eqz);
        INS_TO_STR(i64_eq);
        INS_TO_STR(i64_ne);
        INS_TO_STR(i64_lts);
        INS_TO_STR(i64_ltu);
        INS_TO_STR(i64_gts);
        INS_TO_STR(i64_gtu);
        INS_TO_STR(i64_les);
        INS_TO_STR(i64_leu);
        INS_TO_STR(i64_ges);
        INS_TO_STR(i64_geu);
        INS_TO_STR(f32_eq);
        INS_TO_STR(f32_ne);
        INS_TO_STR(f32_lt);
        INS_TO_STR(f32_gt);
        INS_TO_STR(f32_le);
        INS_TO_STR(f32_ge);
        INS_TO_STR(f64_eq);
        INS_TO_STR(f64_ne);
        INS_TO_STR(f64_lt);
        INS_TO_STR(f64_gt);
        INS_TO_STR(f64_le);
        INS_TO_STR(f64_ge);
        INS_TO_STR(i32_clz);
        INS_TO_STR(i32_ctz);
        INS_TO_STR(i32_popcnt);
        INS_TO_STR(i32_add);
        INS_TO_STR(i32_sub);
        INS_TO_STR(i32_mul);
        INS_TO_STR(i32_divs);
        INS_TO_STR(i32_divu);
        INS_TO_STR(i32_rems);
        INS_TO_STR(i32_remu);
        INS_TO_STR(i32_and);
        INS_TO_STR(i32_or);
        INS_TO_STR(i32_xor);
        INS_TO_STR(i32_shl);
        INS_TO_STR(i32_shrs);
        INS_TO_STR(i32_shru);
        INS_TO_STR(i32_rotl);
        INS_TO_STR(i32_rotr);
        INS_TO_STR(i64_clz);
        INS_TO_STR(i64_ctz);
        INS_TO_STR(i64_popcnt);
        INS_TO_STR(i64_add);
        INS_TO_STR(i64_sub);
        INS_TO_STR(i64_mul);
        INS_TO_STR(i64_divs);
        INS_TO_STR(i64_divu);
        INS_TO_STR(i64_rems);
        INS_TO_STR(i64_remu);
        INS_TO_STR(i64_and);
        INS_TO_STR(i64_or);
        INS_TO_STR(i64_xor);
        INS_TO_STR(i64_shl);
        INS_TO_STR(i64_shrs);
        INS_TO_STR(i64_shru);
        INS_TO_STR(i64_rotl);
        INS_TO_STR(i64_rotr);
        INS_TO_STR(f32_abs);
        INS_TO_STR(f32_neg);
        INS_TO_STR(f32_ceil);
        INS_TO_STR(f32_floor);
        INS_TO_STR(f32_trunc);
        INS_TO_STR(f32_nearest);
        INS_TO_STR(f32_sqrt);
        INS_TO_STR(f32_add);
        INS_TO_STR(f32_sub);
        INS_TO_STR(f32_mul);
        INS_TO_STR(f32_div);
        INS_TO_STR(f32_min);
        INS_TO_STR(f32_max);
        INS_TO_STR(f32_copysign);
        INS_TO_STR(f64_abs);
        INS_TO_STR(f64_neg);
        INS_TO_STR(f64_ceil);
        INS_TO_STR(f64_floor);
        INS_TO_STR(f64_trunc);
        INS_TO_STR(f64_nearest);
        INS_TO_STR(f64_sqrt);
        INS_TO_STR(f64_add);
        INS_TO_STR(f64_sub);
        INS_TO_STR(f64_mul);
        INS_TO_STR(f64_div);
        INS_TO_STR(f64_min);
        INS_TO_STR(f64_max);
        INS_TO_STR(f64_copysign);
        INS_TO_STR(i32_wrap_i64);
        INS_TO_STR(i32_trunc_sf32);
        INS_TO_STR(i32_trunc_uf32);
        INS_TO_STR(i32_trunc_sf64);
        INS_TO_STR(i32_trunc_uf64);
        INS_TO_STR(i64_extend_si32);
        INS_TO_STR(i64_extend_ui32);
        INS_TO_STR(i64_trunc_sf32);
        INS_TO_STR(i64_trunc_uf32);
        INS_TO_STR(i64_trunc_sf64);
        INS_TO_STR(i64_trunc_uf64);
        INS_TO_STR(f32_convert_si32);
        INS_TO_STR(f32_convert_ui32);
        INS_TO_STR(f32_convert_si64);
        INS_TO_STR(f32_convert_ui64);
        INS_TO_STR(f32_demote_f64);
        INS_TO_STR(f64_convert_si32);
        INS_TO_STR(f64_convert_ui32);
        INS_TO_STR(f64_convert_si64);
        INS_TO_STR(f64_convert_ui64);
        INS_TO_STR(f64_promote_f32);
        INS_TO_STR(i32_reinterpret_f32);
        INS_TO_STR(i64_reinterpret_f64);
        INS_TO_STR(f32_reinterpret_i32);
        INS_TO_STR(f64_reinterpret_i64);
    }
}