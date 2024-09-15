//
// Created by lsfco on 2024/9/8.
//

#ifndef LEARN_CPP_PROMPT_H
#define LEARN_CPP_PROMPT_H
#include <iostream>
#include <cxxabi.h>
#include <memory>

namespace prompt {

inline void display_cpp_version() {
    printf("Current version: ");
#if __cplusplus >= 202002L
    printf("C++ 20 or later\n");
#elif __cplusplus >= 201703L
    printf("C++17\n");
#elif __cplusplus >= 201402L
    printf("C++14\n");
#elif __cplusplus >= 201103L
    printf("C++11\n");
#else
    printf("Pre-C++11\n");
#endif
}

struct AutoLog{ 
    AutoLog(std::string name) {
        mFuncName = name;
        printf("**************** %s **************** \n", mFuncName.c_str());
    }
    ~AutoLog() {
        printf("**************** %s **************** \n", mFuncName.c_str());
    }
    std::string mFuncName;
};

inline std::string demangle(const char* name) {
    int status = 0;
    std::unique_ptr<char[], decltype(&std::free)> res {
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
    };
    return (status == 0) ? res.get() : name;
}

} // prompt
#endif //LEARN_CPP_PROMPT_H
