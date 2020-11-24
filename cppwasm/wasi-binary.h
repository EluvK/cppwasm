#pragma once
#include "base/Variant.h"
#include "base/wasi-define.h"
#include "wasi-leb128.h"

#include <fstream>
#include <cinttypes>
/**
 * ======================================================================================================================
 * Binary Format Index
 * ======================================================================================================================
 */

// class TypeIndex {
// public:
//     TypeIndex() {
//     }
//     TypeIndex(byte b) : data{b} {
//     }
//     static TypeIndex GetTypeIndex(byte_IO & BinaryIO) {
//         return TypeIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using TypeIndex = uint64_t;
static TypeIndex GetTypeIndex(byte_IO & BinaryIO) {
    return TypeIndex{U_decode_reader(BinaryIO)};
}

// class FunctionIndex {
// public:
//     FunctionIndex() {
//     }
//     FunctionIndex(byte b) : data{b} {
//     }
//     static FunctionIndex GetFunctionIndex(byte_IO & BinaryIO) {
//         return FunctionIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using FunctionIndex = uint64_t;
static FunctionIndex GetFunctionIndex(byte_IO & BinaryIO) {
    return FunctionIndex{U_decode_reader(BinaryIO)};
}

// class TableIndex {
// public:
//     TableIndex() {
//     }
//     TableIndex(byte b) : data{b} {
//     }
//     static TableIndex GetTableIndex(byte_IO & BinaryIO) {
//         return TableIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using TableIndex = uint64_t;
static TableIndex GetTableIndex(byte_IO & BinaryIO) {
    return TableIndex{U_decode_reader(BinaryIO)};
}

// class MemoryIndex {
// public:
//     MemoryIndex() {
//     }
//     MemoryIndex(byte b) : data{b} {
//     }
//     static MemoryIndex GetMemoryIndex(byte_IO & BinaryIO) {
//         return MemoryIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using MemoryIndex = uint64_t;
static MemoryIndex GetMemoryIndex(byte_IO & BinaryIO) {
    return MemoryIndex{U_decode_reader(BinaryIO)};
}

// class GlobalIndex {
// public:
//     GlobalIndex(byte b) : data{b} {
//     }
//     static GlobalIndex GetGlobalIndex(byte_IO & BinaryIO) {
//         return GlobalIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using GlobalIndex = uint64_t;
static GlobalIndex GetGlobalIndex(byte_IO & BinaryIO) {
    return GlobalIndex{U_decode_reader(BinaryIO)};
}

// class LocalIndex {
// public:
//     LocalIndex(byte b) : data{b} {
//     }
//     static LocalIndex GetLocalIndex(byte_IO & BinaryIO) {
//         return LocalIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };
using LocalIndex = uint64_t;
static LocalIndex GetLocalIndex(byte_IO & BinaryIO) {
    return LocalIndex{U_decode_reader(BinaryIO)};
}

// (delete)  may remove class? using LabelIndex = byte? other index?
// class LabelIndex {
// public:
//     LabelIndex() {
//     }
//     LabelIndex(byte b) : data{b} {
//     }
//     static LabelIndex GetLabelIndex(byte_IO & BinaryIO) {
//         return LabelIndex{U_decode_reader(BinaryIO)};
//     }
//     byte data;
// };

using LabelIndex = uint64_t;
static LabelIndex GetLabelIndex(byte_IO & BinaryIO) {
    return LabelIndex{U_decode_reader(BinaryIO)};
}

/**
 * ======================================================================================================================
 * Binary Format Types
 * ======================================================================================================================
 */

/**
 * @brief Value types are encoded by a single byte.
 * valtype ::= {
 *      0x7f: i32,
 *      0x7e: i64,
 *      0x7d: f32,
 *      0x7c: f64,
 * }
 */
using ValueType = byte;
static ValueType GetValueType(byte_IO & BinaryIO){
    return ValueType{BinaryIO.read_one()};
}
// class ValueType {
// public:
//     ValueType() {
//     }
//     ValueType(byte b) : data{b} {
//     }

//     static ValueType GetValueType(byte_IO & BinaryIO) {
//         return ValueType(BinaryIO.read_one());
//     }
//     bool operator==(ValueType const & other) const noexcept {
//         return data == other.data;
//     }
//     bool operator!=(ValueType const & other) const noexcept {
//         return !(*this == other);
//     }

//     byte data;
// };

/**
 * @brief Result types classify the result of executing instructions or blocks, which is a sequence of values written with brackets.
 *
 * resulttype ::= [valtype?]
 */
class ResultType {
public:
    ResultType() {
    }

    static ResultType GetResultType(byte_IO & BinaryIO) {
        ResultType res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(GetValueType(BinaryIO));
        }
        return res;
    }

    std::vector<ValueType> data;
};

/**
 * @brief Function types are encoded by the byte 0x60 followed by the respective vectors of parameter and result types.
 *
 * functype ::= 0x60 t1∗:vec(valtype) t2∗:vec(valtype) ⇒ [t1∗] → [t2∗]
 */
