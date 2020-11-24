#pragma once
#include "base/Variant.h"
#include "base/wasi-define.h"
// #include "wasi-alu.h"
#include "LittleEndian.h"
#include "wasi-binary.h"

#include <cfenv>

using InputType = Variant<int32_t, int64_t, float, double, std::string>;
#define INPUT_TYPE_I32 1
#define INPUT_TYPE_I64 2
#define INPUT_TYPE_F32 3
#define INPUT_TYPE_F64 4
#define INPUT_TYPE_STR 5

class Value {
public:
    Value() {
    }
    Value(byte_vec bv) : raw_data{bv} {
    }
    Value(int32_t i32) : raw_data{LittleEndian::pack_i32(i32)}, _type{TYPE_i32} {
    }
    Value(int64_t i64) : raw_data{LittleEndian::pack_i64(i64)}, _type{TYPE_i64} {
    }
    Value(uint32_t u32) : raw_data{LittleEndian::pack_u32(u32)}, _type{TYPE_i32} {
    }
    Value(uint64_t u64) : raw_data{LittleEndian::pack_u64(u64)}, _type{TYPE_i64} {
    }
    Value(float f32) : raw_data{LittleEndian::pack_f32(f32)}, _type{TYPE_f32} {
    }
    Value(double f64) : raw_data{LittleEndian::pack_f64(f64)}, _type{TYPE_f64} {
    }
    Value(std::string str) : raw_data{S_encode(str)} {
    }

    int32_t to_i32() {
        return LittleEndian::i32({raw_data.begin(), raw_data.begin() + 4});
    }
    int64_t to_i64() {
        return LittleEndian::i64({raw_data.begin(), raw_data.begin() + 8});
    }
    uint32_t to_u32() {
        return LittleEndian::u32({raw_data.begin(), raw_data.begin() + 4});
    }
    uint64_t to_u64() {
        return LittleEndian::u64({raw_data.begin(), raw_data.begin() + 8});
    }
    float to_f32() {
        return LittleEndian::f32({raw_data.begin(), raw_data.begin() + 4});
    }
    double to_f64() {
        return LittleEndian::f64({raw_data.begin(), raw_data.begin() + 8});
    }
    std::string to_string() {
        return S_decode(raw_data);
    }
    byte_vec raw() {
        return raw_data;
    }
    byte_vec & raw_ref(){
        return raw_data;
    }

    byte type() {
        return _type;
    }

    static Value from_f32_u32(uint32_t u32) {
        Value v{u32};
        v._type = TYPE_f32;
        return v;
    }

    static Value from_f64_u64(uint64_t u64) {
        Value v{u64};
        v._type = TYPE_f64;
        return v;
    }

    static Value newTypeZero(byte type) {
        assert(type <= TYPE_i32 && type >= TYPE_f64);
        switch(type){
            case TYPE_i32:
                return Value((int32_t)0);
            case TYPE_i64:
                return Value((int64_t)0);
            case TYPE_f32:
                return Value((float)0);
            case TYPE_f64:
                return Value((double)0);
            default:
                xerror("cppwasm: unknown type!");
        }
    }

    static Value newValue(InputType data) {
        switch (data.GetType()) {
        case INPUT_TYPE_I32:
            return Value(data.GetConstRef<int32_t>());
        case INPUT_TYPE_I64:
            return Value(data.GetConstRef<int64_t>());
        case INPUT_TYPE_F32:
            return Value(data.GetConstRef<float>());
        case INPUT_TYPE_F64:
            return Value(data.GetConstRef<double>());
        case INPUT_TYPE_STR:
            return Value(data.GetConstRef<std::string>());
        default:
            xerror("cppwasm:unknow input type");
        }
    }

private:
    byte_vec raw_data;
    byte _type;
};

class Result {
public:
    Result() {
    }

    std::vector<Value> data;
};

using FunctionAddress = int64_t;
using TableAddress = int64_t;
using MemoryAddress = int64_t;
using MemoryAddress = int64_t;
using GlobalAddress = int64_t;

/**
 * @brief An external value is the runtime representation of an entity that can be imported or exported. It is an address
 * denoting either a function instance, table instance, memory instance, or global instances in the shared store.
 *
 * externval ::= func funcaddr
 *             | table tableaddr
 *             | mem memaddr
 *             | global globaladdr
 *
 * Variant<FunctionAddress, TableAddress, MemoryAddress, GlobalAddress>
 */
using ExternValue = std::pair<int64_t, int64_t>;  //{type[0-3],value}
#define FUNCTION_EXT_INDEX 0
#define TABLE_EXT_INDEX 1
#define MEMORY_EXT_INDEX 2
#define GLOBAL_EXT_INDEX 3

/**
 * @brief  An export instance is the runtime representation of an export. It defines the export's name and the associated external value.
 *
 * exportinst ::= {name name, value externval}
 */
class ExportInstance {
public:
    ExportInstance(std::string _name, ExternValue _value) : name{_name}, value{_value} {
    }

    std::string name;
    ExternValue value;
};

/**
 * @brief A module instance is the runtime representation of a module. It is created by instantiating a module, and
 * collects runtime representations of all entities that are imported, defined, or exported by the module.
 *
 * moduleinst ::= {
 *      types functype∗
 *      funcaddrs funcaddr∗
 *      tableaddrs tableaddr∗
 *      memaddrs memaddr∗
 *      globaladdrs globaladdr∗
 *      exports exportinst∗
 * }
 *
 */
class ModuleInstance {
public:
    ModuleInstance() {
    }

    std::vector<FunctionType> type_list;
    std::vector<FunctionAddress> function_addr_list;
    std::vector<TableAddress> table_addr_list;
    std::vector<MemoryAddress> memory_addr_list;
    std::vector<GlobalAddress> global_addr_list;
    std::vector<ExportInstance> export_list;
};

class WasmFunc {
public:
    WasmFunc(FunctionType _type, std::shared_ptr<ModuleInstance> _module, Function _code) : type{_type}, module{_module}, code{_code} {
    }

    FunctionType type;
    std::shared_ptr<ModuleInstance> module;
    Function code;
};

/**
 * @brief Any internal method should inherit this class and implement its function in the overloaded operator().
 */
class host_func_base {
public:
    virtual Result operator()(std::vector<Value> args, std::vector<ValueType> types) = 0;
};
using host_func_base_ptr = std::shared_ptr<host_func_base>;
/**
 * @brief A host function is a function expressed outside WebAssembly but passed to a module as an import. The definition
 * and behavior of host functions are outside the scope of this specification. For the purpose of this
 * specification, it is assumed that when invoked, a host function behaves non-deterministically, but within certain
 * constraints that ensure the integrity of the runtime.
 *
 */
class HostFunc {
public:
    HostFunc(FunctionType _type, host_func_base_ptr _callable) : type{_type}, callable{_callable} {
    }

    Result exec(std::vector<Value> function_args, std::vector<ValueType> args_type) const {
        return callable->operator()(function_args, args_type);
    }

    FunctionType type;
    host_func_base_ptr callable;
};

/**
 * @brief A function instance is the runtime representation of a function. It effectively is a closure of the original
 * function over the runtime module instance of its originating module. The module instance is used to resolve
 * references to other definitions during execution of the function.
 *
 * funcinst ::= {type functype,module moduleinst,code func}
 *            | {type functype,hostcode hostfunc}
 * hostfunc ::= ...
 *
 */
using FunctionInstance = Variant<WasmFunc, HostFunc>;
#define FUNCTION_INSTANCE_WASM 1
#define FUNCTION_INSTANCE_HOST 2

/**
 * @brief A table instance is the runtime representation of a table. It holds a vector of function elements and an optional
 * maximum size, if one was specified in the table type at the table’s definition site.
 *
 * Each function element is either empty, representing an uninitialized table entry, or a function address. Function
 * elements can be mutated through the execution of an element segment or by external means provided by the embedder.
 *
 * tableinst ::= {elem vec(funcelem), max u32?}
 * funcelem ::= funcaddr?
 *
 * It is an invariant of the semantics that the length of the element vector never exceeds the maximum size, if present.
 *
 */
class TableInstance {
public:
    TableInstance(int64_t _element_type, Limits _limits) : element_type{_element_type}, limits{_limits} {
        element_list.reserve(limits.n);
    }

    int64_t element_type;
    std::vector<FunctionAddress> element_list;
    Limits limits;
};

/**
 * @brief A memory instance is the runtime representation of a linear memory. It holds a vector of bytes and an optional
 * maximum size, if one was specified at the definition site of the memory.
 *
 * meminst ::= {data vec(byte), max u32?}
 *
 * The length of the vector always is a multiple of the WebAssembly page size, which is defined to be the constant
 * 65536 – abbreviated 64Ki. Like in a memory type, the maximum size in a memory instance is given in units of this
 * page size.
 *
 * The bytes can be mutated through memory instructions, the execution of a data segment, or by external means
 * provided by the embedder.
 *
 * It is an invariant of the semantics that the length of the byte vector, divided by page size, never exceeds the
 * maximum size, if present.
 *
 */
