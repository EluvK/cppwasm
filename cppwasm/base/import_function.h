#pragma once;
#include <functional>
#include <iostream>
#include <map>

static int fib(int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
static void print(int n) {
    std::printf("-----import print----\n");
    std::printf("|      print %d       |\n", n);
    std::printf("---------------------\n");
}

static std::function<int(int n)> f_fib = fib;
static std::function<void(int n)> f_print = print;
