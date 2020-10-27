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
        res += ((b & 0x7f) << offset);
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
        res += ((b & 0x7f) << offset);
        offset += 7;
        last_byte = b;
    }
    if (last_byte & 0x40) {
        res |= -(1 << offset);
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