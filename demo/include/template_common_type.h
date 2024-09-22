#pragma once

#include <array>
#include <type_traits>
#include "cppdemangle.h"
/*
 * array 引出 common_type
 */
template <class ...Ts>
auto array_func(Ts ...ts) {
    using T = std::common_type_t<Ts...>; // Ts...中的公共类型, int long double T为 double
    return std::array<T, sizeof...(ts)>{static_cast<T>(ts)...};
}

template <class T1, class T2>
struct common_type_two {
    using type = decltype(0 ? std::declval<T1>() : std::declval<T2>());
    // 不用decltype(T1() + T2()); 是因为有些继承类也属于common_type
    // 不用decltype(0 ? T1() : T2());是因为有的没有默认构造函数
};
template <class ...Ts>
struct common_type {};

template <class T0>
struct common_type<T0> {
    using type = T0;
};
template <class T0, class T1, class ...Ts>
struct common_type<T0, T1, Ts...> { // 递归
    using type = typename common_type_two<T0, typename common_type<T1, Ts...>::type>::type;
};

struct Animal{};
struct Cat : Animal {
    Cat(Cat &&) = delete; // 删除移动构造，拷贝，默认都会被删除
};
struct Dog : Animal {};

#if __cplusplus >= 202002L
// 函数版获取common_type
template <class T0, class ...Ts>
constexpr auto get_common_type(std::type_identity<T0>, std::type_identity<Ts> ...ts) {
    if constexpr (sizeof...(Ts) == 0) {
        return std::type_identity<T0>{};
    } else {
        return std::type_identity<decltype(0 ? std::declval<T0>() :
                std::declval<typename decltype(get_common_type(ts...))::type>())>{};
    }
}

#else
template <class T>
struct dummy {
    static constexpr T declval() {
        return std::declval<T>();
    }
};

template <class T0, class ...Ts>
constexpr auto get_common_type(dummy<T0> t0, dummy<Ts> ...ts) {
    if constexpr (sizeof...(Ts) == 0) {
        return t0;
    } else {
        return dummy<decltype(0 ? t0.declval() : get_common_type(ts...).declval())>{};
    }
}
#endif

