#pragma once

#include "base/wasi-define.h"

class Value {
public:
    Value() {
    }
    Value(byte _type, byte_vec _data) {
        type = _type;
        data = _data;
    }
    int32_t to_i32();
    int64_t to_i64();
    float to_f32();
    double to_f64();

private:
    byte type;
    byte_vec data;
};

class Label {
public:
    Label(uint32_t _arity, uint32_t _continuation) : arity{_arity}, continuation{_continuation} {
    }

private:
    uint32_t arity;
    uint32_t continuation;
};

class Frame {
public:
    Frame() {
    }


    // module
    // local_list
    Expression expr;
    uint32_t arity;
};