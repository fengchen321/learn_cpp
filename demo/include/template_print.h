#pragma once

#include <iostream>

/*
 * 求和
 */
#if __cplusplus >= 201703L
template <class ...Ts>
auto sum(Ts ...ts) {
    return (0 + ... + ts);
}
#else
auto sum_impl() {
    return 0; 
}

template <class T0, class ...Ts>
auto sum_impl(T0 t0, Ts ...ts) {
    return t0 + sum_impl(ts...);
}

template <class ...Ts>
auto sum(Ts ...ts) {
    return sum_impl(ts...);
}
#endif

/*
 * 打印
 */
#if __cplusplus >= 201703L
#if 1
#include "print.h"
auto printfun() {printnl("{}\n");}
template <class T0, class ... Ts>
auto printfun(T0 t0, Ts ...ts) {
    printnl("{");
    printnl(t0);
    ((printnl(", "), printnl(ts)), ...);
    printnl("}");
    printnl("\n");
//    (print(ts), ...);
}
#else
// 终止条件
void printfun_impl() {}

// 递归情况：当有更多参数时
template <class T0, class ...Ts>
void printfun_impl(T0 t0, Ts ...ts) {
    std::cout << t0;
    if constexpr (sizeof...(ts) != 0) {
        std::cout << ", ";
    }
    printfun_impl(ts ...);
}

template <class ...Ts>
void printfun(Ts ...ts) {
    std::cout << "{";
    printfun_impl(ts ...);
    std::cout << "}\n";
}
#endif
#else
#define REQUIRES(x) std::enable_if_t<(x), int> = 0
void printfun_impl() {}
template <class T0, class ...Ts, REQUIRES(sizeof...(Ts) == 0)>
void printfun_impl(T0 t0) {
    std::cout << t0;
}

template <class T0, class ...Ts,  REQUIRES(sizeof...(Ts) != 0)>
void printfun_impl(T0 t0, Ts ...ts) {
    std::cout << t0 << ", ";
    printfun_impl(ts...);
}

template <class ...Ts>
void printfun(Ts ...ts) {
    std::cout << "{";
     printfun_impl(ts...);
    std::cout << "}\n";
}
#endif