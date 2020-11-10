#pragma once
#include "base/Variant.h"
#include "base/wasi-define.h"
// #include "wasi-alu.h"
#include "wasi-binary.h"

using InputType = Variant<int64_t, double, std::string>;
#define TYPE_I64 1
#define TYPE_F64 2
#define TYPE_STR 3

class Value {
public:
    Value() {
    }
    Value(byte_vec bv) : raw_data{bv} {
    }
    Value(int32_t i32) : raw_data{I_encode(i32)} {
    }
    Value(int64_t i64) : raw_data{I_encode(i64)} {
    }
    Value(uint32_t u32) : raw_data{U_encode(u32)} {
    }
    Value(uint64_t u64) : raw_data{U_encode(u64)} {
    }
    Value(float f32) : raw_data{F_encode(static_cast<double>(f32))} {
    }
    Value(double f64) : raw_data{F_encode(f64)} {
    }
    Value(std::string str) : raw_data{S_encode(str)} {
    }

    int32_t to_i32() {
        return static_cast<int32_t>(I_decode(raw_data));
    }
    int64_t to_i64() {
        return I_decode(raw_data);
    }
    uint32_t to_u32() {
        return static_cast<uint32_t>(U_decode(raw_data));
    }
    uint64_t to_u64() {
        return U_decode(raw_data);
    }
    float to_f32() {
        return static_cast<float>(F_decode(raw_data));
    }
    double to_f64() {
        return F_decode(raw_data);
    }
    std::string to_string() {
        return S_decode(raw_data);
    }