class MemoryInstance {
public:
    MemoryInstance(MemoryType _type) : type{_type} {
        grow(type.limits.n);  // Limits.n
    }

    MemoryType type;
    byte_vec data;
    int64_t size{0};

    void grow(int64_t n) {
        // grow n page memory
        if (type.limits.m && size + n > type.limits.m) {
            xerror("cppwasm: out of memory limit");
        }
        // todo make it static const 2^16 memory_page
        if (size + n > 65536) {
            xerror("cppwasm: out of memory limit");
        }

        size += n;
        data.resize(size * 65536);  // todo make it static const memory_page_size;
    }
};

/**
 * @brief A global instance is the runtime representation of a global variable. It holds an individual value and a flag
 * indicating whether it is mutable.
 *
 * globalinst ::= {value val, mut mut}
 *
 * The value of mutable globals can be mutated through variable instructions or by external means provided by the embedder.
 *
 */
class GlobalInstance {
public:
    GlobalInstance(Value _value, Mut _mut) : value{_value}, mut{_mut} {
    }

    Value value;
    Mut mut;
};

// GlobalInstance can be just Value. IF not need to use Mut.
using imp_variant = Variant<host_func_base_ptr, TableInstance, MemoryInstance, GlobalInstance>;
#define IMP_VAR_TYPE_FUNC 1
#define IMP_VAR_TYPE_TABLE 2
#define IMP_VAR_TYPE_MEMORY 3
#define IMP_VAR_TYPE_GLOBAL 4

/**
 * @brief The store represents all global state that can be manipulated by WebAssembly programs. It consists of the runtime
 * representation of all instances of functions, tables, memories, and globals that have been allocated during the
 * life time of the abstract machine
 * Syntactically, the store is defined as a record listing the existing instances of each category:
 * store ::= {
 *      funcs funcinst∗
 *      tables tableinst∗
 *      mems meminst∗
 *      globals globalinst∗
 * }
 *
 * Addresses are dynamic, globally unique references to runtime objects, in contrast to indices, which are static,
 * module-local references to their original definitions. A memory address memaddr denotes the abstract address of
 * a memory instance in the store, not an offset inside a memory instance.
 *
 */
class Store {
public:
    Store() {
    }

    std::vector<FunctionInstance> function_list{};
    std::vector<TableInstance> table_list{};
    std::vector<MemoryInstance> memory_list{};
    std::vector<GlobalInstance> global_list{};

    // # For compatibility with older 0.4.x versions
    std::vector<MemoryInstance> & mems = memory_list;

    FunctionAddress allocate_wasm_function(std::shared_ptr<ModuleInstance> module, Function const & function) {
        FunctionAddress function_address = function_list.size();
        FunctionType function_type = module->type_list[function.type_index];
        WasmFunc wasmfunc{function_type, module, function};
        function_list.push_back(wasmfunc);
        return function_address;
    }

    // todo check hostfunc
    FunctionAddress allocate_host_function(HostFunc hostfunc) {
        FunctionAddress function_address = function_list.size();
        function_list.push_back(hostfunc);
        return function_address;
    }

    TableAddress allocate_table(TableType table_type) {
        TableAddress table_address = table_list.size();
        // todo make it static const funcref 0x70
        TableInstance table_instance{0x70, table_type.limits};
        table_list.push_back(table_instance);
        return table_address;
    }

    MemoryAddress allocate_memory(MemoryType memory_type) {
        MemoryAddress memory_address = memory_list.size();
        MemoryInstance memory_instance{memory_type};
        memory_list.push_back(memory_instance);
        return memory_address;
    }

    GlobalAddress allocate_global(GlobalType global_type, Value value) {
        GlobalAddress global_address = global_list.size();
        GlobalInstance global_instance{value, global_type.mut};
        global_list.push_back(global_instance);
        return global_address;
    }
};

/**
 * @brief Labels carry an argument arity n and their associated branch target, which is expressed syntactically as an instruction sequence:
 *
 * label ::= labeln{instr∗}
 *
 * Intuitively, instr∗ is the continuation to execute when the branch is taken, in place of the original control construct.
 *
 */
class Label {
public:
    Label(int64_t _arity, int64_t _continuation) : arity{_arity}, continuation{_continuation} {
    }

    // private:
    int64_t arity;
    int64_t continuation;
};

/**
 * @brief Activation frames carry the return arity n of the respective function, hold the values of its locals
 * (including arguments) in the order corresponding to their static local indices, and a reference to the function's
 * own module instance.
 *
 */
class Frame {
public:
    Frame() {
    }
    Frame(std::shared_ptr<ModuleInstance> _module, std::vector<Value> _local_list, Expression _expr, int64_t _arity)
      : module{_module}, local_list{_local_list}, expr{_expr}, arity{_arity} {
    }

    std::shared_ptr<ModuleInstance> module;
    std::vector<Value> local_list;
    Expression expr;
    int64_t arity;
};

// todo may move it somewhere else?
using stack_unit = Variant<Value, Label, Frame>;
#define STACK_UNIT_VALUE_TYPE 1
#define STACK_UNIT_LABEL_TYPE 2
#define STACK_UNIT_FRAME_TYPE 3
/**
 * @brief Besides the store, most instructions interact with an implicit stack. The stack contains three kinds of entries:
 * Values: the operands of instructions.
 * Labels: active structured control instructions that can be targeted by branches.
 * Activations: the call frames of active function calls.
 *
 * These entries can occur on the stack in any order during the execution of a program. Stack entries are described
 * by abstract syntax as follows.
 *
 */
class Stack {
public:
    Stack() {
    }
    uint32_t len() {
        return data.size();
    }
    void append(stack_unit u) {
        data.emplace_back(u);
    }
    stack_unit pop() {
        if (!data.size())
            xerror("cppwasm:empty stack!");
        auto u = data.back();
        data.pop_back();
        return u;
    }
    stack_unit back() {
        return data.back();
    }
    Label get_nth_label(uint32_t i) {
        // for(auto v:data){
        //     if(v.GetType()==STACK_UNIT_LABEL_TYPE){
        //         xdbg("label 1 %d,%d", v.GetRef<Label>().arity, v.GetRef<Label>().continuation);
        //     }
        // }

        for (auto index = data.size() - 1; index >= 0; --index) {
            if (data[index].GetType() == STACK_UNIT_LABEL_TYPE) {
                if (i == 0)
                    return data[index].GetRef<Label>();
                else
                    --i;
            }
        }

        for (auto index = 0; index < data.size(); ++index) {
            if (data[index].GetType() == STACK_UNIT_LABEL_TYPE) {
                if (i == 0)
                    return data[index].GetRef<Label>();
                else
                    --i;
            }
        }
    }

private:
    std::vector<stack_unit> data;
};

// todo make it override the class operator ==
static bool match_limits(Limits a, Limits b) {
    if (a.n >= b.n) {
        if (b.m == 0)
            return true;
        if (a.m != 0 && b.m != 0) {
            return a.m < b.m;
        }
    }
    return false;
}
static bool match_function(FunctionType a, FunctionType b) {
    return a.args.data == b.args.data && a.rets.data == b.rets.data;
}
static bool match_memory(MemoryType a, MemoryType b) {
    return match_limits(a.limits, b.limits);
}

/**
 * ======================================================================================================================
 * Abstract Machine
 * ======================================================================================================================
 */

/**
 * @brief  A configuration consists of the current store and an executing thread.
 * A thread is a computation over instructions that operates relative to a current frame referring to the module
 * instance in which the computation runs, i.e., where the current function originates from.
 *
 * config ::= store;thread
 * thread ::= frame;instr∗
 *
 */
class Configuration {
public:
    Configuration(std::shared_ptr<Store> _store) : store{_store} {
    }

    std::shared_ptr<Store> store;
    Frame frame;
    Stack stack{};
    int64_t depth{0};
    int64_t pc{0};

public:
    Label get_label(uint32_t i) {
        return stack.get_nth_label(i);
    }

    void set_frame(Frame _frame) {
        frame = _frame;
        stack.append(frame);
        stack.append(Label(frame.arity, frame.expr.data.size() - 1));
    }

    Result call(FunctionAddress function_addr, std::vector<Value> & function_args);

    Result exec();
};

/**
 * ======================================================================================================================
 * Instruction Set
 * ======================================================================================================================
 */

