#pragma once
#include "wasi-binary.h"
#include "wasi-execution.h"

class Runtime {
public:
    Runtime(Module const & module, std::map<std::string, std::map<std::string, host_func_base_ptr>> & imps) : machine{} {
        // todo import_list?
        std::vector<ExternValue> extern_value_list;
        for (auto _import : module.import_list) {
            switch (_import.desc) {
                // extern_function = 0x00
                // extern_table = 0x01
                // extern_memory = 0x02
                // extern_global = 0x03
            case 0x00: {
                HostFunc hf{module.type_list[_import.desc], imps[_import.module][_import.name]};  // host_func_base_ptr
                auto addr = machine.store->allocate_host_function(hf);
                extern_value_list.push_back(std::make_pair(FUNCTION_EXT_INDEX, addr));
                break;
            }
            case 0x01: {
                // todo ? how table import
                // TableAddress addr{store.table_list.size()};
                // auto table = imps[_import.module][_import.name]; // table tableinstance
                // store.table_list.push_back(table);
                // extern_value_list.push_back(std::make_pair(TABLE_EXT_INDEX, addr));
                break;
            }
            case 0x02: {
                // MemoryAddress addr{store.memory_list.size()};
                // auto memory = imps[_import.module][_import.name];  // memory: memoryinstance
                // memory.grow(_import.importdesc.GetRef<MemoryType>().limits.n);
                // store.memory_list.push_back(memory);
                // extern_value_list.push_back(std::make_pair(MEMORY_EXT_INDEX,addr));
                break;
            }
            case 0x03: {
                // todo ? how global import
                auto addr = machine.store->allocate_global(_import.importdesc.GetRef<GlobalType>(), Value{});
                extern_value_list.push_back(std::make_pair(GLOBAL_EXT_INDEX, addr));
                break;
            }
            default:
                xerror("cppwasm error import desc");
            }
        }
        machine.instantiate(module, extern_value_list);
    }

    FunctionAddress func_addr(std::string name) {
        // xdbg("find_function_index: %s", name.c_str());
        for (auto _export : machine.module_instance->export_list) {
            // xdbg(" list name: %s [%d, %d]", _export.name.c_str(), _export.value.first, _export.value.second);
            if (_export.name == name && _export.value.first == FUNCTION_EXT_INDEX) {
                return _export.value.second;
            }
        }
        xerror("cppwasm, function not found");
        return -1;
    }

    // todo Variant the Result.
    Result exec_accu(std::string const & name, std::vector<Value> v_args) {
        FunctionAddress addr = func_addr(name);
        return machine.invocate(addr, v_args);
    }

    Result exec(std::string const & name, std::vector<InputType> args) {
        FunctionAddress addr = func_addr(name);
        FunctionInstance func = machine.store->function_list[addr];
        std::vector<Value> v_args{};
        for (auto & a : args) {
            v_args.push_back(Value::newValue(a));
        }

        return exec_accu(name, v_args);
    }

    Machine machine;
    // todo make store in runtime && observer_ptr in machine
    // Store store;
};