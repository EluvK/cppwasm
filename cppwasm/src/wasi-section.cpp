#include "wasi-section.h"


#include "assert.h"

byte xtop_wasi_section_base::section_size() {
    // ASSERT(!data.empty(),"try get empty section's size");
    return static_cast<byte>(data.size());
}

byte xtop_wasi_section_base::list_count() {
    ASSERT(!data.empty(), "try get empty section's list count");
    return data[0];
}

xtop_wasi_custom_section::xtop_wasi_custom_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_custom_section::~xtop_wasi_custom_section() {
}

xtop_wasi_type_section::xtop_wasi_type_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
    extract_type_list();
    // todo possible need to add a type_list to module or better way? store inside and expose interface;
}
xtop_wasi_type_section::~xtop_wasi_type_section() {
}
void xtop_wasi_type_section::extract_type_list() {
    uint32_t type_num = data[0];

    uint32_t pos = 1;
    for (auto index = 0; index < type_num; ++index) {
        uint32_t fun_type_ph = 1;
        ASSERT(data[pos] == 0x60, "should be 0x60,actualyl:0x%02x", data[pos]);
        byte_vec param_v, return_v;
        pos += fun_type_ph;
        uint32_t param_cnt = data[pos];

        std::printf("[type %d] : ", index);
        for (auto i = 1; i <= param_cnt; ++i) {
            param_v.emplace_back(data[i + pos]);
            std::printf("%02x ", data[i + pos]);
        }
        printf("-->");
        pos += param_cnt + 1;
        uint32_t return_cnt = data[pos];
        for (auto i = 1; i <= return_cnt; ++i) {
            return_v.emplace_back(data[i + pos]);
            std::printf("%02x ", data[pos + i]);
        }
        pos += return_cnt + 1;
        printf("\n");
        type_list.emplace_back(std::make_pair(param_v, return_v));
    }
    ASSERT(type_list.size() == type_num, "extract type list failed");
}
std::pair<byte_vec, byte_vec> xtop_wasi_type_section::get_type(byte index) {
    // todo error.
    return type_list.at(index);
}

xtop_wasi_import_section::xtop_wasi_import_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
    extract_import_list();
}
xtop_wasi_import_section::~xtop_wasi_import_section() {
}
void xtop_wasi_import_section::extract_import_list(){
    
    byte import_cnt = data[0];
    uint32_t pos = 1;
    for (auto index = 0; index < import_cnt; ++index) {
        byte mod_name_size = data[pos];
        std::string mod_name{};
        for (byte ns = 1; ns <= mod_name_size; ++ns) {
            mod_name += data[pos + ns];
        }
        pos+=(1+mod_name_size);
        byte func_name_size = data[pos];
        std::string func_name{};
        for(byte ns = 1;ns<=func_name_size;++ns){
            func_name+=data[pos+ns];
        }
        pos+=(func_name_size);
        xdbg("[%s : %s] : import_type:%d, type:%d", mod_name.c_str(), func_name.c_str(), data[pos + 1], data[pos + 2]);
        // todo ! diffenent if data[pos+1]!=0
        ASSERT(data[pos+1]==0,"function support only for now");
        import_list.insert({func_name, byte_vec{data[pos + 2]}});
        // func_name_dict.insert({export_name, std::make_pair(data[pos + name_size + 1], data[pos + name_size + 2])});
        pos += 3;
    }
}
std::vector<byte_vec> xtop_wasi_import_section::import_list_to_code(){
    std::vector<byte_vec> res{};
    xdbg("import_list_to_code");
    for(auto &pair : import_list){
        byte_vec func_code;
        func_code.push_back(0xff);
        for (auto c : pair.first)
            func_code.push_back(c);
        res.push_back(func_code);
    }

    return res;
}

byte_vec xtop_wasi_import_section::import_list_to_function() {
    byte_vec res{};
    for (auto & pair : import_list) {
        // todo ! import make all index -> byte_vec class
        res.push_back(pair.second[0]);
    }
    return res;
}

xtop_wasi_function_section::xtop_wasi_function_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
    // extract_func_list();
}
xtop_wasi_function_section::~xtop_wasi_function_section() {
}
void xtop_wasi_function_section::extract_func_list() {
    uint32_t func_cnt = data[0], pos = 1;
    uint32_t import_func_size = func_list.size();
    for (auto i = 0; i < import_func_size; ++i) {
        xdbg("[func %d (import)]->type: %d", i, func_list[i]);
    }
    for (auto i = import_func_size; i < func_cnt + import_func_size; ++i) {
        xdbg("[func %d]->type: %d", i, data[pos + i - import_func_size]);
        func_list.emplace_back(data[pos + i - import_func_size]);
    }
}
void xtop_wasi_function_section::import_func_list(byte_vec bv){
    func_list = bv;
}