class ArithmeticLogicUnit {
public:
    // todo make it smart_ptr
    static void exec(Configuration * config, Instruction * i) {
        switch (i->opcode) {
        case instruction::unreachable:
            xerror("cppwasm: unreachable");
        case instruction::nop:
            xdbg("instruction: nop  pass");
            break;
        case instruction::block:
            block(config, i);
            break;
        case instruction::loop:
            loop(config, i);
            break;
        case instruction::if_:
            if_(config, i);
            break;
        case instruction::else_:
            else_(config, i);
            break;
        case instruction::end:
            end(config, i);
            break;
        case instruction::br:
            br(config, i);
            break;
        case instruction::br_if:
            br_if(config, i);
            break;
        case instruction::br_table:
            br_table(config, i);
            break;
        case instruction::return_:
            return_(config, i);
            break;
        case instruction::call:
            call(config, i);
            break;
        case instruction::call_indirect:
            call_indirect(config, i);
            break;
        case instruction::drop:
            drop(config, i);
            break;
        case instruction::select:
            select(config, i);
            break;
        case instruction::get_local:
            get_local(config, i);
            break;
        case instruction::set_local:
            set_local(config, i);
            break;
        case instruction::tee_local:
            tee_local(config, i);
            break;
        case instruction::get_global:
            get_global(config, i);
            break;
        case instruction::set_global:
            set_global(config, i);
            break;
        case instruction::i32_load:
            i32_load(config, i);
            break;
        case instruction::i64_load:
            i64_load(config, i);
            break;
        case instruction::f32_load:
            f32_load(config, i);
            break;
        case instruction::f64_load:
            f64_load(config, i);
            break;
        case instruction::i32_load8_s:
            i32_load8_s(config, i);
            break;
        case instruction::i32_load8_u:
            i32_load8_u(config, i);
            break;
        case instruction::i32_load16_s:
            i32_load16_s(config, i);
            break;
        case instruction::i32_load16_u:
            i32_load16_u(config, i);
            break;
        case instruction::i64_load8_s:
            i64_load8_s(config, i);
            break;
        case instruction::i64_load8_u:
            i64_load8_u(config, i);
            break;
        case instruction::i64_load16_s:
            i64_load16_s(config, i);
            break;
        case instruction::i64_load16_u:
            i64_load16_u(config, i);
            break;
        case instruction::i64_load32_s:
            i64_load32_s(config, i);
            break;
        case instruction::i64_load32_u:
            i64_load32_u(config, i);
            break;
        case instruction::i32_store:
            i32_store(config, i);
            break;
        case instruction::i64_store:
            i64_store(config, i);
            break;
        case instruction::f32_store:
            f32_store(config, i);
            break;
        case instruction::f64_store:
            f64_store(config, i);
            break;
        case instruction::i32_store8:
            i32_store8(config, i);
            break;
        case instruction::i32_store16:
            i32_store16(config, i);
            break;
        case instruction::i64_store8:
            i64_store8(config, i);
            break;
        case instruction::i64_store16:
            i64_store16(config, i);
            break;
        case instruction::i64_store32:
            i64_store32(config, i);
            break;
        case instruction::current_memory:
            current_memory(config, i);
            break;
        case instruction::grow_memory:
            grow_memory(config, i);
            break;
        case instruction::i32_const:
            i32_const(config, i);
            break;
        case instruction::i64_const:
            i64_const(config, i);
            break;
        case instruction::f32_const:
            f32_const(config, i);
            break;
        case instruction::f64_const:
            f64_const(config, i);
            break;
        case instruction::i32_eqz:
            i32_eqz(config, i);
            break;
        case instruction::i32_eq:
            i32_eq(config, i);
            break;
        case instruction::i32_ne:
            i32_ne(config, i);
            break;
        case instruction::i32_lts:
            i32_lts(config, i);
            break;
        case instruction::i32_ltu:
            i32_ltu(config, i);
            break;
        case instruction::i32_gts:
            i32_gts(config, i);
            break;
        case instruction::i32_gtu:
            i32_gtu(config, i);
            break;
        case instruction::i32_les:
            i32_les(config, i);
            break;
        case instruction::i32_leu:
            i32_leu(config, i);
            break;
        case instruction::i32_ges:
            i32_ges(config, i);
            break;
        case instruction::i32_geu:
            i32_geu(config, i);
            break;
        case instruction::i64_eqz:
            i64_eqz(config, i);
            break;
        case instruction::i64_eq:
            i64_eq(config, i);
            break;
        case instruction::i64_ne:
            i64_ne(config, i);
            break;
        case instruction::i64_lts:
            i64_lts(config, i);
            break;
        case instruction::i64_ltu:
            i64_ltu(config, i);
            break;
        case instruction::i64_gts:
            i64_gts(config, i);
            break;
        case instruction::i64_gtu:
            i64_gtu(config, i);
            break;
        case instruction::i64_les:
            i64_les(config, i);
            break;
        case instruction::i64_leu:
            i64_leu(config, i);
            break;
        case instruction::i64_ges:
            i64_ges(config, i);
            break;
        case instruction::i64_geu:
            i64_geu(config, i);
            break;
        case instruction::f32_eq:
            f32_eq(config, i);
            break;
        case instruction::f32_ne:
            f32_ne(config, i);
            break;
        case instruction::f32_lt:
            f32_lt(config, i);
            break;
        case instruction::f32_gt:
            f32_gt(config, i);
            break;
        case instruction::f32_le:
            f32_le(config, i);
            break;
        case instruction::f32_ge:
            f32_ge(config, i);
            break;
        case instruction::f64_eq:
            f64_eq(config, i);
            break;
        case instruction::f64_ne:
            f64_ne(config, i);
            break;
        case instruction::f64_lt:
            f64_lt(config, i);
            break;
        case instruction::f64_gt:
            f64_gt(config, i);
            break;
        case instruction::f64_le:
            f64_le(config, i);
            break;
        case instruction::f64_ge:
            f64_ge(config, i);
            break;
        case instruction::i32_clz:
            i32_clz(config, i);
            break;
        case instruction::i32_ctz:
            i32_ctz(config, i);
            break;
        case instruction::i32_popcnt:
            i32_popcnt(config, i);
            break;
        case instruction::i32_add:
            i32_add(config, i);
            break;
        case instruction::i32_sub:
            i32_sub(config, i);
            break;
        case instruction::i32_mul:
            i32_mul(config, i);
            break;
        case instruction::i32_divs:
            i32_divs(config, i);
            break;
        case instruction::i32_divu:
            i32_divu(config, i);
            break;
        case instruction::i32_rems:
            i32_rems(config, i);
            break;
        case instruction::i32_remu:
            i32_remu(config, i);
            break;
        case instruction::i32_and:
            i32_and(config, i);
            break;
        case instruction::i32_or:
            i32_or(config, i);
            break;
        case instruction::i32_xor:
            i32_xor(config, i);
            break;
        case instruction::i32_shl:
            i32_shl(config, i);
            break;
        case instruction::i32_shrs:
            i32_shrs(config, i);
            break;
        case instruction::i32_shru:
            i32_shru(config, i);
            break;
        case instruction::i32_rotl:
            i32_rotl(config, i);
            break;
        case instruction::i32_rotr:
            i32_rotr(config, i);
            break;
        case instruction::i64_clz:
            i64_clz(config, i);
            break;
        case instruction::i64_ctz:
            i64_ctz(config, i);
            break;
        case instruction::i64_popcnt:
            i64_popcnt(config, i);
            break;
        case instruction::i64_add:
            i64_add(config, i);
            break;
        case instruction::i64_sub:
            i64_sub(config, i);
            break;
        case instruction::i64_mul:
            i64_mul(config, i);
            break;
        case instruction::i64_divs:
            i64_divs(config, i);
            break;
        case instruction::i64_divu:
            i64_divu(config, i);
            break;
        case instruction::i64_rems:
            i64_rems(config, i);
            break;
        case instruction::i64_remu:
            i64_remu(config, i);
            break;
        case instruction::i64_and:
            i64_and(config, i);
            break;
        case instruction::i64_or:
            i64_or(config, i);
            break;
        case instruction::i64_xor:
            i64_xor(config, i);
            break;
        case instruction::i64_shl:
            i64_shl(config, i);
            break;
        case instruction::i64_shrs:
            i64_shrs(config, i);
            break;
        case instruction::i64_shru:
            i64_shru(config, i);
            break;
        case instruction::i64_rotl:
            i64_rotl(config, i);
            break;
        case instruction::i64_rotr:
            i64_rotr(config, i);
            break;
        case instruction::f32_abs:
            f32_abs(config, i);
            break;
        case instruction::f32_neg:
            f32_neg(config, i);
            break;
        case instruction::f32_ceil:
            f32_ceil(config, i);
            break;
        case instruction::f32_floor:
            f32_floor(config, i);
            break;
        case instruction::f32_trunc:
            f32_trunc(config, i);
            break;
        case instruction::f32_nearest:
            f32_nearest(config, i);
            break;
        case instruction::f32_sqrt:
            f32_sqrt(config, i);
            break;
        case instruction::f32_add:
            f32_add(config, i);
            break;
        case instruction::f32_sub:
            f32_sub(config, i);
            break;
        case instruction::f32_mul:
            f32_mul(config, i);
            break;
        case instruction::f32_div:
            f32_div(config, i);
            break;
        case instruction::f32_min:
            f32_min(config, i);
            break;
        case instruction::f32_max:
            f32_max(config, i);
            break;
        case instruction::f32_copysign:
            f32_copysign(config, i);
            break;
        case instruction::f64_abs:
            f64_abs(config, i);
            break;
        case instruction::f64_neg:
            f64_neg(config, i);
            break;
        case instruction::f64_ceil:
            f64_ceil(config, i);
            break;
        case instruction::f64_floor:
            f64_floor(config, i);
            break;
        case instruction::f64_trunc:
            f64_trunc(config, i);
            break;
        case instruction::f64_nearest:
            f64_nearest(config, i);
            break;
        case instruction::f64_sqrt:
            f64_sqrt(config, i);
            break;
        case instruction::f64_add:
            f64_add(config, i);
            break;
        case instruction::f64_sub:
            f64_sub(config, i);
            break;
        case instruction::f64_mul:
            f64_mul(config, i);
            break;
        case instruction::f64_div:
            f64_div(config, i);
            break;
        case instruction::f64_min:
            f64_min(config, i);
            break;
        case instruction::f64_max:
            f64_max(config, i);
            break;
        case instruction::f64_copysign:
            f64_copysign(config, i);
            break;
        case instruction::i32_wrap_i64:
            i32_wrap_i64(config, i);
            break;
        case instruction::i32_trunc_sf32:
            i32_trunc_sf32(config, i);
            break;
        case instruction::i32_trunc_uf32:
            i32_trunc_uf32(config, i);
            break;
        case instruction::i32_trunc_sf64:
            i32_trunc_sf64(config, i);
            break;
        case instruction::i32_trunc_uf64:
            i32_trunc_uf64(config, i);
            break;
        case instruction::i64_extend_si32:
            i64_extend_si32(config, i);
            break;
        case instruction::i64_extend_ui32:
            i64_extend_ui32(config, i);
            break;
        case instruction::i64_trunc_sf32:
            i64_trunc_sf32(config, i);
            break;
        case instruction::i64_trunc_uf32:
            i64_trunc_uf32(config, i);
            break;
        case instruction::i64_trunc_sf64:
            i64_trunc_sf64(config, i);
            break;
        case instruction::i64_trunc_uf64:
            i64_trunc_uf64(config, i);
            break;
        case instruction::f32_convert_si32:
            f32_convert_si32(config, i);
            break;
        case instruction::f32_convert_ui32:
            f32_convert_ui32(config, i);
            break;
        case instruction::f32_convert_si64:
            f32_convert_si64(config, i);
            break;
        case instruction::f32_convert_ui64:
            f32_convert_ui64(config, i);
            break;
        case instruction::f32_demote_f64:
            f32_demote_f64(config, i);
            break;
        case instruction::f64_convert_si32:
            f64_convert_si32(config, i);
            break;
        case instruction::f64_convert_ui32:
            f64_convert_ui32(config, i);
            break;
        case instruction::f64_convert_si64:
            f64_convert_si64(config, i);
            break;
        case instruction::f64_convert_ui64:
            f64_convert_ui64(config, i);
            break;
        case instruction::f64_promote_f32:
            f64_promote_f32(config, i);
            break;
        case instruction::i32_reinterpret_f32:
            i32_reinterpret_f32(config, i);
            break;
        case instruction::i64_reinterpret_f64:
            i64_reinterpret_f64(config, i);
            break;
        case instruction::f32_reinterpret_i32:
            f32_reinterpret_i32(config, i);
            break;
        case instruction::f64_reinterpret_i64:
            f64_reinterpret_i64(config, i);
            break;
        default:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        }
        return;
    }

