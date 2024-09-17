//
// Created by lsfco on 2024/9/7.
//
#include <cstdio>
#include <type_traits>
#include <string>
#include <vector>
#include "print.h"
#include "gtest_prompt.h"
/*
 * decltype 配合 decay_t 一起使用
 * std::is_same_v<std::decay_t<decltype(t)>, int>
 * 因为func(T const &t) 中的t是const int &，所以std::decay_t<decltype(t)> 是int
 */

template<class T>
auto func(T const &t) {
    if constexpr (std::is_same_v<std::decay_t<decltype(t)>, int>) {
        return t + 1;
    }  else if constexpr (std::is_same_v<std::decay_t<decltype(t)>, double>) {
        return t + 1.0;
    } else if constexpr (std::is_same_v<std::decay_t<decltype(t)>, std::string>) {
        return t + " world";
    }
    else {
        return t;
    }
}

TEST(FuncTest, DiffInput) {
    int i = 0;
    EXPECT_EQ(func(i), 1);
    double d = 0.0;
    EXPECT_EQ(func(d), 1.0);
    std::string s = "hello";
    EXPECT_EQ(func(s), "hello world");
}

/*
 * std::is_same_v<decltype(f()), void> -> std::is_void_v<decltype(f())> 或者 std::is_void_v<std::invoke_result_t<F>>
 * 或者 std::is_void_v<decltype(std::declval<F>()())>
 * 这里没有配合 decay_t 使用，因为f()返回的是void
 */
template <class F>
auto invoke(F f) {
    if constexpr (std::is_void_v<decltype(std::declval<F>()())>) {
        f();
    } else {
        auto result = f();
        return result;
    }
}

TEST(InvokeTest, ReturnTypeInt) {
    int result = invoke([]() -> int {
        return 42;
    });
    EXPECT_EQ(result, 42);
}

TEST(InvokeTest, ReturnTypeVoid) {
    bool called = false;
    invoke([&]() -> void {
        called = true;
    });
    EXPECT_TRUE(called);
}

#define REQUIRES(x) std::enable_if_t<(x), int> = 0
template <class F, REQUIRES(std::is_void_v<std::invoke_result_t<F>>)>
auto separate_invoke(F f){
    f();
}
template <class F, REQUIRES(!std::is_void_v<std::invoke_result_t<F>>)>
auto separate_invoke(F f){
    auto result = f();
    return result;
}

TEST(SeparateInvokeTest, ReturnTypeInt) {
    int result = separate_invoke([]() -> int {
        return 42;
    });
    EXPECT_EQ(result, 42);
}

TEST(SeparateInvokeTest, ReturnTypeVoid) {
    bool called = false;
    separate_invoke([&]() -> void {
        called = true;
    });
    EXPECT_TRUE(called);
}

/*
 * 判断是否具有特定成员函数
 */
struct mystudent {
    void dismantle() {
        print("student dismantle");
    }
    void rebel(int i) {
        print("student rebel ", i);
    }
};
struct myteacher {
    void rebel(int i) {
        print("teacher rebel ", i);
    }
};
struct myclass {
    void dismantle() {
        print("class dismantle");
    }
};
struct myvoid {};

template <class T, class = void>
struct has_dismantle {
    static constexpr bool value = false;
};
template <class T>
struct has_dismantle<T, std::void_t<decltype(std::declval<T>().dismantle())>> { // 如果T有dismantle成员函数
    static constexpr bool value = true;
};
template <class T, class = void>
struct has_rebel {
    static constexpr bool value = false;
};
template <class T>
struct has_rebel<T, std::void_t<decltype(std::declval<T>().rebel(std::declval<int>()))>> { // 如果T有rebel成员函数
    static constexpr bool value = true;
};
#if __cplusplus >= 202002L
template <class T>
void gench(T t) {
    if constexpr (requires { t.dismantle(); }) {
        t.dismantle();
    }
    else if constexpr (requires (int i) { t.rebel(i); }) {
        for (int i = 1; i <= 4; i++) {
            t.rebel(i);
        }
    }
    else {
        print("no any method supported!");
    }

}
#else
template <class T, REQUIRES(has_dismantle<T>::value)>
void gench(T t) {
    t.dismantle();
}
template <class T, REQUIRES(!has_dismantle<T>::value && has_rebel<T>::value)>
void gench(T t) {
    for (int i = 1; i <= 4; i++) {
        t.rebel(i);
    }
}
template <class T, REQUIRES(!has_dismantle<T>::value && !has_rebel<T>::value)>
void gench(T t) {
    print("no any method supported!");
}
#endif

TEST(JudgeMemberFunc, Gench) {
    mystudent s;
    myteacher t;
    myclass c;
    myvoid v;
    ASSERT_LOGS_STDOUT(gench(s), "student dismantle");
    ASSERT_LOGS_STDOUT(gench(t), "teacher rebel  1\nteacher rebel  2\nteacher rebel  3\nteacher rebel  4");
    ASSERT_LOGS_STDOUT(gench(c), "class dismantle");
    ASSERT_LOGS_STDOUT(gench(v), "no any method supported!");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}