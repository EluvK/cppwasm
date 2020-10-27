#pragma once

#include "base/Variant.h"
#include "wasi-binary.h"
#include "wasi-fund.hpp"

#include <list>
#include <stack>

class Store {
public:
    Store(){};
};

using stack_unit = Variant<Value, Label, Frame>;
#define STACK_UNIT_VALUE_TYPE 1
#define STACK_UNIT_LABEL_TYPE 2
#define FRAME_UNIT_LABEL_TYPE 3
/**
 * @brief Besides the store, most instructions interact with an implicit stack. The stack contains three kinds of entries:
 * Values: the operands of instructions.
 * Labels: active structured control instructions that can be targeted by branches.
 * Activations: the call frames of active function calls.
 * These entries can occur on the stack in any order during the execution of a program. Stack entries are described by abstract syntax as follows.
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
    // std::stack<stack_unit> data;
};

class Configuration {
public:
    Configuration(Store _store) {
        m_store = _store;
        m_stack = Stack();
        depth = 0;
        pc = 0;
    };

    Label get_label(uint32_t i) {
        return m_stack.get_nth_label(i);
    }

    void set_frame(Frame _frame) {
        m_frame = _frame;
        m_stack.append(m_frame);
        m_stack.append(Label(m_frame.arity, m_frame.expr.data.size() - 1));
    }

    // Result call();
    // Result exec();

private:
    Store m_store;
    Stack m_stack;
    Frame m_frame;
    uint32_t depth;
    uint32_t pc;
};

class Machine {
public:
    Machine() {
        // module
        m_store = Store();
    }

    Store m_store;
};