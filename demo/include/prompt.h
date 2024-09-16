//
// Created by lsfco on 2024/9/8.
//

#ifndef LEARN_CPP_PROMPT_H
#define LEARN_CPP_PROMPT_H
#include <iostream>
#include <cxxabi.h>

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

} // prompt
#endif //LEARN_CPP_PROMPT_H
