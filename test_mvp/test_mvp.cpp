#include "test_mvp.h"

TEST_F(cppwasm_test_mvp, test_1) {
    test_case("../spectest/address");
    test_case("../spectest/align");
    test_case("../spectest/binary");
    test_case("../spectest/binary-leb128");
    test_case("../spectest/br_if");
    test_case("../spectest/br_table");
    test_case("../spectest/break-drop");
    test_case("../spectest/comments");
    test_case("../spectest/const");
    test_case("../spectest/custom");
    test_case("../spectest/data");
    test_case("../spectest/elem");
    test_case("../spectest/endianness");
    // todo // test_case("../spectest/exports");
    test_case("../spectest/f32");
    test_case("../spectest/f32_bitwise");
    test_case("../spectest/f32_cmp");
    test_case("../spectest/f64");
    test_case("../spectest/f64_bitwise");
    test_case("../spectest/f64_cmp");
    test_case("../spectest/float_exprs");
    test_case("../spectest/float_literals");
    test_case("../spectest/float_memory");
    test_case("../spectest/float_misc");
    test_case("../spectest/forward");
    test_case("../spectest/func_ptrs");
    test_case("../spectest/global");
    test_case("../spectest/globals");
    test_case("../spectest/imports");
    test_case("../spectest/inline-module");
    test_case("../spectest/int_exprs");
    test_case("../spectest/int_literals");
    test_case("../spectest/labels");
    test_case("../spectest/left-to-right");
    test_case("../spectest/linking");
    test_case("../spectest/load");
    test_case("../spectest/local_get");
    test_case("../spectest/local_set");
    test_case("../spectest/local_tee");

    // use too many time temp commented/
    // test_case("../spectest/memory");
    // test_case("../spectest/memory_grow");
    // test_case("../spectest/memory_redundancy");
    // test_case("../spectest/memory_size");
    // test_case("../spectest/memory_trap");

    test_case("../spectest/names");
    test_case("../spectest/nop");
    test_case("../spectest/return");
    test_case("../spectest/select");
    // use too many time temp commented/
    // test_case("../spectest/skip-stack-guard-page");
    test_case("../spectest/stack");
    test_case("../spectest/start");
    test_case("../spectest/store");
    test_case("../spectest/switch");
    // test_case("../spectest/table");
    // test_case("../spectest/token");
    test_case("../spectest/traps");
    // test_case("../spectest/type");
    // test_case("../spectest/typecheck");
    // test_case("../spectest/unreachable");
    // test_case("../spectest/unreached-invalid");
    // test_case("../spectest/unwind");
    // test_case("../spectest/utf8-custom-section-id");
    // test_case("../spectest/utf8-import-field");
    // test_case("../spectest/utf8-import-module");
    // test_case("../spectest/utf8-invalid-encoding");
}
