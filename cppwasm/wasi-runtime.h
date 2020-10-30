#pragma once
#include "wasi-binary.h"
#include "wasi-execution.h"

class Runtime {
public:
    // todo make bool->function.
    Runtime(Module const & module, std::map<std::string, std::map<std::string, bool>> imps) {
        // todo check imp

        // todo import_list?
        std::vector<ExternValue> extern_value_list;
        for (auto _import : module.import_list) {
            switch (_import.desc) {
                // extern_function = 0x00
                // extern_table = 0x01
                // extern_memory = 0x02
                // extern_global = 0x03
            case 0x00: {
                HostFunc hf{module.type_list[_import.desc], imps[_import.module][_import.name]};  // bool callable?
                auto addr = machine.store.allocate_host_function(hf);
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
                auto addr = store.allocate_global(_import.importdesc.GetRef<GlobalType>(), Value{});
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
        for (auto _export : machine.module_instance.export_list) {
            // todo check if necessary to distingush type.
            if (_export.name == name /* && _export.value is functionAddress*/) {
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
        FunctionInstance func = machine.store.function_list[addr];
        std::vector<Value> v_args{};
        for(auto &a:args){
            switch(a.GetType()){
                case INPUT_TYPE_I64:{
                    xdbg("Get Value I64 %d", a.GetConstRef<int64_t>());
                    v_args.push_back(Value(a.GetConstRef<int64_t>()));
                    break;
                }
                case INPUT_TYPE_F64:{
                    v_args.push_back(Value(a.GetConstRef<double>()));
                    break;
                }
                default:
                    xerror("cppwasm: UNKNOW INPUT TYPE");
            }
        }

        return exec_accu(name,v_args);
    }

    
    Machine machine;
    Store store;
};