class FunctionType {
public:
    FunctionType() {
    }
    static FunctionType GetFunctionType(byte_IO & BinaryIO) {
        FunctionType res{};
        ASSERT(BinaryIO.read_one() == 0x60, "functionType sign wrong");
        res.args = ResultType::GetResultType(BinaryIO);
        res.rets = ResultType::GetResultType(BinaryIO);
        return res;
    }

    ResultType args;
    ResultType rets;
};

/**
 * @brief Limits are encoded with a preceding flag indicating whether a maximum is present.
 * limits ::= 0x00  n:u32        ⇒ {min n,max ϵ}
 *          | 0x01  n:u32  m:u32 ⇒ {min n,max m}
 *
 */
class Limits {
public:
    Limits() {
    }

    static Limits GetLimits(byte_IO & BinaryIO) {
        Limits res{};
        bool flag = BinaryIO.read_one();
        res.n = U_decode_reader(BinaryIO);
        if (flag)
            res.m = U_decode_reader(BinaryIO);
        return res;
    }

    int64_t n{0};
    int64_t m{0};
};

/**
 * @brief Memory types classify linear memories and their size range.
 * memtype ::= limits
 * The limits constrain the minimum and optionally the maximum size of a memory. The limits are given in units of page size.
 *
 */
class MemoryType {
public:
    MemoryType() {
    }

    static MemoryType GetMemoryType(byte_IO & BinaryIO) {
        MemoryType res{};
        res.limits = Limits::GetLimits(BinaryIO);
        return res;
    }

    Limits limits{};
};

/**
 * @brief The element type funcref is the infinite union of all function types. A table of that type thus contains
 * references to functions of heterogeneous type.
 * In future versions of WebAssembly, additional element types may be introduced.
 *
 */
using ElementType = byte;
static ElementType GetElementType(byte_IO & BinaryIO){
    return ElementType{BinaryIO.read_one()};
}
// class ElementType {
// public:
//     ElementType() {
//     }
//     ElementType(byte b) : data{b} {
//     }
//     static ElementType GetElementType(byte_IO & BinaryIO) {
//         return ElementType{BinaryIO.read_one()};
//     }
//     byte data;
// };

/**
 * @brief Table types classify tables over elements of element types within a size range.
 * tabletype ::= limits elemtype
 * elemtype ::= funcref
 *
 * Like memories, tables are constrained by limits for their minimum and optionally maximum size. The limits are
 * given in numbers of entries. The element type funcref is the infinite union of all function types. A table of that
 * type thus contains references to functions of heterogeneous type.
 *
 */
class TableType {
public:
    TableType() {
    }

    static TableType GetTableType(byte_IO & BinaryIO) {
        TableType res{};
        res.element_type = GetElementType(BinaryIO);
        res.limits = Limits::GetLimits(BinaryIO);
        return res;
    }

    ElementType element_type;
    Limits limits;
};

/**
 * @brief # Mut const | var
 */
using Mut = byte;
static Mut GetMut(byte_IO & BinaryIO){
    return Mut{BinaryIO.read_one()};
}
// class Mut {
// public:
//     Mut() {
//     }
//     Mut(byte b) : data{b} {
//     }
//     static Mut GetMut(byte_IO & BinaryIO) {
//         return Mut{BinaryIO.read_one()};
//     }
//     byte data;
// };

class GlobalType {
public:
    GlobalType() {
    }

    static GlobalType GetGlobalType(byte_IO & BinaryIO) {
        GlobalType res{};
        res.value_type = GetValueType(BinaryIO);
        res.mut = GetMut(BinaryIO);
        return res;
    }

    ValueType value_type;
    Mut mut;
};

/**
 * ======================================================================================================================
 * Binary Format Instructions
 * ======================================================================================================================
 */

/**
 * @brief Block types are encoded in special compressed form, by either the byte 0x40 indicating the empty type, as a
 * single value type, or as a type index encoded as a positive signed integer.
 * blocktype ::= 0x40
 *             | t: valtype
 *             | x: s33
 */
using BlockType = byte;
static BlockType GetBlockType(byte_IO & BinaryIO){
    return BlockType{BinaryIO.read_one()};
}
// class BlockType {
// public:
//     BlockType() {
//     }

//     static BlockType GetBlockType(byte_IO & BinaryIO) {
//         BlockType res{};
//         res.data = BinaryIO.read_one();
//         return res;
//     }
//     byte data;
// };

//----------------------
// mywork

// base for ptr.
class instruction_args_base {
public:
    virtual ~instruction_args_base() {
    }
};

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

class args_get_local : public instruction_args_base {
public:
    args_get_local(LabelIndex li) : data(li) {
    }

    LabelIndex data;
};

class args_get_global : public instruction_args_base {
public:
    args_get_global(GlobalIndex li) : data(li) {
    }

    GlobalIndex data;
};

class args_memory : public instruction_args_base {
public:
    args_memory(byte b) : data{b} {
    }

    byte data;
};

class args_i32_count : public instruction_args_base {
public:
    args_i32_count(int32_t i) : data(i) {
    }
    int32_t data;
};

class args_i64_count : public instruction_args_base {
public:
    args_i64_count(int64_t i) : data(i) {
    }
    int64_t data;
};

