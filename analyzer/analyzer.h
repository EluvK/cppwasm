#pragma once
#include "../cppwasm/wasi-binary.h"

#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map.hpp"

#include <sstream>

template<class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;

using unordered_json = nlohmann::basic_json<my_workaround_fifo_map>;
using json = unordered_json;


class analyzer {
public:
    bool analyse_module(Module const & mod);
    json analyse_type(Module const & mod);
    json analyse_func(Module const & mod);
    json analyse_import(Module const & mod);
    json analyse_export(Module const & mod);
    json analyse_all_function(Module const & mod);

    void dump_result();

    json mod_figure;

private:
    std::vector<std::string> convert_expr2str(std::vector<Instruction> const & input);
};