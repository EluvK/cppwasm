mkdir cbuild
cd cbuild
rm -rf *
cmake ..
make wasi_test_on_work && ./test_filed/wasi_test_on_work