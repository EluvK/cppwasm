#pragma once
#include "../cppwasm/wasi-binary.h"
#include "assert.h"

class LittleEndian {
public:
    static int32_t i32(byte_vec const & bv) {
        assert(bv.size() == 4);
        int32_t res{}, t{};
        int offset = 0;
        for (auto & b : bv) {
            res |= ((t | b) << offset);
            offset += 8;
        }
        return res;
    }
    static int64_t i64(byte_vec const & bv) {
        assert(bv.size() == 8);
        int64_t res{}, t{};
        int offset = 0;
        for (auto & b : bv) {
            res |= ((t | b) << offset);
            offset += 8;
        }
        return res;
    }
    static uint32_t u32(byte_vec const & bv) {
        assert(bv.size() == 4);
        uint32_t res{}, t{};
        int offset = 0;
        for (auto & b : bv) {
            res |= ((t | b) << offset);
            offset += 8;
        }
        return res;
    }
    static uint64_t u64(byte_vec const & bv) {
        assert(bv.size() == 8);
        uint64_t res{}, t{};
        int offset = 0;
        for (auto & b : bv) {
            res |= ((t | b) << offset);
            offset += 8;
        }
        return res;
    }

    static byte_vec pack_i32(int32_t i32) {
        byte_vec res{};
        for (auto index = 0; index < 4; ++index) {
            res.push_back(i32 & 0xff);
            i32 >>= 8;
        }
        return res;
    }
    static byte_vec pack_i64(int64_t i64) {
        byte_vec res{};
        for (auto index = 0; index < 8; ++index) {
            res.push_back(i64 & 0xff);
            i64 >>= 8;
        }
        return res;
    }
    static byte_vec pack_u32(uint32_t u32) {
        byte_vec res{};
        for (auto index = 0; index < 4; ++index) {
            res.push_back(u32 & 0xff);
            u32 >>= 8;
        }
        return res;
    }
    static byte_vec pack_u64(uint64_t u64) {
        byte_vec res{};
        for (auto index = 0; index < 8; ++index) {
            res.push_back(u64 & 0xff);
            u64 >>= 8;
        }
        return res;
    }

    static float f32(byte_vec const & bv) {
        assert(bv.size() == 4);
        char p[4] = {};
        for (auto index = 0; index < bv.size(); index++) {
            p[index] = bv[index];
        }
        float res = *(float *)p;
        return res;
    }
    static double f64(byte_vec const & bv) {
        assert(bv.size() == 8);
        char p[8] = {};
        for (auto index = 0; index < bv.size(); index++) {
            p[index] = bv[index];
        }
        double res = *(double *)p;
        return res;
    }
    static byte_vec pack_f32(float f32) {
        byte_vec res{};
        char * p = (char *)&f32;
        for (int index = 0; index < 4; ++index) {
            res.push_back(*p++);
        }
        return res;
    }
    static byte_vec pack_f64(double f64) {
        byte_vec res{};
        char * p = (char *)&f64;
        for (int index = 0; index < 8; ++index) {
            res.push_back(*p++);
        }
        return res;
    }
};