class args_f32_count : public instruction_args_base {
public:
    args_f32_count(float i) : data(i) {
    }
    float data;
};

class args_f64_count : public instruction_args_base {
public:
    args_f64_count(double i) : data(i) {
    }
    double data;
};

class args_call : public instruction_args_base {
public:
    args_call(FunctionIndex fi) : data(fi) {
    }
    FunctionIndex data;
};

class args_call_indirect : public instruction_args_base {
public:
    args_call_indirect(TypeIndex ti, byte _n) : data{std::make_pair(ti, _n)} {
    }

    std::pair<TypeIndex, byte> data;
};

class args_load_store : public instruction_args_base {
public:
    args_load_store(std::pair<uint64_t, uint64_t> pi) : data{pi} {
    }

    std::pair<uint64_t, uint64_t> data;
};
using args_ptr = std::shared_ptr<instruction_args_base>;
//--------------

/**
 * @brief Instructions are encoded by opcodes. Each opcode is represented by a single byte, and is followed by the
 *        instruction's immediate arguments, where present. The only exception are structured control instructions, which
 *        consist of several opcodes bracketing their nested instruction sequences.
 */
class Instruction {
public:
    Instruction(){};

    byte opcode;
    args_ptr args;

    static Instruction GetInstruction(byte_IO & BinaryIO) {
        Instruction o;
        o.opcode = BinaryIO.read_one();
        o.args = {};
        switch (o.opcode) {
        // case instruction::unreachable:
        //     xerror("cppwasm: not support unreachable");
        // case instruction::nop: {
        //     xdbg("nop pass!");
        //     break;
        // }
        case instruction::block:
        case instruction::loop:
        case instruction::if_: {
            auto bt = GetBlockType(BinaryIO);
            o.args = std::make_shared<args_block>(bt);
            break;
        }
        case instruction::br:
        case instruction::br_if: {
            auto li = LabelIndex(U_decode_reader(BinaryIO));
            o.args = std::make_shared<args_br>(li);
            break;
        }
        case instruction::br_table: {
            auto n = U_decode_reader(BinaryIO);
            auto li_list = std::vector<LabelIndex>{};
            for (auto index = 0; index < n; ++index) {
                auto li = LabelIndex(U_decode_reader(BinaryIO));
                li_list.push_back(li);
            }
            auto b = LabelIndex(U_decode_reader(BinaryIO));
            o.args = std::make_shared<args_br_table>(li_list, b);
            break;
        }
        case instruction::call: {
            auto fi = FunctionIndex(U_decode_reader(BinaryIO));
            o.args = std::make_shared<args_call>(fi);
            break;
        }
        case instruction::call_indirect: {
            auto ti = TypeIndex(U_decode_reader(BinaryIO));
            byte n = BinaryIO.read_one();
            o.args = std::make_shared<args_call_indirect>(ti, n);
            break;
        }

        case instruction::get_local:
        case instruction::set_local:
        case instruction::tee_local: {
            auto li = LabelIndex(U_decode_reader(BinaryIO));
            o.args = std::make_shared<args_get_local>(li);
            break;
        }

        case instruction::get_global:
        case instruction::set_global: {
            auto gi = GlobalIndex(U_decode_reader(BinaryIO));
            o.args = std::make_shared<args_get_global>(gi);
            break;
        }

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
        case instruction::i64_store32: {
            uint64_t _f = U_decode_reader(BinaryIO);
            uint64_t _s = U_decode_reader(BinaryIO);
            std::pair<uint64_t, uint64_t> pi = std::make_pair(_f, _s);
            // xdbg("%02x %02x", pi.first, pi.second);
            o.args = std::make_shared<args_load_store>(pi);
            break;
        }

        case instruction::current_memory:
        case instruction::grow_memory:{
            byte n = BinaryIO.read_one();
            o.args = std::make_shared<args_memory>(n);
            break;
        }
        case instruction::i32_const:{
            auto i32 = I_decode_reader(BinaryIO);
            o.args = std::make_shared<args_i32_count>(i32);
            break;
        }
        case instruction::i64_const: {
            auto i64 = I_decode_reader(BinaryIO);
            o.args = std::make_shared<args_i64_count>(i64);
            break;
        }

        case instruction::f32_const:{
            auto f32 = F32_decode_reader(BinaryIO);
            o.args = std::make_shared<args_f32_count>(f32);
            break;
        }

        case instruction::f64_const:{
            auto f64 = F64_decode_reader(BinaryIO);
            o.args = std::make_shared<args_f64_count>(f64);
            break;
        }
        default:
            // xdbg("unknow instruction: 0x%02x", o.opcode);
            break;
        }
        return o;
    };
};

/**
 * ======================================================================================================================
 * Binary Format Modules
 * ======================================================================================================================
 */

/**
 * @brief # custom ::= name byte∗
 */
class Custom {
public:
    Custom() {
    }

    static Custom GetCustom(byte_IO & BinaryIO) {
        Custom res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.name += BinaryIO.read_one();
        }
        // todo ? what is custom data. unknow for now.
        return res;
    }
    std::string name;
    byte_vec data;
};

