#pragma once
#include "xmacro.h"
// #include "xmemory.hpp"
#include <iostream>
#include <map>
#include <memory>
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

static byte i32 = 0x7f;
static byte i64 = 0x7e;
static byte f32 = 0x7d;
static byte f64 = 0x7c;

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

// case instruction::unreachable:
// case instruction::nop:
// case instruction::block:
// case instruction::loop:
// case instruction::if_:
// case instruction::else_:
// case instruction::end:
// case instruction::br:
// case instruction::br_if:
// case instruction::br_table:
// case instruction::return_:
// case instruction::call:
// case instruction::call_indirect:
// case instruction::drop:
// case instruction::select:
// case instruction::get_local:
// case instruction::set_local:
// case instruction::tee_local:
// case instruction::get_global:
// case instruction::set_global:
// case instruction::i32_load:
// case instruction::i64_load:
// case instruction::f32_load:
// case instruction::f64_load:
// case instruction::i32_load8_s:
// case instruction::i32_load8_u:
// case instruction::i32_load16_s:
// case instruction::i32_load16_u:
// case instruction::i64_load8_s:
// case instruction::i64_load8_u:
// case instruction::i64_load16_s:
// case instruction::i64_load16_u:
// case instruction::i64_load32_s:
// case instruction::i64_load32_u:
// case instruction::i32_store:
// case instruction::i64_store:
// case instruction::f32_store:
// case instruction::f64_store:
// case instruction::i32_store8:
// case instruction::i32_store16:
// case instruction::i64_store8:
// case instruction::i64_store16:
// case instruction::i64_store32:
// case instruction::current_memory:
// case instruction::grow_memory:
// case instruction::i32_const:
// case instruction::i64_const:
// case instruction::f32_const:
// case instruction::f64_const:
// case instruction::i32_eqz:
// case instruction::i32_eq:
// case instruction::i32_ne:
// case instruction::i32_lts:
// case instruction::i32_ltu:
// case instruction::i32_gts:
// case instruction::i32_gtu:
// case instruction::i32_les:
// case instruction::i32_leu:
// case instruction::i32_ges:
// case instruction::i32_geu:
// case instruction::i64_eqz:
// case instruction::i64_eq:
// case instruction::i64_ne:
// case instruction::i64_lts:
// case instruction::i64_ltu:
// case instruction::i64_gts:
// case instruction::i64_gtu:
// case instruction::i64_les:
// case instruction::i64_leu:
// case instruction::i64_ges:
// case instruction::i64_geu:
// case instruction::f32_eq:
// case instruction::f32_ne:
// case instruction::f32_lt:
// case instruction::f32_gt:
// case instruction::f32_le:
// case instruction::f32_ge:
// case instruction::f64_eq:
// case instruction::f64_ne:
// case instruction::f64_lt:
// case instruction::f64_gt:
// case instruction::f64_le:
// case instruction::f64_ge:
// case instruction::i32_clz:
// case instruction::i32_ctz:
// case instruction::i32_popcnt:
// case instruction::i32_add:
// case instruction::i32_sub:
// case instruction::i32_mul:
// case instruction::i32_divs:
// case instruction::i32_divu:
// case instruction::i32_rems:
// case instruction::i32_remu:
// case instruction::i32_and:
// case instruction::i32_or:
// case instruction::i32_xor:
// case instruction::i32_shl:
// case instruction::i32_shrs:
// case instruction::i32_shru:
// case instruction::i32_rotl:
// case instruction::i32_rotr:
// case instruction::i64_clz:
// case instruction::i64_ctz:
// case instruction::i64_popcnt:
// case instruction::i64_add:
// case instruction::i64_sub:
// case instruction::i64_mul:
// case instruction::i64_divs:
// case instruction::i64_divu:
// case instruction::i64_rems:
// case instruction::i64_remu:
// case instruction::i64_and:
// case instruction::i64_or:
// case instruction::i64_xor:
// case instruction::i64_shl:
// case instruction::i64_shrs:
// case instruction::i64_shru:
// case instruction::i64_rotl:
// case instruction::i64_rotr:
// case instruction::f32_abs:
// case instruction::f32_neg:
// case instruction::f32_ceil:
// case instruction::f32_floor:
// case instruction::f32_trunc:
// case instruction::f32_nearest:
// case instruction::f32_sqrt:
// case instruction::f32_add:
// case instruction::f32_sub:
// case instruction::f32_mul:
// case instruction::f32_div:
// case instruction::f32_min:
// case instruction::f32_max:
// case instruction::f32_copysign:
// case instruction::f64_abs:
// case instruction::f64_neg:
// case instruction::f64_ceil:
// case instruction::f64_floor:
// case instruction::f64_trunc:
// case instruction::f64_nearest:
// case instruction::f64_sqrt:
// case instruction::f64_add:
// case instruction::f64_sub:
// case instruction::f64_mul:
// case instruction::f64_div:
// case instruction::f64_min:
// case instruction::f64_max:
// case instruction::f64_copysign:
// case instruction::i32_wrap_i64:
// case instruction::i32_trunc_sf32:
// case instruction::i32_trunc_uf32:
// case instruction::i32_trunc_sf64:
// case instruction::i32_trunc_uf64:
// case instruction::i64_extend_si32:
// case instruction::i64_extend_ui32:
// case instruction::i64_trunc_sf32:
// case instruction::i64_trunc_uf32:
// case instruction::i64_trunc_sf64:
// case instruction::i64_trunc_uf64:
// case instruction::f32_convert_si32:
// case instruction::f32_convert_ui32:
// case instruction::f32_convert_si64:
// case instruction::f32_convert_ui64:
// case instruction::f32_demote_f64:
// case instruction::f64_convert_si32:
// case instruction::f64_convert_ui32:
// case instruction::f64_convert_si64:
// case instruction::f64_convert_ui64:
// case instruction::f64_promote_f32:
// case instruction::i32_reinterpret_f32:
// case instruction::i64_reinterpret_f64:
// case instruction::f32_reinterpret_i32:
// case instruction::f64_reinterpret_i64:
