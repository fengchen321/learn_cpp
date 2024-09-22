#pragma once

#include <type_traits>
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

/*
 * 判断是否具有特定成员函数
 */
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
        printf("%s\n", "no any method supported!");
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
    printf("%s\n", "no any method supported!");
}
#endif