/**
 * @brief Custom sections have the id 0. They are intended to be used for debugging
 * information or third-party extensions, and are ignored by the WebAssembly
 * semantics. Their contents consist of a name further identifying the custom
 * section, followed by an uninterpreted sequence of bytes for custom use.
 *
 * customsec ::= section0(custom)
 * custom ::= name byte∗
 *
 */
class CustomSection {
public:
    CustomSection() {
    }

    static CustomSection GetCustomSection(byte_IO & BinaryIO) {
        CustomSection res{};
        res.data = Custom::GetCustom(BinaryIO);
        return res;
    }

    Custom data;
};

class TypeSection {
public:
    TypeSection() {
    }

    static TypeSection GetTypeSection(byte_IO & BinaryIO) {
        TypeSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(FunctionType::GetFunctionType(BinaryIO));
        }
        return res;
    }

    std::vector<FunctionType> data;
};

/**
 * @brief The imports component of a module defines a set of imports that are required for instantiation.
 *
 * import ::= {module name, name name, desc importdesc}
 * importdesc ::= func typeidx | table tabletype | mem memtype | global globaltype
 *
 * Each import is labeled by a two-level name space, consisting of a module name and a name for an entity within
 * that module. Importable definitions are functions, tables, memories, and globals. Each import is specified by a
 * descriptor with a respective type that a definition provided during instantiation is required to match. Every
 * import defines an index in the respective index space. In each index space, the indices of imports go before the
 * first index of any definition contained in the module itself.
 *
 */
class Import {
public:
    Import() {
    }

    static Import GetImport(byte_IO & BinaryIO) {
        Import res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.module += BinaryIO.read_one();
        }
        size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.name += BinaryIO.read_one();
        }
        res.desc = BinaryIO.read_one();
        switch (res.desc) {
        // extern_function = 0x00
        // extern_table = 0x01
        // extern_memory = 0x02
        // extern_global = 0x03
        case 0:
            res.importdesc = GetTypeIndex(BinaryIO);
            break;
        case 1:
            res.importdesc = TableType::GetTableType(BinaryIO);
            break;
        case 2:
            res.importdesc = MemoryType::GetMemoryType(BinaryIO);
            break;
        case 3:
            res.importdesc = GlobalType::GetGlobalType(BinaryIO);
            break;
        default:
            xerror("cppwasm: something bug ? %d", res.desc);
        }
        return res;
    }
    std::string module;
    std::string name;
    byte desc;
    Variant<TypeIndex, TableType, MemoryType, GlobalType> importdesc;
};

/**
 * @brief The import section has the id 2. It decodes into a vector of imports
 * that represent the imports component of a module.
 *
 * importsec ::= im∗:section2(vec(import)) ⇒ im∗
 * import ::= mod:name nm:name d:importdesc ⇒ {module mod, name nm, desc d}
 * importdesc ::= 0x00 x:typeidx ⇒ func x
 *              | 0x01 tt:tabletype ⇒ table tt
 *              | 0x02 mt:memtype ⇒ mem mt
 *              | 0x03 gt:globaltype ⇒ global gt
 */
class ImportSection {
public:
    ImportSection() {
    }

    static ImportSection GetImportSection(byte_IO & BinaryIO) {
        ImportSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Import::GetImport(BinaryIO));
        }
        return res;
    }
    std::vector<Import> data;
};

/**
 * @brief
 * The function section has the id 3. It decodes into a vector of type
 * indices that represent the type fields of the functions in the funcs
 * component of a module. The locals and body fields of the respective
 * functions are encoded separately in the code section.
 * funcsec ::= x∗:section3(vec(typeidx)) ⇒ x∗
 */
class FunctionSection {
public:
    FunctionSection() {
    }

    static FunctionSection GetFunctionSection(byte_IO & BinaryIO) {
        FunctionSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(GetTypeIndex(BinaryIO));
        }
        return res;
    }
    std::vector<TypeIndex> data;
};

/**
 * @brief The tables component of a module defines a vector of tables described by their table type:
 * table ::= {type tabletype}
 */
class Table {
public:
    Table() {
    }

    static Table GetTable(byte_IO & BinaryIO) {
        Table res{};
        res.type = TableType::GetTableType(BinaryIO);
        return res;
    }
    TableType type;
};

/**
 * @brief The table section has the id 4. It decodes into a vector of tables that
 * represent the tables component of a module.
 *
 * tablesec ::= tab∗:section4(vec(table)) ⇒ tab∗
 * table ::= tt:tabletype ⇒ {type tt}
 *
 */
class TableSection {
public:
    TableSection() {
    }

    static TableSection GetTableSection(byte_IO & BinaryIO) {
        TableSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Table::GetTable(BinaryIO));
        }
        return res;
    }
    std::vector<Table> data;
};

/**
 * @brief The mems component of a module defines a vector of linear memories (or memories for short) as described by their memory type:
 * mem ::= {type memtype}
 */
class Memory {
public:
    Memory() {
    }