    static void block(Configuration * config, Instruction * i) {
        xdbg("instruction: block");
        auto ptr = dynamic_cast<args_block *>(i->args.get());
        int32_t arity{1};
        // todo make 0x40:convention.empty
        if (ptr->data == 0x40) {
            arity = 0;
        }
        auto continuation = config->frame.expr.position[config->pc][1];
        config->stack.append(Label{arity, continuation});
    }

    static void loop(Configuration * config, Instruction * i) {
        xdbg("instruction: loop");
        auto ptr = dynamic_cast<args_block *>(i->args.get());
        int32_t arity{1};
        // todo make 0x40:convention.empty
        if (ptr->data == 0x40) {
            arity = 0;
        }
        auto continuation = config->frame.expr.position[config->pc][0];
        config->stack.append(Label{arity, continuation});
    }

    static void if_(Configuration * config, Instruction * i) {
        xdbg("instruction: if_");
        auto c = config->stack.pop().GetRef<Value>().to_i32();
        auto ptr = dynamic_cast<args_block *>(i->args.get());
        int32_t arity{1};
        // todo make 0x40:convention.empty
        if (ptr->data == 0x40) {
            arity = 0;
        }
        auto continuation = config->frame.expr.position[config->pc][1];
        config->stack.append(Label{arity, continuation});
        if (c == 0) {
            if (config->frame.expr.position[config->pc].size() == 3) {
                config->pc = config->frame.expr.position[config->pc][2];
            } else {
                config->pc = config->frame.expr.position[config->pc][1];
                config->stack.pop();
            }
        }
    }

    static void else_(Configuration * config, Instruction * i) {
        xdbg("instruction: else_");
        auto L = config->get_label(0).arity;
        std::vector<stack_unit> tmp{};
        xdbg(" - L:%d", L);
        while (L--) {
            tmp.push_back(config->stack.pop());
        }
        while (true) {
            auto s = config->stack.pop();
            if (s.GetType() == STACK_UNIT_LABEL_TYPE)
                break;
            xdbg(" - end: pop ");
        }
        for (auto & v : tmp) {
            config->stack.append(v);
        }
        config->pc = config->frame.expr.position[config->pc][1];
    }

    static void end(Configuration * config, Instruction * i) {
        xdbg("instruction: end");
        auto L = config->get_label(0).arity;
        std::vector<stack_unit> tmp{};
        xdbg(" - L:%d", L);
        while (L--) {
            tmp.push_back(config->stack.pop());
            // auto s = config->stack.pop();
            // if (s.GetType() == STACK_UNIT_VALUE_TYPE) {
            // res_value_vec.push_back(s.GetRef<Value>());
            // }
        }
        while (true) {
            auto s = config->stack.pop();
            if (s.GetType() == STACK_UNIT_LABEL_TYPE)
                break;
            xdbg(" - end: pop ");
        }
        for (auto & v : tmp) {
            config->stack.append(v);
        }
    }

    static void br_label(Configuration * config, uint32_t i) {
        Label L = config->get_label(i);
        xdbg(" -Lable %d: arity: %d, continuation:%d", i, L.arity, L.continuation);

        std::vector<stack_unit> _v;
        for (auto index = 0; index < L.arity; ++index) {
            _v.push_back(config->stack.pop());
        }
        int32_t s = 0, n = i;
        if (L.continuation >= config->pc)
            n++;
        while (s != n) {
            auto e = config->stack.pop();
            if (e.GetType() == STACK_UNIT_LABEL_TYPE)
                s++;
        }
        for (auto & e : _v) {
            config->stack.append(e);
        }
        config->pc = L.continuation;
    }

    static void br(Configuration * config, Instruction * i) {
        xdbg("instruction: br");
        auto ptr = dynamic_cast<args_br *>(i->args.get());
        return br_label(config, ptr->data);
    }

    static void br_if(Configuration * config, Instruction * i) {
        xdbg("instruction: br_if");
        if (config->stack.pop().GetRef<Value>().to_i32()) {
            auto ptr = dynamic_cast<args_br *>(i->args.get());
            return br_label(config, ptr->data);
        }
    }

    static void br_table(Configuration * config, Instruction * i) {
        xdbg("instruction: br_table");
        auto ptr = dynamic_cast<args_br_table *>(i->args.get());
        auto a = ptr->data.first;
        auto l = ptr->data.second;
        auto c = config->stack.pop().GetRef<Value>().to_i32();
        if (c >= 0 && c < a.size()) {
            l = a[c];
        }
        return br_label(config, l);
    }

    static void return_(Configuration * config, Instruction * i) {
        xdbg("instruction: return_");
        std::vector<stack_unit> tmp;
        for (auto index = 0; index < config->frame.arity; index++) {
            tmp.push_back(config->stack.pop());
        }
        while (true) {
            auto e = config->stack.pop();
            if (e.GetType() == STACK_UNIT_FRAME_TYPE) {
                config->stack.append(e);
                break;
            }
        }
        for (auto & e : tmp) {
            config->stack.append(e);
        }
        config->pc = config->frame.expr.data.size() - 1;
    }

