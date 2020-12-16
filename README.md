## CPPWASM
A wasm interpreter written in c++

## LOG
- 2020.10.27
    ✔️ init project
- 2020.11.26
    ✔️ pass mvp test
- 2020.12.16
    - [ ] upload to github
- future:
    - [ ] figure out mvp test case like `assert_malformed` & `assert_uninstantiable`
    - [ ] llvm-wasm-libc Project, supporting using part of libc while compiling C++ into wasm 

## TEST
#### NEED:
- cmake 3.8 or higher
- googletest
- GCC 4.8.5 or higher

#### mvp test:
try `bash build_mvp_test.sh`

## Thanks 
- [pywasm](https://github.com/mohanson/pywasm)