    static Value newValue(InputType data) {
        switch (data.GetType()) {
        case TYPE_I64:
            return Value(data.GetConstRef<int64_t>());
        case TYPE_F64:
            return Value(data.GetConstRef<double>());
        case TYPE_STR:
            return Value(data.GetConstRef<std::string>());
        default:
            xerror("cppwasm unknow input type");
        }
    }

private:
    byte_vec raw_data;
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
    std::vector<GlobalAddress> gloabl_addr_list;
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
        if (type.limits.m && size + n > type.limits.n) {
            // todo can add memory limit test;
            xerror("pywasm: out of memory limit");
        }
        // todo make it static const 2^16 memory_page
        if (size + n > 65536) {
            xerror("pywasm: out of memory limit");
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
            xerror("cppwasm empty stack!");
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
        case instruction::nop:
            xerror("cppwasm: not support nop && unreachable");
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
        case instruction::set_global:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i32_load:
            i32_load(config, i);
            break;
        case instruction::i64_load:
            i64_load(config, i);
            break;
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
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i32_store:
            i32_store(config, i);
            break;
        case instruction::i64_store:
            i64_store(config, i);
            break;
        case instruction::f32_store:
            xerror("false");
        case instruction::f64_store:
            f64_store(config, i);
            break;
        case instruction::i32_store8:
        case instruction::i32_store16:
        case instruction::i64_store8:
        case instruction::i64_store16:
        case instruction::i64_store32:
        case instruction::current_memory:
        case instruction::grow_memory:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i32_const:
            i32_const(config, i);
            break;
        case instruction::i64_const:
            i64_const(config, i);
            break;
        case instruction::f32_const:
        case instruction::f64_const:
        case instruction::i32_eqz:
        case instruction::i32_eq:
        case instruction::i32_ne:
        case instruction::i32_lts:
        case instruction::i32_ltu:
        case instruction::i32_gts:
        case instruction::i32_gtu:
        case instruction::i32_les:
        case instruction::i32_leu:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i32_ges:
            i32_ges(config, i);
            break;
        case instruction::i32_geu:
        case instruction::i64_eqz:
        case instruction::i64_eq:
        case instruction::i64_ne:
        case instruction::i64_lts:
        case instruction::i64_ltu:
        case instruction::i64_gts:
        case instruction::i64_gtu:
        case instruction::i64_les:
        case instruction::i64_leu:
        case instruction::i64_ges:
        case instruction::i64_geu:
        case instruction::f32_eq:
        case instruction::f32_ne:
        case instruction::f32_lt:
        case instruction::f32_gt:
        case instruction::f32_le:
        case instruction::f32_ge:
        case instruction::f64_eq:
        case instruction::f64_ne:
        case instruction::f64_lt:
        case instruction::f64_gt:
        case instruction::f64_le:
        case instruction::f64_ge:
        case instruction::i32_clz:
        case instruction::i32_ctz:
        case instruction::i32_popcnt:
            xdbg("instruction: 0x%02x", i->opcode);
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
        case instruction::i32_divu:
        case instruction::i32_rems:
        case instruction::i32_remu:
        case instruction::i32_and:
        case instruction::i32_or:
        case instruction::i32_xor:
        case instruction::i32_shl:
        case instruction::i32_shrs:
        case instruction::i32_shru:
        case instruction::i32_rotl:
        case instruction::i32_rotr:
        case instruction::i64_clz:
        case instruction::i64_ctz:
        case instruction::i64_popcnt:
        case instruction::i64_add:
        case instruction::i64_sub:
        case instruction::i64_mul:
        case instruction::i64_divs:
        case instruction::i64_divu:
        case instruction::i64_rems:
        case instruction::i64_remu:
        case instruction::i64_and:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i64_or:
            i64_or(config, i);
            break;
        case instruction::i64_xor:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i64_shl:
            i64_shl(config, i);
            break;
        case instruction::i64_shrs:
        case instruction::i64_shru:
        case instruction::i64_rotl:
        case instruction::i64_rotr:
        case instruction::f32_abs:
        case instruction::f32_neg:
        case instruction::f32_ceil:
        case instruction::f32_floor:
        case instruction::f32_trunc:
        case instruction::f32_nearest:
        case instruction::f32_sqrt:
        case instruction::f32_add:
        case instruction::f32_sub:
        case instruction::f32_mul:
        case instruction::f32_div:
        case instruction::f32_min:
        case instruction::f32_max:
        case instruction::f32_copysign:
        case instruction::f64_abs:
        case instruction::f64_neg:
        case instruction::f64_ceil:
        case instruction::f64_floor:
        case instruction::f64_trunc:
        case instruction::f64_nearest:
        case instruction::f64_sqrt:
        case instruction::f64_add:
        case instruction::f64_sub:
        case instruction::f64_mul:
        case instruction::f64_div:
        case instruction::f64_min:
        case instruction::f64_max:
        case instruction::f64_copysign:
        case instruction::i32_wrap_i64:
        case instruction::i32_trunc_sf32:
        case instruction::i32_trunc_uf32:
        case instruction::i32_trunc_sf64:
        case instruction::i32_trunc_uf64:
        case instruction::i64_extend_si32:
            xdbg("instruction: 0x%02x", i->opcode);
            break;
        case instruction::i64_extend_ui32:
            i64_extend_ui32(config, i);
            break;
        case instruction::i64_trunc_sf32:
        case instruction::i64_trunc_uf32:
        case instruction::i64_trunc_sf64:
        case instruction::i64_trunc_uf64:
        case instruction::f32_convert_si32:
        case instruction::f32_convert_ui32:
        case instruction::f32_convert_si64:
        case instruction::f32_convert_ui64:
        case instruction::f32_demote_f64:
        case instruction::f64_convert_si32:
        case instruction::f64_convert_ui32:
        case instruction::f64_convert_si64:
        case instruction::f64_convert_ui64:
        case instruction::f64_promote_f32:
        case instruction::i32_reinterpret_f32:
        case instruction::i64_reinterpret_f64:
        case instruction::f32_reinterpret_i32:
        case instruction::f64_reinterpret_i64:

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
        xerror("haven't write this ins");
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
        xdbg("instruction: get_local %d", config->stack.back().GetRef<Value>().to_i32());
    }

    static void set_local(Configuration * config, Instruction * i) {
        xdbg("instruction: set_local");
        auto ptr = dynamic_cast<args_get_local *>(i->args.get());
        auto v = config->stack.pop();
        ASSERT(v.GetType() == STACK_UNIT_VALUE_TYPE, "non value type to be set local");
        config->frame.local_list[ptr->data] = v.GetRef<Value>();
    }

    static void tee_local(Configuration * config, Instruction * i) {
        xdbg("instruction: tee_local");
        auto ptr = dynamic_cast<args_get_local *>(i->args.get());
        auto v = config->stack.back();
        ASSERT(v.GetType() == STACK_UNIT_VALUE_TYPE, "non value type to be set local");
        config->frame.local_list[ptr->data] = v.GetRef<Value>();
    }

    static void get_global(Configuration * config, Instruction * i) {
    }

    static void set_global(Configuration * config, Instruction * i) {
    }
    //..........

    static void mem_load(Configuration * config, Instruction * i, int64_t size) {
    }

    static void i32_load(Configuration * config, Instruction * i) {
        std::size_t size = 4;
        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto const & memory = config->store->memory_list[memory_addr];
        auto offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        auto addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: i32_load  addr:%d offset:%d", addr, offset);
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        byte_vec bv(memory.data.begin() + addr, memory.data.begin() + addr + size);
        Value r{I_decode(bv)};
        xdbg("         - : load val into stack: %d", r.to_i32());
        config->stack.append(r);
    }