    static Memory GetMemory(byte_IO & BinaryIO) {
        Memory res{};
        res.type = MemoryType::GetMemoryType(BinaryIO);
        return res;
    }
    MemoryType type;
};

/**
 * @brief The memory section has the id 5. It decodes into a vector of memories
 * that represent the mems component of a module.
 * memsec ::= mem∗:section5(vec(mem)) ⇒ mem∗
 * mem ::= mt:memtype ⇒ {type mt}
 *
 */
class MemorySection {
public:
    MemorySection() {
    }

    static MemorySection GetMemorySection(byte_IO & BinaryIO) {
        MemorySection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Memory::GetMemory(BinaryIO));
        }
        return res;
    }
    std::vector<Memory> data;
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
    std::map<int32_t, std::vector<int32_t>> position;

    static std::map<int32_t, std::vector<int32_t>> mark(std::vector<Instruction> const & data) {
        // for(auto _I:data){
        //     xdbg("0x%02x", _I.opcode);
        // }
        std::vector<std::vector<int32_t>> st;
        std::map<int32_t, std::vector<int32_t>> _position;
        for (auto index = 0; index < data.size(); ++index) {
            auto & _instruction = data[index];
            auto & _opcode = _instruction.opcode;

            if (_instruction.opcode == instruction::block || _instruction.opcode == instruction::loop || _instruction.opcode == instruction::if_) {
                // xdbg(" mark: push");
                st.push_back({index});
            } else if (_opcode == instruction::else_) {
                st.back().push_back(index);
            } else if (_opcode == instruction::end) {
                // xdbg(" mark: end");
                if (!st.empty()) {
                    // xdbg(" mark: end - pop");
                    std::vector<int32_t> b = st.back();
                    st.pop_back();
                    b.insert(b.begin() + 1, index);
                    for (auto _i : b) {
                        _position[_i] = b;
                    }
                }
            }
        }
        ASSERT(st.empty(), "cppwasm: tmp stack not empty. %d", st.size());
        return _position;
    }

    static Expression GetExpression(byte_IO & BinaryIO) {
        Expression exp{};
        uint32_t d = 1;
        while (true) {
            Instruction ins = Instruction::GetInstruction(BinaryIO);
            exp.data.push_back(ins);
            // xdbg(" GETexpression: ins: opcode:0x%02x", ins.opcode);
            switch (ins.opcode) {
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
            if (!d)
                break;
        }
        if (exp.data.back().opcode != instruction::end) {
            xerror("cppwasm: expression did not end with 0x0b(END) ");
        }
        exp.position = mark(exp.data);
        // // ! debug
        // for(auto _v:exp.position){
        //     printf("\n{ %d : [ ", _v.first);
        //     for (auto u : _v.second)
        //         printf("%d ", u);
        //     printf("]}");
        // }
        return exp;
    }
};

/**
 * @brief The globals component of a module defines a vector of global variables (or globals for short):
 * global ::= {type globaltype, init expr}
 */
class Global {
public:
    Global() {
    }

    static Global GetGlobal(byte_IO & BinaryIO) {
        Global res{};
        res.type = GlobalType::GetGlobalType(BinaryIO);
        res.expr = Expression::GetExpression(BinaryIO);
        return res;
    }

    GlobalType type;
    Expression expr;
};

/**
 * @brief The global section has the id 6. It decodes into a vector of globals
 * that represent the globals component of a module.
 *
 * globalsec ::= glob*:section6(vec(global)) ⇒ glob∗
 * global ::= gt:globaltype e:expr ⇒ {type gt, init e}
 *
 */
class GlobalSection {
public:
    GlobalSection() {
    }

    static GlobalSection GetGlobalSection(byte_IO & BinaryIO) {
        GlobalSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Global::GetGlobal(BinaryIO));
        }
        return res;
    }

    std::vector<Global> data;
};

#define EXPORT_TYPE_FUNC 0
#define EXPORT_TYPE_TABLE 1
#define EXPORT_TYPE_MEMORY 2
#define EXPORT_TYPE_GLOBAL 3
/**
 * @brief The exports component of a module defines a set of exports that become accessible to the host environment once the module has been instantiated.
 * export ::= {name name, desc exportdesc}
 * exportdesc ::= func funcidx | table tableidx | mem memidx | global globalidx
 *
 * Each export is labeled by a unique name. Exportable definitions are functions, tables, memories, and globals,
 * which are referenced through a respective descriptor.
 *
 */
class Export {
public:
    Export() {
    }

    static Export GetExport(byte_IO & BinaryIO) {
        Export res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.name += BinaryIO.read_one();
        }
        res.type = BinaryIO.read_one();
        switch (res.type) {
            // extern_function = 0x00
            // extern_table = 0x01
            // extern_memory = 0x02
            // extern_global = 0x03
        case EXPORT_TYPE_FUNC:
            res.exportdesc = GetFunctionIndex(BinaryIO);
            break;
        case EXPORT_TYPE_TABLE:
            res.exportdesc = GetTableIndex(BinaryIO);
            break;
        case EXPORT_TYPE_MEMORY:
            res.exportdesc = GetMemoryIndex(BinaryIO);
            break;
        case EXPORT_TYPE_GLOBAL:
            res.exportdesc = GetGlobalIndex(BinaryIO);
            break;
        default:
            xerror("cppwasm: something bug ? %d", res.type);
        }
        xdbg(" export_ name:%s  type:%d  index:%d", res.name.c_str(), res.type, res.exportdesc);
        return res;
    }
    std::string name;
    byte type;
    uint64_t exportdesc;
};

