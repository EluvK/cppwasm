# mkdir cbuild
cd cbuild
# rm -rf *
cmake ..
make cppwasm_mvp_test && ./test_mvp/cppwasm_mvp_test