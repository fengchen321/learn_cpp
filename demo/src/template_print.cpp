//
// Created by lsfco on 2024/9/8.
//
#include <vector>
#include <iostream>
#include "prompt.h"
#include "gtest_prompt.h"

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

TEST(PrintTest, Sum) {
    EXPECT_EQ(sum(), 0);
    EXPECT_EQ(sum(1), 1);
    EXPECT_EQ(sum(1, 2), 3);
}
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

TEST(PrintTest, Print) {
    ASSERT_LOGS_STDOUT(printfun(), "{}\n");
    ASSERT_LOGS_STDOUT(printfun(1), "{1}\n");
    ASSERT_LOGS_STDOUT(printfun(1,2), "{1, 2}\n");
    ASSERT_LOGS_STDOUT(printfun(1,2,3), "{1, 2, 3}\n");
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}