/**
 * @brief The export section has the id 7. It decodes into a vector of exports that represent the exports component of a module.
 * exportsec ::= ex∗:section7(vec(export)) ⇒ ex∗
 * export :: =nm:name d:exportdesc ⇒ {name nm, desc d}
 * exportdesc ::= 0x00 x:funcidx ⇒ func x
 *              | 0x01 x:tableidx ⇒ table x
 *              | 0x02 x:memidx ⇒ mem x
 *              | 0x03 x:globalidx⇒global x
 *
 */
class ExportSection {
public:
    ExportSection() {
    }

    static ExportSection GetExportSection(byte_IO & BinaryIO) {
        ExportSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        xdbg("export_section size",size);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Export::GetExport(BinaryIO));
        }
        return res;
    }
    std::vector<Export> data;
};

/**
 * @brief  The start component of a module declares the function index of a start function that is automatically invoked
 * when the module is instantiated, after tables and memories have been initialized.
 *
 * start ::= {func funcidx}
 */
class StartFunction {
public:
    StartFunction() {
    }
    static StartFunction GetStartFunction(byte_IO & BinaryIO) {
        StartFunction res{};
        res.function_index = GetFunctionIndex(BinaryIO);
        res.exit = true;
        return res;
    }
    bool exit{false};
    FunctionIndex function_index;
};

/**
 * @brief The start section has the id 8. It decodes into an optional start
 * function that represents the start component of a module.
 *
 * startsec ::= st?:section8(start) ⇒ st?
 * start ::= x:funcidx ⇒ {func x}
 *
 */
class StartSection {
public:
    StartSection() {
    }
    static StartSection GetStartSection(byte_IO & BinaryIO) {
        StartSection res{};
        res.start = StartFunction::GetStartFunction(BinaryIO);
        return res;
    }
    StartFunction start;
};

class Element {
public:
    Element() {
    }
    static Element GetElement(byte_IO & BinaryIO) {
        Element res{};
        res.table_index = GetTableIndex(BinaryIO);
        res.offset = Expression::GetExpression(BinaryIO);
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.init.push_back(GetFunctionIndex(BinaryIO));
        }
        return res;
    }
    TableIndex table_index;
    Expression offset;
    std::vector<FunctionIndex> init;
};

/**
 * @brief The element section has the id 9. It decodes into a vector of element
 * segments that represent the elem component of a module.
 *
 * elemsec ::= seg∗:section9(vec(elem)) ⇒ seg
 * elem ::= x:tableidx e:expr y∗:vec(funcidx) ⇒ {table x, offset e, init y∗}
 *
 */
class ElementSection {
public:
    ElementSection() {
    }

    static ElementSection GetElementSection(byte_IO & BinaryIO) {
        ElementSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Element::GetElement(BinaryIO));
        }
        return res;
    }
    std::vector<Element> data;
};

/**
 * @brief  The locals declare a vector of mutable local variables and their types. These variables are referenced through
 * local indices in the function’s body. The index of the first local is the smallest index not referencing a parameter.
 *
 * locals ::= n:u32 t:valtype ⇒ tn
 */
class Locals {
public:
    Locals() {
    }

    static Locals GetLocals(byte_IO & BinaryIO) {
        Locals res{};
        res.n = U_decode_reader(BinaryIO);
        if (res.n > 0x10000000) {
            xerror("cppwasm: too many locals");
        }
        res.type = GetValueType(BinaryIO);
        return res;
    }

    int64_t n{0};
    ValueType type;
};

class Func {
public:
    Func() {
    }
    static Func GetFunc(byte_IO & BinaryIO) {
        Func res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            // local decl count
            res.local_list.push_back(Locals::GetLocals(BinaryIO));
        }
        res.expr = Expression::GetExpression(BinaryIO);
        return res;
    }
    std::vector<Locals> local_list;
    Expression expr;
};

/**
 * @brief The encoding of each code entry consists of
 * - the u32 size of the function code in bytes
 * - the actual function code, which in turn consists of
 * - the declaration of locals
 * - the function body as an expression.
 *
 * Local declarations are compressed into a vector whose entries consist of
 * - a u32 count
 * - a value type.
 *
 * code ::= size:u32 code:func ⇒ code(ifsize=||func||)
 * func ::= (t∗)∗:vec(locals) e:expr ⇒ concat((t∗)∗), e∗(if|concat((t∗)∗)|<232)
 * locals ::= n:u32 t:valtype ⇒ tn
 *
 */
class Code {
public:
    Code() {
    }

