#pragma once

#include "base/wasi-define.h"
class xtop_wasi_section_base {
public:
    xtop_wasi_section_base() = default;
    explicit xtop_wasi_section_base(byte_vec const & bv) : data{bv} {
    }
    virtual ~xtop_wasi_section_base() {
    }

    byte section_size();
    byte list_count();

protected:
    byte_vec data;
};
using xwasi_section_base_t = xtop_wasi_section_base;

#define section_class(name)                                                                                                                                                        \
    class xtop_wasi_##name##_section final : public xwasi_section_base_t {                                                                                                         \
    public:                                                                                                                                                                        \
        xtop_wasi_##name##_section();                                                                                                                                              \
        explicit xtop_wasi_##name##_section(byte_vec const & bv);                                                                                                                 \
        ~xtop_wasi_##name##_section() override;                                                                                                                                    \
    };                                                                                                                                                                             \
    using xwasi_##name##_section_t = xtop_wasi_##name##_section;

// Custom section 0
class xtop_wasi_custom_section final : public xwasi_section_base_t {
public:
    xtop_wasi_custom_section() = default;
    explicit xtop_wasi_custom_section(byte_vec const & bv);
    ~xtop_wasi_custom_section() override;
};
using xwasi_custom_section_t = xtop_wasi_custom_section;

// Type section 1
class xtop_wasi_type_section final : public xwasi_section_base_t {
public:
    xtop_wasi_type_section() = default;
    explicit xtop_wasi_type_section(byte_vec const & bv);
    ~xtop_wasi_type_section() override;

    void extract_type_list();
    std::pair<byte_vec, byte_vec> get_type(byte index);

private:
    std::vector<std::pair<byte_vec, byte_vec>> type_list;
};
using xwasi_type_section_t = xtop_wasi_type_section;

// Import section 2
class xtop_wasi_import_section final : public xwasi_section_base_t {
public:
    xtop_wasi_import_section() = default;
    explicit xtop_wasi_import_section(byte_vec const & bv);
    ~xtop_wasi_import_section() override;

    void extract_import_list();
    std::vector<byte_vec> import_list_to_code();
    byte_vec import_list_to_function();
    
private:
    std::map<std::string, byte_vec> import_list;  // {func_name,type_index}
};
using xwasi_import_section_t = xtop_wasi_import_section;

// Function section 3
class xtop_wasi_function_section final : public xwasi_section_base_t {
public:
    xtop_wasi_function_section() = default;
    explicit xtop_wasi_function_section(byte_vec const & bv);
    ~xtop_wasi_function_section() override;

    void extract_func_list();// from bytes;
    void import_func_list(byte_vec bv); // from import function

    byte get_func_type(byte index);

private:
    byte_vec func_list;
};
using xwasi_function_section_t = xtop_wasi_function_section;

// Table section 4
class xtop_wasi_table_section final : public xwasi_section_base_t {
public:
    xtop_wasi_table_section() = default;
    explicit xtop_wasi_table_section(byte_vec const & bv);
    ~xtop_wasi_table_section() override;
};
using xwasi_table_section_t = xtop_wasi_table_section;

// Memory section 5
class xtop_wasi_memory_section final : public xwasi_section_base_t {
public:
    xtop_wasi_memory_section() = default;
    explicit xtop_wasi_memory_section(byte_vec const & bv);
    ~xtop_wasi_memory_section() override;
};
using xwasi_memory_section_t = xtop_wasi_memory_section;

// Global section 6
class xtop_wasi_global_section final : public xwasi_section_base_t {
public:
    xtop_wasi_global_section() = default;
    explicit xtop_wasi_global_section(byte_vec const & bv);
    ~xtop_wasi_global_section() override;
};
using xwasi_global_section_t = xtop_wasi_global_section;

// Export section 7
class xtop_wasi_export_section final : public xwasi_section_base_t {
public:
    xtop_wasi_export_section() = default;
    explicit xtop_wasi_export_section(byte_vec const & bv);
    ~xtop_wasi_export_section() override;

    void extract_export_dict();
    byte get_import_func_index(std::string const & name);

private:
    std::map<std::string, std::pair<byte, byte>> func_name_dict;  //{name,{type,func_index}};
};
using xwasi_export_section_t = xtop_wasi_export_section;

// Start section 8
class xtop_wasi_start_section final : public xwasi_section_base_t {
public:
    xtop_wasi_start_section() = default;
    explicit xtop_wasi_start_section(byte_vec const & bv);
    ~xtop_wasi_start_section() override;
};
using xwasi_start_section_t = xtop_wasi_start_section;

// Element section 9
class xtop_wasi_element_section final : public xwasi_section_base_t {
public:
    xtop_wasi_element_section() = default;
    explicit xtop_wasi_element_section(byte_vec const & bv);
    ~xtop_wasi_element_section() override;
};
using xwasi_element_section_t = xtop_wasi_element_section;

// Code section 10
class xtop_wasi_code_section final : public xwasi_section_base_t {
public:
    xtop_wasi_code_section() = default;
    explicit xtop_wasi_code_section(byte_vec const & bv);
    ~xtop_wasi_code_section() override;

    void extract_code_list(); // from bytes;
    void import_code_list(std::vector<byte_vec> code_list); // from import function

    byte_vec & get_code_IR(byte index);

private:
    std::vector<byte_vec> code_IR_vec;
};
using xwasi_code_section_t = xtop_wasi_code_section;

// Data section 11
class xtop_wasi_data_section final : public xwasi_section_base_t {
public:
    xtop_wasi_data_section() = default;
    explicit xtop_wasi_data_section(byte_vec const & bv);
    ~xtop_wasi_data_section() override;
};
using xwasi_data_section_t = xtop_wasi_data_section;
