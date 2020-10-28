#pragma once
#include "base/Variant.h"
#include "base/wasi-define.h"
#include "wasi-alu.h"
#include "wasi-binary.h"
class Value {
public:
    Value() {
    }
    Value(byte _data) : data{_data} {
    }
    Value(byte _type, byte_vec _data) {
        type = _type;
        data = _data;
    }
    int32_t to_i32() {
    }
    int64_t to_i64() {
    }
    float to_f32() {
    }
    double to_f64() {
    }

private:
    byte type;
    byte_vec data;
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
    WasmFunc(FunctionType _type, ModuleInstance _module, Function _code) : type{_type}, module{_module}, code{_code} {
    }

    FunctionType type;
    ModuleInstance module;
    Function code;
};

/**
 * @brief A host function is a function expressed outside WebAssembly but passed to a module as an import. The definition
 * and behavior of host functions are outside the scope of this specification. For the purpose of this
 * specification, it is assumed that when invoked, a host function behaves non-deterministically, but within certain
 * constraints that ensure the integrity of the runtime.
 *
 */
class HostFunc {
public:
    HostFunc(FunctionType _type, bool _callable) : type{_type}, callable{_callable} {
    }

    FunctionType type;
    bool callable{false};
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
        data.reserve(size * 65536);  // todo make it static const memory_page_size;
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

    FunctionAddress allocate_wasm_function(ModuleInstance module, Function function) {
        FunctionAddress function_address = function_list.size();
        FunctionType function_type = module.type_list[function.type_index.data];
        WasmFunc wasmfunc{function_type, module, function};
        function_list.push_back(wasmfunc);
        return function_address;
    }

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

private:
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
    Frame(ModuleInstance _module, std::vector<Value> _local_list, Expression _expr, int64_t _arity) : module{_module}, local_list{_local_list}, expr{_expr}, arity{_arity} {
    }

    ModuleInstance module;
    std::vector<Value> local_list;
    Expression expr;
    int64_t arity;
};

// todo may move it somewhere else?
using stack_unit = Variant<Value, Label, Frame>;
#define STACK_UNIT_VALUE_TYPE 1
#define STACK_UNIT_LABEL_TYPE 2
#define FRAME_UNIT_LABEL_TYPE 3
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
        auto u = data.back();
        data.pop_back();
        return u;
    }
    // todo complete it.
    Label get_nth_label(uint32_t i) {
        while (i--) {
            for (auto index = 0; index < data.size(); ++index) {
                if (data[index].GetType() == STACK_UNIT_LABEL_TYPE && i == 0) {
                    return data[index].GetRef<Label>();
                }
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
// todo complete it.
class Configuration {
public:
    Configuration(Store _store) : store{_store} {
    }

    Store store;
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

    Result call(FunctionAddress function_addr, std::vector<Value> function_args) {
    }

    Result exec() {
        auto instruction_list = frame.expr.data;
        auto size = instruction_list.size();
        while (pc < size) {
            Instruction * i = &instruction_list[pc];
            ArithmeticLogicUnit::exec(this, i);
            pc += 1;
        }
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
    }
    ModuleInstance module_instance{};
    Store store{};

    void instantiate(Module module, std::vector<ExternValue> extern_value_list) {
        module_instance.type_list = module.type_list;

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
            Frame frame{aux, {}, _global.expr, 1};
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
            auto table_addr = module_instance.table_addr_list[_element.table_index.data];
            auto & table_instance = store.table_list[table_addr];
            for (auto index = 0; index < _element.init.size(); ++index) {
                table_instance.element_list[offset + index] = _element.init[index].data;
            }
        }

        for (auto _data : module.data_list) {
            xdbg("init data");
            Frame frame{module_instance, {}, _data.offset, 1};
            Configuration config{store};
            config.set_frame(frame);
            auto r = config.exec().data[0];
            auto offset = r.to_i64();
            auto memory_addr = module_instance.memory_addr_list[_data.memory_index.data];
            auto & memory_instance = store.memory_list[memory_addr];
            for (auto b : _data.init) {
                memory_instance.data[offset++] = b;
            }
        }

        // todo run start?
    }

    void allocate(Module module, std::vector<ExternValue> extern_value_list, std::vector<Value> global_value) {
        for (auto p : extern_value_list) {
            switch (p.first) {
            case FUNCTION_EXT_INDEX:
                module_instance.function_addr_list.push_back(p.second);
                break;
            case TABLE_EXT_INDEX:
                module_instance.table_addr_list.push_back(p.second);
                break;
            case MEMORY_EXT_INDEX:
                module_instance.memory_addr_list.push_back(p.second);
                break;
            case GLOBAL_EXT_INDEX:
                module_instance.gloabl_addr_list.push_back(p.second);
                break;
            }
        }

        for (auto _function : module.function_list) {
            auto function_addr = store.allocate_wasm_function(module_instance, _function);
            module_instance.function_addr_list.push_back(function_addr);
        }

        for (auto _table : module.table_list) {
            auto table_addr = store.allocate_table(_table.type);
            module_instance.table_addr_list.push_back(table_addr);
        }

        for (auto _memory : module.memory_list) {
            auto memory_addr = store.allocate_memory(_memory.type);
            module_instance.memory_addr_list.push_back(memory_addr);
        }

        for (auto index = 0; index < module.global_list.size(); ++index) {
            auto _global = module.global_list[index];
            auto global_addr = store.allocate_global(_global.type, global_value[index]);
        }

        for (auto _export : module.export_list) {
            // Variant<FunctionIndex, TableIndex, MemoryIndex, GlobalIndex> exportdesc;
            ExternValue extern_value{};
            switch (_export.exportdesc.GetType()) {
            case 1: {
                // FunctionIndex
                auto addr = module_instance.function_addr_list[_export.exportdesc.GetRef<FunctionIndex>().data];
                extern_value = std::make_pair(0, addr);
                break;
            }
            case 2: {
                auto addr = module_instance.table_addr_list[_export.exportdesc.GetRef<TableIndex>().data];
                extern_value = std::make_pair(1, addr);
                break;
            }
            case 3: {
                auto addr = module_instance.memory_addr_list[_export.exportdesc.GetRef<MemoryIndex>().data];
                extern_value = std::make_pair(2, addr);
                break;
            }
            case 4: {
                auto addr = module_instance.gloabl_addr_list[_export.exportdesc.GetRef<GlobalIndex>().data];
                extern_value = std::make_pair(3, addr);
                break;
            }
            }
            auto export_inst = ExportInstance{_export.name, extern_value};
            module_instance.export_list.push_back(export_inst);
        }
    }

    Result invocate(FunctionAddress function_addr, std::vector<Value> function_args) {
        Configuration config{store};
        return config.call(function_addr, function_args);
    }
};