    static void call_function_addr(Configuration * config, FunctionAddress function_addr) {
        // todo make it const var
        if (config->depth > 1024) {
            xerror("cppwasm: call stack exhausted");
        }

        FunctionInstance function = config->store->function_list[function_addr];
        // auto func_type = function.GetRef<>
        FunctionType func_type{};
        if (function.GetType() == FUNCTION_INSTANCE_HOST) {
            func_type = function.GetRef<HostFunc>().type;
        } else {
            ASSERT(function.GetType() == FUNCTION_INSTANCE_WASM, "error function varient");
            func_type = function.GetRef<WasmFunc>().type;
        }
        std::vector<Value> function_args{};
        for (auto index = 0; index < func_type.args.data.size(); ++index) {
            function_args.push_back(config->stack.pop());
        }
        // todo make it insert.
        std::reverse(function_args.begin(), function_args.end());

        // xdbg("%d,%d", func_type.args.data.size(), function_args[0].to_i32());
        Configuration subconf(config->store);
        subconf.depth = config->depth + 1;
        Result r = subconf.call(function_addr, function_args);
        for (auto _v : r.data) {
            config->stack.append(_v);
        }
    }

    static void call(Configuration * config, Instruction * i) {
        xdbg("instruction: call");
        auto ptr = dynamic_cast<args_call *>(i->args.get());
        call_function_addr(config, ptr->data);
    }

    static void call_indirect(Configuration * config, Instruction * i) {
        xdbg("instruction: call_indirect");
        auto ptr = dynamic_cast<args_call_indirect *>(i->args.get());
        if (ptr->data.second != 0x00) {
            xerror("cppwasm: zero byte malformed in call_indirect")
        }
        auto const & ta = config->frame.module->table_addr_list[0];
        auto const & tab = config->store->table_list[ta];
        auto idx = config->stack.pop().GetRef<Value>().to_i32();
        if (idx < 0 || idx >= tab.element_list.size()) {
            xerror("cppwasm: undefined element");
        }
        auto const & func_addr = tab.element_list[idx];
        call_function_addr(config, func_addr);
    }

    static void drop(Configuration * config, Instruction * i) {
        config->stack.pop();
    }

    static void select(Configuration * config, Instruction * i) {
        auto c = config->stack.pop().GetRef<Value>().to_i32();
        auto b = config->stack.pop();
        auto a = config->stack.pop();
        if (c) {
            config->stack.append(a);
        } else {
            config->stack.append(b);
        }
    }

    static void get_local(Configuration * config, Instruction * i) {
        auto ptr = dynamic_cast<args_get_local *>(i->args.get());
        config->stack.append(config->frame.local_list[ptr->data]);
        xdbg("instruction: get_local index: %u %d", ptr->data, config->stack.back().GetRef<Value>().to_i32());
    }

    static void set_local(Configuration * config, Instruction * i) {
        xdbg("instruction: set_local");
        auto ptr = dynamic_cast<args_get_local *>(i->args.get());
        auto v = config->stack.pop();
        ASSERT(v.GetType() == STACK_UNIT_VALUE_TYPE, "non value type to be set local");
        auto & locallist = config->frame.local_list;
        if (locallist.size() <= ptr->data) {
            auto new_size = std::max(locallist.size(), (size_t)1);
            while (new_size <= ptr->data)
                new_size <<= 1;
            locallist.resize(new_size);
        }
        locallist[ptr->data] = v.GetRef<Value>();
        // config->frame.local_list.insert(config->frame.local_list.begin() + ptr->data, v.GetRef<Value>());
    }

    static void tee_local(Configuration * config, Instruction * i) {
        xdbg("instruction: tee_local");
        auto ptr = dynamic_cast<args_get_local *>(i->args.get());
        auto v = config->stack.back();
        ASSERT(v.GetType() == STACK_UNIT_VALUE_TYPE, "non value type to be set local");
        auto & locallist = config->frame.local_list;
        if (locallist.size() <= ptr->data) {
            auto new_size = ptr->data + 1;
            locallist.resize(new_size);
        }
        locallist[ptr->data] = v.GetRef<Value>();
        // config->frame.local_list.insert(config->frame.local_list.begin() + ptr->data, v.GetRef<Value>());
    }

    static void get_global(Configuration * config, Instruction * i) {
        xdbg("instruction: get_global");
        auto ptr = dynamic_cast<args_get_global *>(i->args.get());
        auto a = config->frame.module->global_addr_list[ptr->data];
        auto const & glob = config->store->global_list[a];
        auto & r = glob.value;
        config->stack.append(r);
    }

    static void set_global(Configuration * config, Instruction * i) {
        xdbg("instruction: set_global");
        auto ptr = dynamic_cast<args_get_global *>(i->args.get());
        auto a = config->frame.module->global_addr_list[ptr->data];
        auto & glob = config->store->global_list[a];
        // todo make is static const var = 0x01;
        ASSERT(glob.mut == 0x01, "should be mutable");
        glob.value = config->stack.pop().GetConstRef<Value>();
    }

    static int32_t byte2i32(byte_vec input) {
        int32_t res = 0;
        for (int index = input.size() - 1; index >= 0; index--) {
            res <<= 8;
            res |= input[index];
        }
        return res;
    }

    static int32_t ext32s(byte_vec input) {
        ASSERT(input.size() <= 4, "no need ext");
        auto res = input;
        // for (auto b : input)
        //     xdbg(" input: 0x%02x ", b);
        bool sign = input.back() & 0x80;  // true - 1 false - 0
        if (sign) {
            while (res.size() < 4) {
                res.push_back(0xff);
            }
        } else {
            while (res.size() < 4) {
                res.push_back(0x00);
            }
        }
        return byte2i32(res);
    }
    static int32_t ext32u(byte_vec input) {
        ASSERT(input.size() <= 4, "no need ext");
        auto res = input;
        while (res.size() < 4) {
            res.push_back(0x00);
        }
        return byte2i32(res);
    }

    static int64_t byte2i64(byte_vec input) {
        int64_t res = 0;
        for (int index = input.size() - 1; index >= 0; index--) {
            res <<= 8;
            res |= input[index];
        }
        return res;
    }
    static int64_t ext64s(byte_vec input) {
        ASSERT(input.size() <= 8, "no need ext");
        auto res = input;
        bool sign = input.back() & 0x80;  // true - 1 false - 0
        if (sign) {
            while (res.size() < 8) {
                res.push_back(0xff);
            }
        } else {
            while (res.size() < 8) {
                res.push_back(0x00);
            }
        }
        return byte2i64(res);
    }
    static int64_t ext64u(byte_vec input) {
        ASSERT(input.size() <= 8, "no need ext");
        auto res = input;
        while (res.size() < 8) {
            res.push_back(0x00);
        }
        return byte2i64(res);
    }

    static byte_vec mem_load(Configuration * config, Instruction * i, std::size_t size) {
        byte_vec res{};

        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];
        uint64_t offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        int64_t addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: mem_load  addr:%" PRId64 " offset:%" PRIu64 " memory data size : %zu", addr, offset, memory.data.size());
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        auto & mem = memory.data;

        for (auto index = 0; index < size; ++index) {
            res.push_back(mem[index + addr]);
        }

        // debug:
        std::printf("------mem load: ");
        for (auto index = 0; index < res.size(); ++index) {
            std::printf("0x%02x ", res[index]);
        }
        std::printf("\n");

