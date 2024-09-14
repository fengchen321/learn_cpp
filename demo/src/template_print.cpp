//
// Created by lsfco on 2024/9/8.
//
#include <vector>
#include <iostream>
#include "prompt.h"

/*
 * 求和
 */
template <class ...Ts>
auto sum(Ts ...ts) {
    return (0 + ... + ts);
}
void test_sum() {
    std::cout << sum() << "\n";
    std::cout << sum(1) << "\n";
    std::cout << sum(1, 2) << "\n";
}
/*
 * 打印
 */
#if __cplusplus >= 201703L
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
#if 0
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
#endif

void test_print() {
    printfun();
    printfun(1);
    printfun(1, 2);
    printfun(1, 2, 3);
}

int main() {
    display_cpp_version();
    test_sum();
    test_print();
    return 0;
}