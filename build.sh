# mkdir cbuild
cd cbuild
rm -rf *
cmake ..
make wasi_test && ./test/wasi_test