        return res;
    }

    static void i32_load(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext32s(mem_load(config, i, 4))));
        xdbg("instruction: i32_load %d", config->stack.back().GetRef<Value>().to_i32());
    }

    static void i64_load(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64s(mem_load(config, i, 8))));
        xdbg("instruction: i64_load %d", config->stack.back().GetRef<Value>().to_i64());
    }

    static void f32_load(Configuration * config, Instruction * i) {
        config->stack.append(Value(F32_decode(mem_load(config, i, 4))));
        xdbg("instruction: f32_load %f", config->stack.back().GetRef<Value>().to_f32());
    }

    static void f64_load(Configuration * config, Instruction * i) {
        config->stack.append(Value(F64_decode(mem_load(config, i, 8))));
        xdbg("instruction: f64_load %f", config->stack.back().GetRef<Value>().to_f64());
    }

    static void i32_load8_s(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext32s(mem_load(config, i, 1))));
        xdbg("instruction: i32_load8_s %d", config->stack.back().GetRef<Value>().to_i32());
    }
    static void i32_load8_u(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext32u(mem_load(config, i, 1))));
        xdbg("instruction: i32_load8_u %d", config->stack.back().GetRef<Value>().to_i32());
    }
    static void i32_load16_s(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext32s(mem_load(config, i, 2))));
        xdbg("instruction: i32_load16_s %d", config->stack.back().GetRef<Value>().to_i32());
    }
    static void i32_load16_u(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext32u(mem_load(config, i, 2))));
        xdbg("instruction: i32_load16_u %d", config->stack.back().GetRef<Value>().to_i32());
    }
    static void i64_load8_s(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64s(mem_load(config, i, 1))));
        xdbg("instruction: i64_load8_s %lld", config->stack.back().GetRef<Value>().to_i64());
    }
    static void i64_load8_u(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64u(mem_load(config, i, 1))));
        xdbg("instruction: i64_load8_u %lld", config->stack.back().GetRef<Value>().to_u64());
    }
    static void i64_load16_s(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64s(mem_load(config, i, 2))));
        xdbg("instruction: i64_load16_s %lld", config->stack.back().GetRef<Value>().to_i64());
    }
    static void i64_load16_u(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64u(mem_load(config, i, 2))));
        xdbg("instruction: i64_load16_u %lld", config->stack.back().GetRef<Value>().to_i64());
    }
    static void i64_load32_s(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64s(mem_load(config, i, 4))));
        xdbg("instruction: i64_load32_s %lld", config->stack.back().GetRef<Value>().to_i64());
    }
    static void i64_load32_u(Configuration * config, Instruction * i) {
        config->stack.append(Value(ext64u(mem_load(config, i, 4))));
        xdbg("instruction: i64_load32_u %lld", config->stack.back().GetRef<Value>().to_i64());
    }

    static void mem_store(Configuration * config, Instruction * i, std::size_t size) {
        auto r = config->stack.pop();

        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];
        uint64_t offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        int64_t addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: mem_store  addr:%" PRId64 " offset:%" PRIu64 " memory data size : %zu", addr, offset, memory.data.size());
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        auto & mem = memory.data;
        auto data = r.GetRef<Value>().raw();

        for (auto index = 0; index < std::min(size, data.size()); ++index) {
            mem[index + addr] = data[index];
        }

        // debug
        printf("------mem store:  ");
        for (auto index = 0; index < size; ++index) {
            printf("0x%02x ", mem[index + addr]);
        }
        printf("\n");
    }

    static void i32_store(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_store");
        mem_store(config, i, 4);
    }
    static void i64_store(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_store");
        mem_store(config, i, 8);
    }
    static void f32_store(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_store");
        mem_store(config, i, 4);
    }
    static void f64_store(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_store");
        mem_store(config, i, 8);
    }
    static void i32_store8(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_store8");
        mem_store(config, i, 1);
    }
    static void i32_store16(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_store16");
        mem_store(config, i, 2);
    }
    static void i64_store8(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_store8");
        mem_store(config, i, 4);
    }
    static void i64_store16(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_store16");
        mem_store(config, i, 2);
    }
    static void i64_store32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_store32");
        mem_store(config, i, 4);
    }

    static void current_memory(Configuration * config, Instruction * i) {
        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto const & memory = config->store->memory_list[memory_addr];
        int32_t size = memory.size;
        config->stack.append(Value{size});
        xdbg("instruction: current_memory %d", size);
    }

    static void grow_memory(Configuration * config, Instruction * i) {
        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];
        auto r = config->stack.pop().GetRef<Value>().to_i32();
        int32_t size = memory.size;
        xdbg("instruction: grow_memory %d->%d", size, size + r);
        try {
            memory.grow(r);
            config->stack.append(Value{size});

        } catch (...) {
            config->stack.append(Value{(int32_t)-1});
        }
    }

    static void i32_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_i32_count *>(i->args.get())->data));
        xdbg("instruction: i32_const %d", config->stack.back().GetRef<Value>().to_i32())
    }

    static void i64_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_i64_count *>(i->args.get())->data));
        xdbg("instruction: i64_const %d", config->stack.back().GetRef<Value>().to_i64())
    }

    static void f32_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_f32_count *>(i->args.get())->data));
        xdbg("instruction: f32_const %f", config->stack.back().GetRef<Value>().to_f32())
    }

    static void f64_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_f64_count *>(i->args.get())->data));
        xdbg("instruction: f64_const %lf", config->stack.back().GetRef<Value>().to_f64())
    }

    // i32:
    static void i32_eqz(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_eqz");
        int32_t res = (config->stack.pop().GetRef<Value>().to_i32() == 0) ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_eq(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_eq");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a == b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_ne(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_ne");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a != b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_lts(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_lts");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_ltu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_ltu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_gts(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_gts");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_gtu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_gtu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_les(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_les");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_leu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_leu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i32_ges(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_ges");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }
    static void i32_geu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_geu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }

    // i64:
    static void i64_eqz(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_eqz");
        int32_t res = (config->stack.pop().GetRef<Value>().to_i64() == 0) ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_eq(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_eq");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a == b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_ne(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_ne");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a != b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_lts(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_lts");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_ltu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_ltu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_gts(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_gts");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_gtu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_gtu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_les(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_les");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_leu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_leu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void i64_ges(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_ges");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }
    static void i64_geu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_geu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }

    // f32&f64:
    static void f32_eq(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_eq");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a == b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f32_ne(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_ne");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a != b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f32_lt(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_lt");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f32_gt(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_gt");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f32_le(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_le");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f32_ge(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_ge");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }
    static void f64_eq(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_eq");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a == b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f64_ne(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_ne");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a != b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f64_lt(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_lt");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a < b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f64_gt(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_gt");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a > b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f64_le(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_le");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a <= b ? 1 : 0;
        config->stack.append(Value{res});
    }
    static void f64_ge(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_ge");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        int32_t res = a >= b ? 1 : 0;
        config->stack.append(Value(res));
    }

    static void i32_clz(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_clz");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = 0;
        while (res < 32 && ((a & 0x80000000) == 0)) {
            res++;
            a <<= 1;
        }
        config->stack.append(Value(res));
    }
    static void i32_ctz(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_ctz");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = 0;
        while (res < 32 && ((a & 0x01) == 0)) {
            res++;
            a >>= 1;
        }
        config->stack.append(Value(res));
    }
    static void i32_popcnt(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_popcnt");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t res = 0;
        for (auto index = 0; index < 32; ++index) {
            if ((a & 0x01)) {
                res++;
            }
            a >>= 1;
        }
        config->stack.append(Value(res));
    }

    static void i32_add(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        xdbg("instruction: i32_add [%d + %d = %d]", a, b, a + b);
        auto c = Value(a + b);
        config->stack.append(c);
    }

    static void i32_sub(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        xdbg("instruction: i32_sub [%d - %d = %d]", a, b, a - b);
        auto c = Value(a - b);
        config->stack.append(c);
    }

    static void i32_mul(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_mul");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a * b);
        config->stack.append(c);
    }

    static void i32_divs(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_divs");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        if (b == -1 && a == -(1 << 31))
            xerror("cppwasm: integer overflow");
        auto c = Value(a / b);
        config->stack.append(c);
    }

    static void i32_divu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_divu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a / b);
        config->stack.append(c);
    }
    static void i32_rems(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_rems");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a * b > 0 ? a % b : -(-a % b));
        config->stack.append(c);
    }
    static void i32_remu(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_remu");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a % b);
        config->stack.append(c);
    }
    static void i32_and(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_and");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a & b);
        config->stack.append(c);
    }
    static void i32_or(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_or");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a | b);
        config->stack.append(c);
    }
    static void i32_xor(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_xor");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a ^ b);
        config->stack.append(c);
    }
    static void i32_shl(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_shl");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a << (b % 0x20));
        config->stack.append(c);
    }
    static void i32_shrs(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_shrs");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(a >> (b % 0x20));
        config->stack.append(c);
    }
    static void i32_shru(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_shru");
        auto b = config->stack.pop().GetRef<Value>().to_u32();
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        auto c = Value(a >> (b % 0x20));
        config->stack.append(c);
    }
    static void i32_rotl(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_rotl");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(((a << (b % 0x20)) & 0xffffffff) | (a >> (0x20 - (b % 0x20))));
        config->stack.append(c);
    }
    static void i32_rotr(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_rotr");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto c = Value(((a >> (b % 0x20)) | ((a << (0x20 - (b % 0x20))) & 0xffffffff)));
        config->stack.append(c);
    }
    static void i64_clz(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_clz");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int64_t res = 0;
        while (res < 64 && ((a & 0x8000000000000000) == 0)) {
            res++;
            a <<= 1;
        }
        config->stack.append(Value(res));
    }
    static void i64_ctz(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_ctz");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int64_t res = 0;
        while (res < 64 && ((a & 0x01) == 0)) {
            res++;
            a >>= 1;
        }
        config->stack.append(Value(res));
    }
    static void i64_popcnt(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_popcnt");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        int64_t res = 0;
        for (auto index = 0; index < 64; ++index) {
            if ((a & 0x01)) {
                res++;
            }
            a >>= 1;
        }
        config->stack.append(Value(res));
    }
    static void i64_add(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        xdbg("instruction: i64_add [%d + %d = %d]", a, b, a + b);
        auto c = Value(a + b);
        config->stack.append(c);
    }
    static void i64_sub(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        xdbg("instruction: i64_sub [%d - %d = %d]", a, b, a - b);
        auto c = Value(a - b);
        config->stack.append(c);
    }
    static void i64_mul(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_mul");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a * b);
        config->stack.append(c);
    }
    static void i64_divs(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_divs");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        if (b == -1 && a == (int64_t)1 << 63)
            xerror("cppwasm: integer overflow");
        auto c = Value(a / b);
        config->stack.append(c);
    }
    static void i64_divu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_divu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a / b);
        config->stack.append(c);
    }
    static void i64_rems(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_rems");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a * b > 0 ? a % b : -(-a % b));
        config->stack.append(c);
    }
    static void i64_remu(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_remu");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        if (b == 0)
            xerror("cppwasm: integer divide by zero");
        auto c = Value(a % b);
        config->stack.append(c);
    }
    static void i64_and(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_and");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a & b);
        config->stack.append(c);
    }

    static void i64_or(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_or");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a | b);
        config->stack.append(c);
    }

    static void i64_xor(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_xor");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a ^ b);
        config->stack.append(c);
    }

    static void i64_shl(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_shl");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a << (b % 0x40));
        config->stack.append(c);
    }

    static void i64_shrs(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_shrs");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a >> (b % 0x40));
        config->stack.append(c);
    }
    static void i64_shru(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_shru");
        auto b = config->stack.pop().GetRef<Value>().to_u64();
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        auto c = Value(a >> (b % 0x40));
        config->stack.append(c);
    }
    static void i64_rotl(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_rotl");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(((a << (b % 0x20)) & 0xffffffff) | (a >> (0x20 - (b % 0x20))));
        config->stack.append(c);
    }
    static void i64_rotr(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_rotr");
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(((a >> (b % 0x20)) | ((a << (0x20 - (b % 0x20))) & 0xffffffff)));
        config->stack.append(c);
    }
    static void f32_abs(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_abs");
        auto a = config->stack.pop().GetRef<Value>();
        a.raw_ref()[3] = a.raw()[3] & 0x7f;
        config->stack.append(a);
    }
    static void f32_neg(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_neg");
        auto a = config->stack.pop().GetRef<Value>();
        if (a.raw()[3] & 0x80) {
            // neg:
            a.raw_ref()[3] = a.raw()[3] & 0x7f;
        } else {
            // pos:
            a.raw_ref()[3] = a.raw()[3] | 0x80;
        }
        config->stack.append(a);
    }
    static void f32_ceil(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_ceil");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::ceil(a)));
    }
    static void f32_floor(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_floor");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::floor(a)));
    }
    static void f32_trunc(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_trunc");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::trunc(a)));
    }
    static void f32_nearest(Configuration * config, Instruction * i) {
        // todo test if is this work
        // https://cloud.tencent.com/developer/section/1009475
        std::fesetround(FE_TONEAREST);
        xdbg("instruction: f32_nearest");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::nearbyintf(a)));
    }
    static void f32_sqrt(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_sqrt");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::sqrt(a)));
    }
    static void f32_add(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        xdbg("instruction: f32_add [%d + %d = %d]", a, b, a + b);
        auto c = Value(a + b);
        config->stack.append(c);
    }
    static void f32_sub(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        xdbg("instruction: f32_sub [%d - %d = %d]", a, b, a - b);
        auto c = Value(a - b);
        config->stack.append(c);
    }
    static void f32_mul(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_mul");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto c = Value(a * b);
        config->stack.append(c);
    }
    static void f32_div(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_div");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto c = Value(a / b);
        config->stack.append(c);
    }
    static void f32_min(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        xdbg("instruction: f32_min %f %f %f", a, b, std::min(a, b));
        // std::signbit(a) // return false if a is positive. true if a is negative
        if (a == 0.0 && b == 0.0 && (std::signbit(a) || std::signbit(b))) {
            // wrong if a==b==0???? a(0.0)!=b(-0.0) but a==0.0 && b==0.0???
            xdbg("instruction: f32_min negativezero");
            config->stack.append(Value::from_f32_u32(f32_negative_zero));
        } else if (isnanf(a) || isnanf(b)) {
            // todo might need figure out nan. one is nan than min/max result is nan?? strange logic CHECK DOCUMENT!
            config->stack.append(Value(isnanf(a) ? a : b));
        } else {
            xdbg("instruction: f32_min a!=b");
            // todo magicly result is different with diffent order nan can't be compare. MIGHT need to write a totally float wheel.
            config->stack.append(Value(std::min(a, b)));
        }
    }
    static void f32_max(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        xdbg("instruction: f32_max %f %f %f", a, b, std::max(b, a));
        if (a == 0.0 && b == 0.0 && !(std::signbit(a) && std::signbit(b))) {
            xdbg("instruction: f32_max positive_zero");
            config->stack.append(Value::from_f32_u32(f32_positive_zero));
        } else if (isnanf(a) || isnanf(b)) {
            config->stack.append(Value(isnanf(a) ? a : b));
        } else {
            xdbg("instruction: f32_max a!=b");
            config->stack.append(Value(std::max(b, a)));
        }
    }
    static void f32_copysign(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_copysign");
        auto b = config->stack.pop().GetRef<Value>().to_f32();
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(std::copysignf(a, b)));
    }
    static void f64_abs(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_abs");
        auto a = config->stack.pop().GetRef<Value>();
        a.raw_ref()[7] = a.raw()[7] & 0x7f;
        config->stack.append(a);
    }
    static void f64_neg(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_neg");
        auto a = config->stack.pop().GetRef<Value>();
        if (a.raw()[7] & 0x80) {
            // neg:
            a.raw_ref()[7] = a.raw()[7] & 0x7f;
        } else {
            // pos:
            a.raw_ref()[7] = a.raw()[7] | 0x80;
        }
        config->stack.append(a);
    }
    static void f64_ceil(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_ceil");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::ceil(a)));
    }
    static void f64_floor(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_floor");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::floor(a)));
    }
    static void f64_trunc(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_trunc");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::trunc(a)));
    }
    static void f64_nearest(Configuration * config, Instruction * i) {
        // todo test if is this work
        std::fesetround(FE_TONEAREST);
        xdbg("instruction: f64_nearest");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::nearbyint(a)));
    }
    static void f64_sqrt(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_sqrt");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::sqrt(a)));
    }
    static void f64_add(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        xdbg("instruction: f64_add [%d + %d = %d]", a, b, a + b);
        auto c = Value(a + b);
        config->stack.append(c);
    }
    static void f64_sub(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        xdbg("instruction: f64_sub [%d - %d = %d]", a, b, a - b);
        auto c = Value(a - b);
        config->stack.append(c);
    }
    static void f64_mul(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_mul");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto c = Value(a * b);
        config->stack.append(c);
    }
    static void f64_div(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_div");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto c = Value(a / b);
        config->stack.append(c);
    }
    static void f64_min(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        xdbg("instruction: f64_min %f %f %f", a, b, std::min(a, b));
        // std::signbit(a) // return false if a is positive. true if a is negative
        if (a == 0.0 && b == 0.0 && (std::signbit(a) || std::signbit(b))) {
            xdbg("instruction: f64_min negativezero");
            config->stack.append(Value::from_f64_u64(f64_negative_zero));
        } else if (isnanf(a) || isnanf(b)) {
            // todo might need figure out nan. one is nan than min/max result is nan?? strange logic CHECK DOCUMENT!
            config->stack.append(Value(isnanf(a) ? a : b));
        } else {
            xdbg("instruction: f64_min a!=b");
            config->stack.append(Value(std::min(a, b)));
        }
    }
    static void f64_max(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        xdbg("instruction: f64_max %f %f %f", a, b, std::max(a, b));
        if (a == 0.0 && b == 0.0 && !(std::signbit(a) && std::signbit(b))) {
            xdbg("instruction: f64_max positive_zero");
            config->stack.append(Value::from_f64_u64(f32_positive_zero));
        } else if (isnanf(a) || isnanf(b)) {
            // todo might need figure out nan. one is nan than min/max result is nan?? strange logic CHECK DOCUMENT!
            config->stack.append(Value(isnanf(a) ? a : b));
        } else {
            xdbg("instruction: f64_max a!=b");
            config->stack.append(Value(std::max(a, b)));
        }
    }
    static void f64_copysign(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_copysign");
        auto b = config->stack.pop().GetRef<Value>().to_f64();
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(std::copysign(a, b)));
    }
    static void i32_wrap_i64(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_wrap_i64");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        // todo check static_cast ==? wrap
        config->stack.append(Value(static_cast<int32_t>(a)));
    }
    static void i32_trunc_sf32(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_trunc_sf32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto r = static_cast<int32_t>(a);
        config->stack.append(Value(r));
    }
    static void i32_trunc_uf32(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_trunc_uf32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto r = static_cast<uint32_t>(a);
        config->stack.append(Value(r));
    }
    static void i32_trunc_sf64(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_trunc_sf64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto r = static_cast<int32_t>(a);
        config->stack.append(Value(r));
    }
    static void i32_trunc_uf64(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_trunc_uf64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto r = static_cast<uint32_t>(a);
        config->stack.append(Value(r));
    }
    static void i64_extend_si32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_extend_si32");
        auto a = config->stack.pop().GetRef<Value>().raw();
        config->stack.append(Value(ext64s(a)));
    }
    static void i64_extend_ui32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_extend_ui32");
        auto a = config->stack.pop().GetRef<Value>().raw();
        config->stack.append(Value(ext64u(a)));
    }
    static void i64_trunc_sf32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_trunc_sf32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto r = static_cast<int64_t>(a);
        config->stack.append(Value(r));
    }
    static void i64_trunc_uf32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_trunc_uf32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto r = static_cast<uint64_t>(a);
        config->stack.append(Value(r));
    }
    static void i64_trunc_sf64(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_trunc_sf64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto r = static_cast<int64_t>(a);
        config->stack.append(Value(r));
    }
    static void i64_trunc_uf64(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_trunc_uf64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto r = static_cast<uint64_t>(a);
        config->stack.append(Value(r));
    }
    static void f32_convert_si32(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_convert_si32");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto r = static_cast<float>(a);
        config->stack.append(Value(r));
    }
    static void f32_convert_ui32(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_convert_ui32");
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        auto r = static_cast<float>(a);
        config->stack.append(Value(r));
    }
    static void f32_convert_si64(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_convert_si64");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto r = static_cast<float>(a);
        config->stack.append(Value(r));
    }
    static void f32_convert_ui64(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_convert_si64");
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        auto r = static_cast<float>(a);
        config->stack.append(Value(r));
    }
    static void f32_demote_f64(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_demote_f64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        auto r = static_cast<float>(a);
        config->stack.append(Value(r));
    }
    static void f64_convert_si32(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_convert_si32");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        auto r = static_cast<double>(a);
        config->stack.append(Value(r));
    }
    static void f64_convert_ui32(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_convert_ui32");
        auto a = config->stack.pop().GetRef<Value>().to_u32();
        auto r = static_cast<double>(a);
        config->stack.append(Value(r));
    }
    static void f64_convert_si64(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_convert_si64");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto r = static_cast<double>(a);
        config->stack.append(Value(r));
    }
    static void f64_convert_ui64(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_convert_ui64");
        auto a = config->stack.pop().GetRef<Value>().to_u64();
        auto r = static_cast<double>(a);
        config->stack.append(Value(r));
    }
    static void f64_promote_f32(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_promote_f32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        auto r = static_cast<double>(a);
        config->stack.append(Value(r));
    }
    static void i32_reinterpret_f32(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_reinterpret_f32");
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        config->stack.append(Value(a));
    }
    static void i64_reinterpret_f64(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_reinterpret_f64");
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        config->stack.append(Value(a));
    }
    static void f32_reinterpret_i32(Configuration * config, Instruction * i) {
        xdbg("instruction: f32_reinterpret_i32");
        auto a = config->stack.pop().GetRef<Value>().to_f32();
        config->stack.append(Value(a));
    }
    static void f64_reinterpret_i64(Configuration * config, Instruction * i) {
        xdbg("instruction: f64_reinterpret_i64");
        auto a = config->stack.pop().GetRef<Value>().to_f64();
        config->stack.append(Value(a));
    }
};