byte xtop_wasi_function_section::get_func_type(byte index) {
    // todo error.
    return func_list.at(index);
}

xtop_wasi_table_section::xtop_wasi_table_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_table_section::~xtop_wasi_table_section() {
}

xtop_wasi_memory_section::xtop_wasi_memory_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_memory_section::~xtop_wasi_memory_section() {
}

xtop_wasi_global_section::xtop_wasi_global_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_global_section::~xtop_wasi_global_section() {
}

xtop_wasi_export_section::xtop_wasi_export_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
    extract_export_dict();
}
xtop_wasi_export_section::~xtop_wasi_export_section() {
}
void xtop_wasi_export_section::extract_export_dict() {
    byte export_cnt = data[0];
    uint32_t pos = 1;
    for (auto index = 0; index < export_cnt; ++index) {
        byte name_size = data[pos];
        std::string export_name{};
        for (byte ns = 1; ns <= name_size; ++ns) {
            export_name += data[pos + ns];
        }
        xdbg("[%s] : export_type:%d, index:%d", export_name.c_str(), data[pos + name_size + 1], data[pos + name_size + 2]);
        // todo ! diffenent if data[pos+1]!=0
        func_name_dict.insert({export_name, std::make_pair(data[pos + name_size + 1], data[pos + name_size + 2])});
        pos += name_size + 3;
    }
}
byte xtop_wasi_export_section::get_import_func_index(std::string const & name) {
    ASSERT(func_name_dict.find(name) != func_name_dict.end(), "import_func_dict dont have %s", name.c_str());
    ASSERT(func_name_dict.at(name).first == 0x00, "%s is not the func(type 0) name , type:%d", name.c_str(), func_name_dict.at(name).first);
    return func_name_dict.at(name).second;
}

xtop_wasi_start_section::xtop_wasi_start_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_start_section::~xtop_wasi_start_section() {
}

xtop_wasi_element_section::xtop_wasi_element_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_element_section::~xtop_wasi_element_section() {
}

xtop_wasi_code_section::xtop_wasi_code_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_code_section::~xtop_wasi_code_section() {
}
void xtop_wasi_code_section::extract_code_list() {
    uint32_t func_num = data[0];

    uint32_t pos = 1;

    // todo code_seciont should get import_code ph
    // todo when exec run at c++ local.
    uint32_t import_func_size = code_IR_vec.size();
    for(auto index = 0;index<import_func_size;++index){
        printf("[code %d] (import)", index);

        for (auto b = 1; b < code_IR_vec[index].size(); ++b) {
            printf("%02x ", code_IR_vec[index][b]);
        }
        printf("\n");
    }

    // todo 
    // code_IR_vec.push(cnt(import_func))

    for (auto index = import_func_size; index < func_num+import_func_size; ++index) {
        uint32_t func_byte_size = data[pos];
        // todo 区域变数 decl count set default 1 temp
        uint32_t decl_count = 1;
        pos += decl_count;
        uint32_t code_count = func_byte_size - decl_count;
        byte_vec code_IR{};
        printf("[code %d]", index);
        for (auto b = 1; b <= code_count; ++b) {
            // code IR
            code_IR.push_back(data[b + pos]);
            std::printf("%02x ", data[b + pos]);
        }
        std::printf("\n");
        assert(code_IR.size() == code_count);
        assert(code_IR.back() == 0x0b);  // func end tag
        code_IR_vec.push_back(code_IR);
        pos += func_byte_size;
    }
}
void xtop_wasi_code_section::import_code_list(std::vector<byte_vec> bv){
    // import code list from import_section.
    xdbg("import_code_list , code size: %d",bv.size());
    code_IR_vec = bv;
}

byte_vec & xtop_wasi_code_section::get_code_IR(byte index) {
    return code_IR_vec[index];
}

xtop_wasi_data_section::xtop_wasi_data_section(byte_vec const & bv) : xtop_wasi_section_base{bv} {
}
xtop_wasi_data_section::~xtop_wasi_data_section() {
}