    static void i64_load(Configuration * config, Instruction * i) {
        std::size_t size = 8;
        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto const & memory = config->store->memory_list[memory_addr];
        auto offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        auto addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: i64_load  addr:%d offset:%d", addr, offset);
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        byte_vec bv(memory.data.begin() + addr, memory.data.begin() + addr + size);
        Value r{I_decode(bv)};
        config->stack.append(r);
    }

    static void mem_store(Configuration * config, Instruction * i, int64_t size) {
    }

    static void i32_store(Configuration * config, Instruction * i) {
        std::size_t size = 4;
        auto r = config->stack.pop();

        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];
        auto offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        auto addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: i32_store  addr:%d offset:%d", addr, offset);
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        auto & mem = memory.data;
        auto data = I_encode(r.GetRef<Value>().to_i32());

        for (auto index = 0; index < std::min(size, data.size()); ++index) {
            mem[index + addr] = data[index];
        }
        // printf("\n");
        // for(auto index = 0;index<32;++index){
        //     printf("0x%02x ",config->store->memory_list[0].data[index]);
        // }
        // printf("\n");
    }

    static void i64_store(Configuration * config, Instruction * i) {
        std::size_t size = 8;
        auto r = config->stack.pop();

        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];
        auto offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        auto addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: i64_store  addr:%d offset:%d", addr, offset);
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        auto & mem = memory.data;
        auto data = I_encode(r.GetRef<Value>().to_i64());
        for (auto index = 0; index < std::min(size, data.size()); ++index) {
            mem[index + addr] = data[index];
        }
    }

    static void f64_store(Configuration * config, Instruction * i) {
        std::size_t size = 8;
        auto r = config->stack.pop();

        auto memory_addr = config->frame.module->memory_addr_list[0];
        auto & memory = config->store->memory_list[memory_addr];

        auto offset = dynamic_cast<args_load_store *>(i->args.get())->data.second;
        auto addr = config->stack.pop().GetRef<Value>().to_i32() + offset;
        xdbg("instruction: f64_store  addr:%d offset:%d", addr, offset);
        if (addr < 0 || addr + size > memory.data.size()) {
            // todo  make it exception.
            xerror("cppwasm: out of bounds memory access");
        }
        auto & mem = memory.data;
        // todo check tofloat
        auto data = I_encode(r.GetRef<Value>().to_f64());
        mem.insert(mem.begin() + addr, data.begin(), data.begin() + size);
    }

    static void i32_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_i32_count *>(i->args.get())->data));
        xdbg("instruction: i32_const %d", config->stack.back().GetRef<Value>().to_i32())
    }

    static void i64_const(Configuration * config, Instruction * i) {
        config->stack.append(Value(dynamic_cast<args_i32_count *>(i->args.get())->data));
        xdbg("instruction: i32_const %d", config->stack.back().GetRef<Value>().to_i64())
    }

    static void i32_ges(Configuration * config, Instruction * i) {
        xdbg("instruction: i32_ges");
        auto b = config->stack.pop().GetRef<Value>().to_i32();
        auto a = config->stack.pop().GetRef<Value>().to_i32();
        int32_t c = a >= b ? 1 : 0;
        config->stack.append(Value(c));
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
        // todo ? exception
        if (b == 0)
            xerror("cppwasm : integer divide by zero");
        auto c = Value(a / b);
        config->stack.append(c);
    }

    static void i64_or(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a | b);
        config->stack.append(c);
    }

    static void i64_shl(Configuration * config, Instruction * i) {
        auto b = config->stack.pop().GetRef<Value>().to_i64();
        auto a = config->stack.pop().GetRef<Value>().to_i64();
        auto c = Value(a << (b % 0x40));
        config->stack.append(c);
    }

    static void i64_extend_ui32(Configuration * config, Instruction * i) {
        xdbg("instruction: i64_extend_ui32");
        auto a = config->stack.pop().GetRef<Value>().to_u32();
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
                aux.gloabl_addr_list.push_back(p.second);
            }
        }
        for (auto & _global : module.global_list) {
            xdbg("init global value");
            // todo check this shared ptr liveness
            Frame frame{std::make_shared<ModuleInstance>(aux), {}, _global.expr, 1};
            Configuration config{store};
            config.set_frame(frame);
            auto r = config.exec().data[0];
            // todo how? Value->result  Result = list[Value]
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
                module_instance->gloabl_addr_list.push_back(p.second);
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
                auto addr = module_instance->gloabl_addr_list[_export.exportdesc];
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