/**
 * @brief Execution behavior is defined in terms of an abstract machine that models the program state. It includes a stack,
 * which records operand values and control constructs, and an abstract store containing global state.
 *
 */
// todo complete it.
class Machine {
public:
    Machine() {
        module_instance = std::make_shared<ModuleInstance>();
        store = std::make_shared<Store>();
    }
    ~Machine() {
        if (module_instance != nullptr) {
            module_instance.reset();
        }
    }
    std::shared_ptr<ModuleInstance> module_instance;
    // ModuleInstance module_instance{};
    std::shared_ptr<Store> store;

    void instantiate(Module const & module, std::vector<ExternValue> extern_value_list) {
        module_instance->type_list = module.type_list;

        // todo more check

        // todo global_value_init
        std::vector<Value> global_values;
        ModuleInstance aux{};

        for (auto & p : extern_value_list) {
            if (p.first == GLOBAL_EXT_INDEX) {
                aux.global_addr_list.push_back(p.second);
            }
        }
        for (auto & _global : module.global_list) {
            xdbg("init global value");
            // todo check this shared ptr liveness
            Frame frame{std::make_shared<ModuleInstance>(aux), {}, _global.expr, 1};
            Configuration config{store};
            config.set_frame(frame);
            auto r = config.exec().data[0];
            global_values.push_back(r);
        }
        allocate(module, extern_value_list, global_values);

        for (auto _element : module.element_list) {
            xdbg("init element");
            Frame frame{module_instance, {}, _element.offset, 1};
            Configuration config{store};
            config.set_frame(frame);
            auto r = config.exec().data[0];
            auto offset = r.to_i64();
            auto table_addr = module_instance->table_addr_list[_element.table_index];
            auto & table_instance = store->table_list[table_addr];
            if (table_instance.element_list.size() <= offset + _element.init.size()) {
                auto new_size = offset + _element.init.size();
                table_instance.element_list.resize(new_size);
            }
            for (auto index = 0; index < _element.init.size(); ++index) {
                table_instance.element_list[offset + index] = _element.init[index];
            }
        }

        for (auto _data : module.data_list) {
            xdbg("init data");
            Frame frame{module_instance, {}, _data.offset, 1};
            Configuration config{store};
            config.set_frame(frame);
            auto r = config.exec().data[0];
            auto offset = r.to_i64();
            auto memory_addr = module_instance->memory_addr_list[_data.memory_index];
            auto & memory_instance = store->memory_list[memory_addr];
            for (auto b : _data.init) {
                memory_instance.data[offset++] = b;
            }
        }
        // todo run start?
        if (module.start.exit) {
            xdbg("running start function");
            invocate(module_instance->function_addr_list[module.start.function_index], {});
        }
    }

