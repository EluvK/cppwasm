#pragma once
#include "base/wasi-define.h"

#include "wasi-leb128.h"
#include "wasi-section.h"

class BlockType {
public:
    BlockType(byte i) : data{i} {
    }
    byte data;
};

class LabelIndex {
public:
    LabelIndex(uint64_t i) : data{i} {
    }
    uint64_t data;
};

// base for ptr.
class instruction_args_base {};

class args_block : public instruction_args_base {
public:
    args_block(BlockType bt) : data(bt) {
    }

    BlockType data;
};

class args_br : public instruction_args_base {
public:
    args_br(LabelIndex li) : data(li) {
    }
    LabelIndex data;
};

class args_br_table : public instruction_args_base {
public:
    args_br_table(std::vector<LabelIndex> li_list, LabelIndex li) : data(li_list, li) {
    }

    std::pair<std::vector<LabelIndex>, LabelIndex> data;
};

using args_ptr = std::shared_ptr<instruction_args_base>;
/**
 * @brief Instructions are encoded by opcodes. Each opcode is represented by a single byte, and is followed by the
 *        instruction's immediate arguments, where present. The only exception are structured control instructions, which
 *        consist of several opcodes bracketing their nested instruction sequences.
 */
class Instruction {
public:
    Instruction(){};

    byte opcode;
    std::vector<args_ptr> args;
    // args
    // std::vector<Ins_args_ptr>
};

static Instruction GetInstruction(byte_IO & BinaryIO) {
    Instruction o;
    o.opcode = BinaryIO.read_one();
    o.args = {};
    switch (o.opcode) {
    case instruction::unreachable:
    case instruction::nop: {
        xdbg("unreachable nop?");
        break;
    }
    case instruction::block:
    case instruction::loop:
    case instruction::if_: {
        auto bt = BlockType(BinaryIO.read_one());
        o.args.push_back(std::make_shared<args_block>(bt));
        break;
    }
    case instruction::br:
    case instruction::br_if: {
        auto li = LabelIndex(U_decode_reader(BinaryIO));
        o.args.push_back(std::make_shared<args_br>(li));
        break;
    }
    case instruction::br_table: {
        auto n = U_decode_reader(BinaryIO);
        auto li = LabelIndex(U_decode_reader(BinaryIO));
        auto li_list = std::vector<LabelIndex>(n, li);
        auto b = LabelIndex(U_decode_reader(BinaryIO));
        o.args.push_back(std::make_shared<args_br_table>(li_list, b));
        break;
    }
    case instruction::call:

    case instruction::call_indirect:

    case instruction::get_local:
    case instruction::set_local:
    case instruction::tee_local:

    case instruction::get_global:
    case instruction::set_global:

    case instruction::i32_load:
    case instruction::i64_load:
    case instruction::f32_load:
    case instruction::f64_load:
    case instruction::i32_load8_s:
    case instruction::i32_load8_u:
    case instruction::i32_load16_s:
    case instruction::i32_load16_u:
    case instruction::i64_load8_s:
    case instruction::i64_load8_u:
    case instruction::i64_load16_s:
    case instruction::i64_load16_u:
    case instruction::i64_load32_s:
    case instruction::i64_load32_u:
    case instruction::i32_store:
    case instruction::i64_store:
    case instruction::f32_store:
    case instruction::f64_store:
    case instruction::i32_store8:
    case instruction::i32_store16:
    case instruction::i64_store8:
    case instruction::i64_store16:
    case instruction::i64_store32:

    case instruction::current_memory:
    case instruction::grow_memory:

    case instruction::i32_const:

    case instruction::i64_const:

    case instruction::f32_const:

    case instruction::f64_const:
        break;
    }
    return o;
};


/**
 * @brief Function bodies, initialization values for globals, and offsets of element or data segments are given as
 *        expressions, which are sequences of instructions terminated by an end marker.
 * 
 *        expr ::= instr∗ 0x0B
 * 
 *        In some places, validation restricts expressions to be constant, which limits the set of allowable instructions.
 */
class Expression {
public:
    Expression() {
    }

    std::vector<Instruction> data;
    //todo make position
};
static Expression GetExpression(byte_IO BinaryIO) {
    Expression exp{};
    uint32_t d = 1;
    while(true){
        Instruction ins = GetInstruction(BinaryIO);
        exp.data.push_back(ins);
        switch (ins.opcode)
        {
            case instruction::block:
            case instruction::loop:
            case instruction::if_:
                d++;
                break;
            case instruction::end:
                d--;
                break;
            default:
            break;
        }
        if(!d) break;
    }
    if(exp.data.back().opcode!=instruction::end){
        xerror("cppwasm: expression did not end with 0x0b(END) ");
    }
    // todo make position;
    return exp;
}


class Module{
public:
    Module(byte_IO BinaryIO){
        if(BinaryIO.read(4)!=byte_vec{0x00,0x61,0x73,0x6d}){
            xerror("cppwasm : magic header not detected");
        }
        if(BinaryIO.read(4)!=byte_vec{0x01,0x00,0x00,0x00}){
            xerror("cppwasm : unknow wasm version");
        }
    };
    xwasi_custom_section_t Custom_section{};
    xwasi_type_section_t Type_section{};
    xwasi_import_section_t Import_section{};
    xwasi_function_section_t Function_section{};
    xwasi_table_section_t Table_section{};
    xwasi_memory_section_t Memory_section{};
    xwasi_global_section_t Global_section{};
    xwasi_export_section_t Export_section{};
    xwasi_start_section_t Start_section{};
    xwasi_element_section_t Element_section{};
    xwasi_code_section_t Code_section{};
    xwasi_data_section_t Data_section{};
};