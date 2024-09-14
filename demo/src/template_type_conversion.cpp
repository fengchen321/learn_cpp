#include <array>
#include <type_traits>
#include <iostream>
#include <variant>
#include <vector>
#include "prompt.h"
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
    return {t0, ts...};
}
#else
template<class ...Ts>
std::enable_if_t<(true && ... && std::is_convertible_v<Ts, int>), void>
vec_int(Ts ...ts) { }

template <class T0, class ...Ts>
std::enable_if_t<(true && ... && std::is_convertible_v<Ts, T0>), std::array<T0, sizeof...(Ts) + 1>>
vec_int(T0 t0, Ts ...ts) {
    return {t0, ts...};
}
#endif

void test_vec_int() {
    vec_int(1, 1u);
    std::cout << "is_same_any<int, int, float>::value: " << is_same_any<int, int, float>::value << "\n";
}

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

void test_tuple_size() {
    using Tup = std::tuple<int, float, double>;
    constexpr auto i = std::tuple_size<Tup>::value;
    constexpr auto j = tuple_size<Tup>::value;
    std::cout << i << "," << j << "\n";
}

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

void test_tuple_apply() {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;
//    using what = tuple_apply_common_type<Tup>::type;
//    using what2 = tuple_apply_common_type<Var>::type;
#if TUPLE_APPLY
    using what = tuple_apply<std::common_type, Tup>::type::type;
    using what2 = tuple_apply<std::common_type, Var>::type::type;
    using what3 = tuple_apply<std::tuple, Var>::type;
#else
    using what = tuple_apply<common_type_wrapper, Var>::type;
    using what2 = tuple_apply<variant_wrapper, Var>::type;
    using what3 = tuple_apply<tuple_wrapper, Var>::type;
#endif
    printf("what: %s\n", demangle(typeid(what).name()).c_str());
    printf("what2: %s\n", demangle(typeid(what2).name()).c_str());
    printf("what3: %s\n", demangle(typeid(what3).name()).c_str());

}

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

void test_tuple_map() {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

    using what = tuple_map<vector_wrapper, Tup>::type;
    using what2 = tuple_map<vector_wrapper, Var>::type;
    using what3 = tuple_map<array_wrapper<3>, Tup>::type;

    printf("what: %s\n", demangle(typeid(what).name()).c_str());
    printf("what2: %s\n", demangle(typeid(what2).name()).c_str());
    printf("what3: %s\n", demangle(typeid(what3).name()).c_str());
}

int main() {
    display_cpp_version();
    test_vec_int();
    test_tuple_size();
    test_tuple_apply();
    test_tuple_map();
    return 0;
}