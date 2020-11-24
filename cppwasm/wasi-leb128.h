#include "base/wasi-define.h"

static byte_vec U_encode(uint64_t i) {
    byte_vec res{};
    while (true) {
        byte b = i & 0x7f;
        i >>= 7;
        if (i == 0) {
            res.push_back(b);
            return res;
        }
        res.push_back(0x80 | b);
    }
}
static uint64_t U_decode(byte_vec const & bv) {
    uint64_t res{0};
    int32_t offset{0};
    for (auto & b : bv) {
        uint64_t u64 = b;
        res += ((u64 & 0x7f) << offset);
        offset += 7;
    }
    return res;
}

static uint64_t U_decode_reader(byte_IO & bio) {
    byte_vec bv;
    while (true) {
        byte b = bio.read_one();
        bv.push_back(b);
        if ((b & 0x80) == 0)
            break;
    }
    return U_decode(bv);
}

static byte_vec I_encode(int64_t i) {
    byte_vec res{};
    while (true) {
        byte b = i & 0x7f;
        i >>= 7;
        if ((i == 0 && ((b & 0x40) == 0)) || (i == -1 && ((b & 0x40) != 0))) {
            res.push_back(b);
            return res;
        }
        res.push_back(0x80 | b);
    }
}

static int64_t I_decode(byte_vec const & bv) {
    int64_t res{0};
    int32_t offset{0};
    byte last_byte{};
    for (auto & b : bv) {
        int64_t u64 = b;
        res += ((u64 & 0x7f) << offset);
        offset += 7;
        last_byte = b;
    }
    if (last_byte & 0x40) {
        res |= -((int64_t)1 << std::min(offset, 63));
    }

    return res;
}

static int64_t I_decode_reader(byte_IO & bio) {
    byte_vec bv;
    while (true) {
        byte b = bio.read_one();
        bv.push_back(b);
        if ((b & 0x80) == 0)
            break;
    }
    return I_decode(bv);
}

static byte_vec F32_encode(float f32) {
    byte_vec res{};
    char * p = (char *)&f32;
    for (int index = 0; index < 4; ++index) {
        res.push_back(*p++);
    }
    return res;
}
static float F32_decode(byte_vec bv) {
    char p[4] = {};
    for (auto index = 0; index < bv.size(); index++) {
        p[index] = bv[index];
    }
    float res = *(float *)p;
    return res;
}
static float F32_decode_reader(byte_IO & bio) {
    byte_vec bv;
    int cnt = 4;
    while (cnt--) {
        auto b = bio.read_one();
        bv.push_back(b);
    }
    return F32_decode(bv);
}

static byte_vec F64_encode(double f64) {
    byte_vec res{};
    char * p = (char *)&f64;
    for (int index = 0; index < 8; ++index) {
        res.push_back(*p++);
    }
    return res;
}
static double F64_decode(byte_vec bv) {
    char p[8] = {};
    for (auto index = 0; index < bv.size(); index++) {
        p[index] = bv[index];
    }
    double res = *(double *)p;
    return res;
}
static double F64_decode_reader(byte_IO & bio) {
    byte_vec bv;
    int cnt = 8;
    while (cnt--) {
        auto b = bio.read_one();
        bv.push_back(b);
    }
    return F64_decode(bv);
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