#include "cppwasm/wasi-execution.h"

Result Configuration::call(FunctionAddress function_addr, std::vector<Value> & function_args) {
    auto & function = store.function_list[function_addr];
    xdbg("-----------------Configuration call--------------");
    xdbg("call %s function_addr: %d (list size:%d)", function.GetType() == 1 ? "wasmfunc" : "hostfunc", function_addr, store.function_list.size());

    // todo assert func type match args.

    switch (function.GetType()) {
    case 1: {
        // WASM func
        auto const & func = function.GetConstRef<WasmFunc>();
        auto const & value_type_vec = func.code.local_list;
        for (auto const & _value_type : value_type_vec) {
            function_args.push_back(Value::newValue(_value_type, 0));
        }
        Frame frame{func.module, function_args, func.code.expr, func.type.rets.data.size()};
        set_frame(frame);
        return exec();
    }
    case 2: {
        // LOCAL func
        auto const & host_func = function.GetConstRef<HostFunc>();
        // auto func_class = host_func.callable;
        auto const & args_type = host_func.type.args.data;
        // for (auto p : args_type)
        //     xdbg("0x%02x", p);
        // for(auto p: function_args)
        //     xdbg("0x%02x", p.to_i32());
        auto r = host_func.exec(function_args, args_type);
        // auto const & type = func.type.rets.data;
        return r;
    }
    default:
        xerror("cppwasm: unknow function type %d", function.GetType());
    }

    return {};
}

Result Configuration::exec() {
    xdbg("--------exec code: ----------");
    auto instruction_list = frame.expr.data;
    auto size = instruction_list.size();
    xdbg("pc: %d ,sz: %d", pc, size);
    while (pc < size) {
        Instruction * i = &instruction_list[pc];
        ArithmeticLogicUnit::exec(this, i);
        xdbg("PC: %d", pc);
        pc += 1;
    }
    auto _arity = frame.arity;
    Result res;
    xdbg(" exec result: stack len: %d arity: %d", stack.len(), _arity);
    while (_arity--) {
        // xdbg("  stack len:%d", stack.len());
        auto r = stack.pop();
        // xdbg("  type:%d",r.GetType());
        switch (r.GetType()) {
        case STACK_UNIT_VALUE_TYPE:
            res.data.push_back(r.GetRef<Value>());
            xdbg("Result: %d", r.GetRef<Value>().to_i32());
            break;
        default:
            xerror("error type  %d in stack should all be Value Result ", r.GetType());
            break;
        }
    }
    // todo can also check this pop() == frame.
    ASSERT(stack.pop().GetType() == STACK_UNIT_FRAME_TYPE, "error?");
    // xdbg("RESULT??");
    return res;
}