    void allocate(Module const & module, std::vector<ExternValue> extern_value_list, std::vector<Value> global_value) {
        xdbg("---------allocate--------------");
        for (auto p : extern_value_list) {
            switch (p.first) {
            case FUNCTION_EXT_INDEX:
                module_instance->function_addr_list.push_back(p.second);
                break;
            case TABLE_EXT_INDEX:
                module_instance->table_addr_list.push_back(p.second);
                break;
            case MEMORY_EXT_INDEX:
                module_instance->memory_addr_list.push_back(p.second);
                break;
            case GLOBAL_EXT_INDEX:
                module_instance->global_addr_list.push_back(p.second);
                break;
            }
        }

        for (auto & _function : module.function_list) {
            auto function_addr = store->allocate_wasm_function(module_instance, _function);
            module_instance->function_addr_list.push_back(function_addr);
        }

        for (auto & _table : module.table_list) {
            auto table_addr = store->allocate_table(_table.type);
            module_instance->table_addr_list.push_back(table_addr);
        }

        for (auto & _memory : module.memory_list) {
            auto memory_addr = store->allocate_memory(_memory.type);
            module_instance->memory_addr_list.push_back(memory_addr);
            xdbg("size: %d type: %d val:%d", module_instance->memory_addr_list.size(), _memory.type, memory_addr);
        }

        for (auto index = 0; index < module.global_list.size(); ++index) {
            auto & _global = module.global_list[index];
            auto global_addr = store->allocate_global(_global.type, global_value[index]);
            module_instance->global_addr_list.push_back(global_addr);
        }

        for (auto & _export : module.export_list) {
            // Variant<FunctionIndex, TableIndex, MemoryIndex, GlobalIndex> exportdesc;
            ExternValue extern_value{};
            switch (_export.type) {
            case EXPORT_TYPE_FUNC: {
                // FunctionIndex
                xdbg(" GET export_func name:%s  type:%d  index:%d", _export.name.c_str(), _export.type, _export.exportdesc);
                auto addr = module_instance->function_addr_list[_export.exportdesc];
                extern_value = std::make_pair(0, addr);
                break;
            }
            case EXPORT_TYPE_TABLE: {
                xdbg(" GET export_table name:%s  type:%d  index:%d", _export.name.c_str(), _export.type, _export.exportdesc);
                auto addr = module_instance->table_addr_list[_export.exportdesc];
                extern_value = std::make_pair(1, addr);
                break;
            }
            case EXPORT_TYPE_MEMORY: {
                xdbg(" GET export_memory name:%s  type:%d  index:%d", _export.name.c_str(), _export.type, _export.exportdesc);
                auto addr = module_instance->memory_addr_list[_export.exportdesc];
                extern_value = std::make_pair(2, addr);
                break;
            }
            case EXPORT_TYPE_GLOBAL: {
                xdbg(" GET export_global name:%s  type:%d  index:%d", _export.name.c_str(), _export.type, _export.exportdesc);
                auto addr = module_instance->global_addr_list[_export.exportdesc];
                extern_value = std::make_pair(3, addr);
                break;
            }
            }
            auto export_inst = ExportInstance{_export.name, extern_value};
            module_instance->export_list.push_back(export_inst);
        }
    }

    Result invocate(FunctionAddress function_addr, std::vector<Value> function_args) {
        Configuration config{store};
        return config.call(function_addr, function_args);
    }
};