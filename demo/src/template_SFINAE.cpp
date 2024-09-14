//
// Created by lsfco on 2024/9/7.
//
#include <cstdio>
#include <type_traits>
#include <string>
#include <vector>
#include "print.h"
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
void test_func() {
    int i = 0;
    double d = 0.0;
    std::string s = "hello";
    print(func(i));
    print(func(d));
    print(func(s));
}
class Log {
public:
    Log(std::string str) : _str(std::move(str)) {
        printf("entered %s\n", _str.c_str());
    }
    ~Log() {
        printf("exited %s\n", _str.c_str());
    }
private:
    std::string _str;
};
/*
 * std::is_same_v<decltype(f()), void> -> std::is_void_v<decltype(f())> 或者 std::is_void_v<std::invoke_result_t<F>>
 * 或者 std::is_void_v<decltype(std::declval<F>()())>
 * 这里没有配合 decay_t 使用，因为f()返回的是void
 */
template <class F>
auto invoke(F f) {
    Log log("test");
    if constexpr (std::is_void_v<decltype(std::declval<F>()())>) {
        f();
    } else {
        auto result = f();
        return result;
    }
}

void test_invoke() {
    std::vector<int> i{42};
    {
        invoke([]() -> int {
            return 1;
        });
    }
    {
        invoke([]() -> void {
            print("hello world");
        });
    }
}
#define REQUIRES(x) std::enable_if_t<(x), int> = 0
template <class F, REQUIRES(std::is_void_v<std::invoke_result_t<F>>)>
auto separate_invoke(F f){
    Log log("void");
    f();
}
template <class F, REQUIRES(!std::is_void_v<std::invoke_result_t<F>>)>
auto separate_invoke(F f){
    Log log("non-void");
    auto result = f();
    return result;
}

void test_separate_invoke() {
    std::vector<int> i{42};
    separate_invoke([]()->int {
        return 1;
    });
    separate_invoke([]()->void {
        print("hello world");
    });
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
int main() {
//    test_func();
//    test_invoke();
//    test_separate_invoke();
    mystudent s;
    myteacher t;
    myclass c;
    myvoid v;
    gench(c);
    gench(s);
    gench(t);
    gench(v);
    return 0;
}