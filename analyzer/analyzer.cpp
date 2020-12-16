#include "analyzer.h"

bool analyzer::analyse_module(Module const & mod) {
    mod_figure.clear();
    mod_figure["type_list"] = analyse_type(mod);
    mod_figure["func_list"] = analyse_func(mod);
    // table
    // memory
    // global
    // element
    // data
    // start
    mod_figure["import_list"] = analyse_import(mod);
    mod_figure["export_list"] = analyse_export(mod);

    mod_figure["all_function_list"] = analyse_all_function(mod);
}

json analyzer::analyse_type(Module const & mod) {
    auto type_list = mod.type_list;
    json res;
    auto index = 0;
    for (auto _t : type_list) {
        json _ts;
        std::string type_index = "type " + std::to_string(index++);
        _ts["args"] = {_t.args.data};
        _ts["rets"] = {_t.rets.data};

        res[type_index] = _ts;
    }
    return res;
}

json analyzer::analyse_func(Module const & mod) {
    auto func_list = mod.function_list;
    json res;
    auto index = 0;
    for (auto _func : func_list) {
        json _fs;
        std::string func_index = "func " + std::to_string(index++);
        _fs["type_index"] = _func.type_index;
        _fs["local_list_type"] = {_func.local_list};
        _fs["expr_size"] = _func.expr.data.size();

        auto opcodes = convert_expr2str(_func.expr.data);
        _fs["expr_content"] = {opcodes};

        res[func_index] = _fs;
    }
    return res;
}

json analyzer::analyse_import(Module const & mod) {
    auto import_list = mod.import_list;
    json res;
    auto index = 0;
    for(auto _imp:import_list){
        switch (_imp.desc) {
        case EXPORT_TYPE_FUNC: {
            json _is_func;
            _is_func["module"] = _imp.module;
            _is_func["name"] = _imp.name;
            _is_func["index"] = _imp.importdesc.GetConstRef<TypeIndex>();

            res["function"].push_back(_is_func);
            break;
        }
        case EXPORT_TYPE_TABLE: {
            json _is_table;
            _is_table["module"] = _imp.module;
            _is_table["name"] = _imp.name;
            // _is_table["index"] = _imp.importdesc.GetConstRef<TableType>();

            res["table"].push_back(_is_table);
            break;
        }
        case EXPORT_TYPE_MEMORY: {
            json _is_memory;
            _is_memory["module"] = _imp.module;
            _is_memory["name"] = _imp.name;
            // _is_memory["index"] = _imp.importdesc.GetConstRef<MemoryType>();

            res["memory"].push_back(_is_memory);
            break;
        }
        case EXPORT_TYPE_GLOBAL: {
            json _is_global;
            _is_global["module"] = _imp.module;
            _is_global["name"] = _imp.name;
            // _is_global["index"] = _imp.importdesc.GetConstRef<GlobalType>();

            res["global"].push_back(_is_global);
            break;
        }
        default : 
            xerror("analyzer: wrong export type");
        }
    }

    return res;
}
json analyzer::analyse_export(Module const & mod) {
    auto export_list = mod.export_list;
    json res;
    for(auto _exp : export_list){
        switch (_exp.type) {
        case EXPORT_TYPE_FUNC: {
            json _es_func;
            _es_func["name"] = _exp.name;
            _es_func["index"] = _exp.exportdesc;

            res["function"].push_back(_es_func);
            break;
        }
        case EXPORT_TYPE_TABLE: {
            json _es_table;
            _es_table["name"] = _exp.name;
            _es_table["index"] = _exp.exportdesc;

            res["table"].push_back(_es_table);
            break;
        }
        case EXPORT_TYPE_MEMORY: {
            json _es_memory;
            _es_memory["name"] = _exp.name;
            _es_memory["index"] = _exp.exportdesc;

            res["memory"].push_back(_es_memory);
            break;
        }
        case EXPORT_TYPE_GLOBAL: {
            json _es_global;
            _es_global["name"] = _exp.name;
            _es_global["index"] = _exp.exportdesc;

            res["global"].push_back(_es_global);
            break;
        }
        default : 
            xerror("analyzer: wrong export type");
        }
    }
    return res;
}

json analyzer::analyse_all_function(Module const & mod){
    json res;
    json import_res = analyse_import(mod);
    json export_res = analyse_export(mod);
    json func_res = analyse_func(mod);
    for(auto _imp_func: import_res["function"]){
        res.push_back(_imp_func);
    }
    for(auto _exp_func: export_res["function"]){
        json _detail;
        int index = _exp_func["index"];
        std::string func_index = "func " + std::to_string(index);
        _detail.push_back(func_res[func_index]);
        _exp_func["detail"] = _detail;
        res.push_back(_exp_func);
    }
    return res;
}

void analyzer::dump_result() {
    std::stringstream ss;
    ss << mod_figure;
    std::printf("%s\n", ss.str().c_str());
}

std::vector<std::string> analyzer::convert_expr2str(std::vector<Instruction> const & input){
    std::vector<std::string> res;
    for(auto _i:input){
        auto str = instruction_to_string(_i.opcode);
        res.push_back(str);
    }
    return res;
}

void usage() {
    std::printf("usage:");
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        usage();
    }
    analyzer AN;
    char * file_path = argv[1];
    Module mod{file_path};
    AN.analyse_module(mod);
    AN.dump_result();
    return 0;
}