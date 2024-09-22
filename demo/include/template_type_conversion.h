#pragma once
#include <type_traits>
#include <iostream>
#include <array>
#include <variant>
#include <vector>
#include "cppdemangle.h"

/*
 * 类型检查和参数处理
 */
template <class T, class ...Ts>
struct is_same_any {
    static constexpr bool value = (false || ... || std::is_same_v<T, Ts>);
};

template <class T, class ...Ts>
struct is_convertible_any {
    static constexpr bool value = (false || ... || std::is_convertible_v<T, Ts>);
};

#if __cplusplus >= 202002L
template<class ...Ts>
    requires((true && ... && std::is_convertible_v<Ts, int>))  // is_same_v 太严格，换成is_convertible_v
void vec_int(Ts ...ts) { }

template <class T0, class ...Ts>
    requires((true && ... && std::is_convertible_v<Ts, T0>))
std::array<T0, sizeof...(Ts) + 1> vec_int(T0 t0, Ts ...ts) {
    return {static_cast<T0>(t0), (static_cast<T0>(ts))...};
}
#else
template<class ...Ts>
std::enable_if_t<(true && ... && std::is_convertible_v<Ts, int>), void>
vec_int(Ts ...ts) { }

template <class T0, class ...Ts>
std::enable_if_t<(true && ... && std::is_convertible_v<Ts, T0>), std::array<T0, sizeof...(Ts) + 1>>
vec_int(T0 t0, Ts ...ts) {
    return {static_cast<T0>(t0), (static_cast<T0>(ts))...};
}
#endif

/*
 * tuple size 求tuple的size
 */
template <class Tup>
struct tuple_size {};

template<>
struct tuple_size<std::tuple<>> {
    static constexpr std::size_t value = 0;
};

template <class T0, class ...Ts>
struct tuple_size<std::tuple<T0, Ts...>> {
    static constexpr std::size_t value = 1 + tuple_size<std::tuple<Ts...>>::value;
};

/*
 * tuple apply common type
 */
template <class Tup>
struct tuple_apply_common_type {};

#if 1
template <class ...Ts>
struct tuple_apply_common_type<std::tuple<Ts...>> : std::common_type<Ts...> {
};
template <class ...Ts>
struct tuple_apply_common_type<std::variant<Ts...>> : std::common_type<Ts...> {
};
#else
template <class ...Ts>
struct tuple_apply_common_type<std::tuple<Ts...>> {
    using type = std::common_type_t<Ts...>;
};

template <class ...Ts>
struct tuple_apply_common_type<std::variant<Ts...>> {
    using type = std::common_type_t<Ts...>;
};
#endif

/*
 * 通用的tuple apply 进行类型转换
 */
#define TUPLE_APPLY 0
#if TUPLE_APPLY
template <template <class ...Ts> class Tmpl, class Tup>
struct tuple_apply {};

template <template <class ...Ts> class Tmpl, class ...Ts>
struct tuple_apply<Tmpl, std::tuple<Ts...>> {
    using type = Tmpl<Ts...>;
};

template <template <class ...Ts> class Tmpl, class ...Ts>
struct tuple_apply<Tmpl, std::variant<Ts...>> {
    using type = Tmpl<Ts...>;
};
#else
struct common_type_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = std::common_type_t<Ts...>;
    };
};
struct tuple_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = std::tuple<Ts...>;
    };
};
struct variant_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = std::variant<Ts...>;
    };
};
template <class Tmpl, class Tup>
struct tuple_apply {};

template <class Tmpl, class ...Ts>
struct tuple_apply<Tmpl, std::tuple<Ts...>> {
    using type = typename Tmpl::template rebind<Ts...>::type;
};

template <class Tmpl, class ...Ts>
struct tuple_apply<Tmpl, std::variant<Ts...>> {
    using type = typename Tmpl::template rebind<Ts...>::type;
};
#endif

/*
 * 使用 tuple_map 和类型包装器（array_wrapper 和 vector_wrapper）来转换元组和变体中的类型。
 */
template <size_t N>
struct array_wrapper {
    template <class T>
    struct rebind {
        using type = std::array<T, N>;
    };
};
struct vector_wrapper {
    template <class T>
    struct rebind {
        using type = std::vector<T>;
    };
};
template <class Tmpl, class Tup>
struct tuple_map {};

template <class Tmpl, class ...Ts>
struct tuple_map<Tmpl, std::tuple<Ts...>> {
    using type = std::tuple<typename Tmpl::template rebind<Ts>::type...>;
};
template <class Tmpl, class ...Ts>
struct tuple_map<Tmpl, std::variant<Ts...>> {
    using type = std::variant<typename Tmpl::template rebind<Ts>::type...>;
};

/*
 * 使用 tuple_cat 进行拼接 
 * tuple_push_front 前向拼接
 */
template <class Tup1, class Tup2>
struct tuple_cat { };

template <class ...T1s, class ...T2s>
struct tuple_cat<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
};

template <class T1, class Tup2>
struct tuple_push_front { };

template <class T1, class ...T2s>
struct tuple_push_front<T1, std::tuple<T2s...>> {
    using type = std::tuple<T1, T2s...>;
};

/*
* 获取类型 tuple_get_first
*/

template <class Tup>
struct tuple_get_first { };

template <class ...Ts>
struct tuple_get_first<std::tuple<Ts...>> {
    using type = std::tuple_element_t<0, std::tuple<Ts...>>;
};

// tuple_get_first -> tuple_front
template <class Tup>
struct tuple_front { 
    using type = void;
};

template <class T0, class ...Ts>
struct tuple_front<std::tuple<T0, Ts...>> {
    using type = T0;
};

template <class Tup>
struct tuple_2d { };

template <class T0, class T1, class ...Ts>
struct tuple_2d<std::tuple<T0, T1, Ts...>> {
    using type = T1;
};

// 实现之前的 tuple_element_t  递归
template <size_t I, class Tup>
struct tuple_element { };

template <class T0, class ...Ts>
struct tuple_element<0, std::tuple<T0, Ts...>> {
    using type = T0;
};

template <size_t N, class T0, class ...Ts>
struct tuple_element<N, std::tuple<T0, Ts...>> {
    using type = typename tuple_element<N - 1, std::tuple<Ts...>>::type;
};

/*
* 判断 tupe 类型是否全为int
*/
template <class Tup>
struct tuple_is_all_integral { };
#if 0
template <>
struct tuple_is_all_integral<std::tuple<>> {
    static constexpr bool value = true;
};

template <class T0, class ...Ts>
struct tuple_is_all_integral<std::tuple<T0, Ts...>> {
    static constexpr bool value = std::is_integral_v<T0>
                                && tuple_is_all_integral<std::tuple<Ts...>>::value;
};
#else
template <class ...Ts>
struct tuple_is_all_integral<std::tuple<Ts...>> {
    static constexpr bool value = (true && ... && std::is_integral_v<Ts>);
};
#endif

template <class Tmpl, class Tup>
struct tuple_is_all {};

template <class Tmpl, class ...Ts>
struct tuple_is_all<Tmpl, std::tuple<Ts...>> {
    static constexpr bool value = (true && ... && Tmpl::template rebind<Ts>::value);
};

struct CheckInt {
    template <class T>
    struct rebind {
        static constexpr bool value = std::is_integral_v<T>;
    };
};

struct CheckFloat {
    template <class T>
    struct rebind {
        static constexpr bool value = std::is_same_v<T, float>;
    };
};