    static Code GetCode(byte_IO & BinaryIO) {
        Code res{};
        res.size = U_decode_reader(BinaryIO);
        byte_IO code_bytes{BinaryIO.read(res.size)};
        // ! for debug
        // printf("size: 0x%02x ", res.size);
        for (auto _b : code_bytes.data) {
            printf("0x%02x ", _b);
        }
        printf("\n");
        res.func = Func::GetFunc(code_bytes);
        ASSERT(code_bytes.empty(), "should be empty to be get all code.");
        return res;
    }
    uint64_t size{0};
    Func func;
};

/**
 * @brief The code section has the id 10. It decodes into a vector of code
 * entries that are pairs of value type vectors and expressions. They
 * represent the locals and body field of the functions in the funcs
 * component of a module. The type fields of the respective functions are
 * encoded separately in the function section.
 *
 */
class CodeSection {
public:
    CodeSection() {
    }
    static CodeSection GetCodeSection(byte_IO & BinaryIO) {
        CodeSection res{};
        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Code::GetCode(BinaryIO));
        }
        return res;
    }
    std::vector<Code> data;
};

/**
 * @brief The initial contents of a memory are zero-valued bytes. The data component of a module defines a vector of data
 * segments that initialize a range of memory, at a given offset, with a static vector of bytes.
 *
 * data::={data memidx,offset expr,init vec(byte)}
 *
 * The offset is given by a constant expression.
 */
class Data {
public:
    Data() {
    }
    static Data GetData(byte_IO & BinaryIO) {
        Data res{};
        res.memory_index = GetMemoryIndex(BinaryIO);
        res.offset = Expression::GetExpression(BinaryIO);
        uint64_t size = U_decode_reader(BinaryIO);
        res.init = BinaryIO.read(size);
        ASSERT(res.init.size() == size, "cppwasm : unexpected end of section of function");
        return res;
    }
    MemoryIndex memory_index;
    Expression offset;
    byte_vec init;
};

/**
 * @brief The data section has the id 11. It decodes into a vector of data
 * segments that represent the data component of a module.
 *
 * datasec ::= seg∗:section11(vec(data)) ⇒ seg
 * data ::= x:memidx e:expr b∗:vec(byte) ⇒ {data x,offset e,init b∗}
 *
 */
class DataSection {
public:
    DataSection() {
    }
    static DataSection GetDataSection(byte_IO & BinaryIO) {
        DataSection res{};

        uint64_t size = U_decode_reader(BinaryIO);
        for (auto index = 0; index < size; ++index) {
            res.data.push_back(Data::GetData(BinaryIO));
        }
        return res;
    }
    std::vector<Data> data;
};

/**
 * @brief  The funcs component of a module defines a vector of functions with the following structure:
 *
 * func ::= {type typeidx, locals vec(valtype), body expr}
 *
 * The type of a function declares its signature by reference to a type defined in the module. The parameters of the
 * function are referenced through 0-based local indices in the function’s body; they are mutable.
 *
 * The locals declare a vector of mutable local variables and their types. These variables are referenced through
 * local indices in the function’s body. The index of the first local is the smallest index not referencing a parameter.
 *
 * The body is an instruction sequence that upon termination must produce a stack matching the function type’s result type.
 *
 * Functions are referenced through function indices, starting with the smallest index not referencing a function import.
 *
 */
class Function {
public:
    Function() {
    }
    // static Function GetFunction(byte_IO & BinaryIO){
    //     Function res{};

    //     return res;
    // }

    TypeIndex type_index;
    std::vector<ValueType> local_list;
    Expression expr;
};

/**
 * @brief The binary encoding of modules is organized into sections. Most sections correspond to one component of a module
 * record, except that function definitions are split into two sections, separating their type declarations in the
 * function section from their bodies in the code section.
 * 
 */
