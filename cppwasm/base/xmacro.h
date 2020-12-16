
#pragma once
#include <iostream>

// debug on
#define DEBUG

#define xdbg(...)                                                                                                                                                                  \
    fprintf(stderr, __VA_ARGS__);                                                                                                                                                  \
    printf("\n");
// #define xdbg(...)
#define xerror(...)                                                                                                                                                                \
    {                                                                                                                                                                              \
        fprintf(stderr, "Error(%s:%d): ", __FILE__, __LINE__);                                                                                                                     \
        fprintf(stderr, __VA_ARGS__);                                                                                                                                              \
        printf("\n");                                                                                                                                                              \
        throw(__VA_ARGS__);                                                                                                                                                        \
    }
// #define xerror(...) throw(__VA_ARGS__);
// exit(1);
#define xwarn(...) fprintf(stderr, __VA_ARGS__);

#define ASSERT(exp, ...)                                                                                                                                                           \
    {                                                                                                                                                                              \
        if (!(exp)) {                                                                                                                                                              \
            fprintf(stderr, "Assert Failed (%s:%d): ", __FILE__, __LINE__);                                                                                                        \
            fprintf(stderr, __VA_ARGS__);                                                                                                                                          \
            exit(1);                                                                                                                                                               \
        }                                                                                                                                                                          \
    }