class Module {
public:
    Module(){};
    Module(const char * file_path) {
        unsigned char * bytes;
        uint32_t bytes_size;

        std::ifstream file_size(file_path, std::ifstream::ate | std::ifstream::binary);
        bytes_size = file_size.tellg();
        file_size.close();

        byte_vec data{};
        data.reserve(bytes_size);

        ASSERT(bytes_size > 0, "wrong file.");

        std::ifstream in(file_path, std::ifstream::binary);
        bytes = (uint8_t *)malloc(bytes_size);
        in.read(reinterpret_cast<char *>(bytes), bytes_size);
        xdbg("file:%s size: %d", file_path, bytes_size);
        int cnt = 0;
        for (auto index = 0; index < bytes_size; ++index) {
            data.push_back(*(bytes + index));
            // std::printf("%02x ", *(bytes + index));
            // fflush(stdout);
            // if (++cnt == 4) {
            // std::printf("\n");fflush(stdout);
            // cnt = 0;
            // }
        }
        in.close();
        byte_IO BinaryIO{data};

        if (BinaryIO.read(4) != byte_vec{0x00, 0x61, 0x73, 0x6d}) {
            xerror("cppwasm: magic header not detected");
        }
        if (BinaryIO.read(4) != byte_vec{0x01, 0x00, 0x00, 0x00}) {
            xerror("cppwasm: unknow wasm version");
        }
        xdbg("----------- Module init ------------");
        while (!BinaryIO.empty()) {
            byte index = BinaryIO.read_one();
            auto sz = U_decode_reader(BinaryIO);
            byte_IO sectionIO = BinaryIO.read(sz);
            ASSERT(sz == sectionIO.size(), "cppwasm: unexpected end of section or function");
            // xdbg("index: %d size: %d", index, sz);
            switch (index) {
            case CUSTOM_SECTION_INDEX:
                xdbg("distribute Custom section(%d): size: %d", CUSTOM_SECTION_INDEX, sz);
                Custom_section = CustomSection::GetCustomSection(sectionIO);
                break;
            case TYPE_SECTION_INDEX:
                xdbg("distribute TYPE section(%d): size: %d", TYPE_SECTION_INDEX, sz);
                Type_section = TypeSection::GetTypeSection(sectionIO);
                type_list = Type_section.data;
                break;
            case IMPORT_SECTION_INDEX:
                xdbg("distribute IMPORT section(%d): size: %d", IMPORT_SECTION_INDEX, sz);
                Import_section = ImportSection::GetImportSection(sectionIO);
                import_list = Import_section.data;
                break;
            case FUNCTION_SECTION_INDEX:
                xdbg("distribute FUNCTION section(%d): size: %d", FUNCTION_SECTION_INDEX, sz);
                Function_section = FunctionSection::GetFunctionSection(sectionIO);
                break;
            case TABLE_SECTION_INDEX:
                xdbg("distribute TABLE section(%d): size: %d", TABLE_SECTION_INDEX, sz);
                Table_section = TableSection::GetTableSection(sectionIO);
                table_list = Table_section.data;
                break;
            case MEMORY_SECTION_INDEX:
                xdbg("distribute MEMORY section(%d): size: %d", MEMORY_SECTION_INDEX, sz);
                Memory_section = MemorySection::GetMemorySection(sectionIO);
                memory_list = Memory_section.data;
                break;
            case GLOBAL_SECTION_INDEX:
                xdbg("distribute GLOBAL section(%d): size: %d", GLOBAL_SECTION_INDEX, sz);
                Global_section = GlobalSection::GetGlobalSection(sectionIO);
                global_list = Global_section.data;
                break;
            case EXPORT_SECTION_INDEX:
                xdbg("distribute EXPORT section(%d): size: %d", EXPORT_SECTION_INDEX, sz);
                Export_section = ExportSection::GetExportSection(sectionIO);
                export_list = Export_section.data;
                break;
            case START_SECTION_INDEX:
                if (start.exit) {
                    xerror("cppwasm: junk after last section");
                }
                xdbg("distribute START section(%d): size: %d", START_SECTION_INDEX, sz);
                Start_section = StartSection::GetStartSection(sectionIO);
                start = Start_section.start;
                // xerror("cppwasm:should not have start section");
                break;
            case ELEMENT_SECTION_INDEX:
                xdbg("distribute ELEMENT section(%d): size: %d", ELEMENT_SECTION_INDEX, sz);
                Element_section = ElementSection::GetElementSection(sectionIO);
                element_list = Element_section.data;
                break;
            case CODE_SECTION_INDEX:
                xdbg("distribute CODE section(%d): size: %d", CODE_SECTION_INDEX, sz);
                Code_section = CodeSection::GetCodeSection(sectionIO);
                ASSERT(Function_section.data.size() == Code_section.data.size(), "cppwasm: function and code section have inconsistent lengths");
                for (auto index = 0; index < Code_section.data.size(); ++index) {
                    Function func{};
                    func.type_index = Function_section.data[index];
                    func.local_list = {};
                    auto & ll = Code_section.data[index].func.local_list;
                    for (auto _local : ll) {
                        for (auto i = 0; i < _local.n; ++i) {
                            func.local_list.push_back(_local.type);
                        }
                    }
                    func.expr = Code_section.data[index].func.expr;
                    function_list.push_back(func);
                }
                break;
            case DATA_SECTION_INDEX:
                xdbg("distribute DATA section(%d): size: %d", DATA_SECTION_INDEX, sz);
                Data_section = DataSection::GetDataSection(sectionIO);
                data_list = Data_section.data;
                break;
            default:
                xerror("cppwasm: out of section index !");
                break;
            }
        }
        xdbg("------------- Module end init --------------\n");
        ASSERT(Function_section.data.size() == Code_section.data.size(), "cppwasm: function and code section have inconsistent lengths");
    };

    CustomSection Custom_section{};
    TypeSection Type_section{};
    ImportSection Import_section{};
    FunctionSection Function_section{};
    TableSection Table_section{};
    MemorySection Memory_section{};
    GlobalSection Global_section{};
    ExportSection Export_section{};
    StartSection Start_section{};
    ElementSection Element_section{};
    CodeSection Code_section{};
    DataSection Data_section{};

    std::vector<FunctionType> type_list{};
    std::vector<Function> function_list{};
    std::vector<Table> table_list{};
    std::vector<Memory> memory_list{};
    std::vector<Global> global_list{};
    std::vector<Element> element_list{};
    std::vector<Data> data_list{};
    StartFunction start;
    std::vector<Import> import_list{};
    std::vector<Export> export_list{};
};