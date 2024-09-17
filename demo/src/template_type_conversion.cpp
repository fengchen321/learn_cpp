#include <array>
#include <type_traits>
#include <iostream>
#include <variant>
#include <vector>
#include "prompt.h"
#include "cppdemangle.h"
#include "gtest_prompt.h"
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

TEST(TypeConversion, type_check) {
    EXPECT_TRUE((is_same_any<int, int, float>::value));
    EXPECT_FALSE((is_same_any<double, int, float>::value));
    EXPECT_TRUE((is_convertible_any<int, float, double>::value));
    EXPECT_FALSE((is_convertible_any<std::string, int, float>::value));
}

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

TEST(TypeConversion, VecInt) {
    auto it = vec_int(1, 1u);
    using ElementType = decltype(it)::value_type;
    EXPECT_TRUE((std::is_same_v<ElementType, int>));
    EXPECT_FALSE((std::is_same_v<ElementType, unsigned int>));
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

TEST(TypeConversion, TupleSize) {
    using Tup = std::tuple<int, float, double>;
    constexpr auto i = std::tuple_size<Tup>::value;
    constexpr auto j = tuple_size<Tup>::value;
    ASSERT_EQ(i, j);
    ASSERT_EQ(i, 3);
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
TEST(TypeConversion, TupleApplyCommonType) {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

    using what = tuple_apply_common_type<Tup>::type;
    using what2 = tuple_apply_common_type<Var>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"double");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"double");
}
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

TEST(TypeConversion, TupleApply) {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

#if TUPLE_APPLY
    using what = tuple_apply<std::common_type, Tup>::type::type;
    using what2 = tuple_apply<std::common_type, Var>::type::type;
    using what3 = tuple_apply<std::variant, Tup>::type;
    using what4 = tuple_apply<std::tuple, Var>::type;
#else
    using what = tuple_apply<common_type_wrapper, Tup>::type;
    using what2 = tuple_apply<common_type_wrapper, Var>::type;
    using what3 = tuple_apply<variant_wrapper, Tup>::type;
    using what4 = tuple_apply<tuple_wrapper, Var>::type;
#endif
    
    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"double");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"double");
    EXPECT_STREQ(cppdemangle(typeid(what3).name()).c_str(),"std::variant<int, float, double>");
    EXPECT_STREQ(cppdemangle(typeid(what4).name()).c_str(),"std::tuple<int, float, double>");
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

TEST(TypeConversion, TupleMap) {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

    using what = tuple_map<vector_wrapper, Tup>::type;
    using what2 = tuple_map<vector_wrapper, Var>::type;
    using what3 = tuple_map<array_wrapper<3>, Tup>::type;

    EXPECT_NE(cppdemangle(typeid(what).name()).find("std::tuple<std::vector"), std::string::npos)
        << "Substring not found in the full string";
    EXPECT_NE(cppdemangle(typeid(what2).name()).find("std::variant<std::vector"), std::string::npos)
        << "Substring not found in the full string";
    EXPECT_NE(cppdemangle(typeid(what3).name()).find("std::tuple<std::array"), std::string::npos)
        << "Substring not found in the full string";
}

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

TEST(TypeConversion, TupleCat) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<std::vector<double>, std::array<int, 4>>;

    using what = tuple_cat<Tup, Tup2>::type;
    using what2 = tuple_push_front<char *, Tup>::type;

    EXPECT_NE(cppdemangle(typeid(what).name()).find("std::tuple<int, float, double, std::vector"), std::string::npos)
        << "Substring not found in the full string";
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"std::tuple<char*, int, float, double>");
}

/*
* 获取类型 tuple_get_first
*/

template <class Tup>
struct tuple_get_first { };

template <class ...Ts>
struct tuple_get_first<std::tuple<Ts...>> {
    using type = std::tuple_element_t<0, std::tuple<Ts...>>;
};

TEST(TypeConversion, TupleGetFirst) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<std::vector<double>, std::array<int, 4>>;

    using what = tuple_get_first<Tup>::type;
    using what2 = tuple_get_first<Tup2>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_NE(cppdemangle(typeid(what2).name()).find("std::vector<double"), std::string::npos)
        << "Substring not found in the full string";
}

// tuple_get_first -> tuple_front
template <class Tup>
struct tuple_front { 
    using type = void;
};

template <class T0, class ...Ts>
struct tuple_front<std::tuple<T0, Ts...>> {
    using type = T0;
};

TEST(TypeConversion, TupleFront) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_front<Tup>::type;
    using what2 = tuple_front<std::tuple<>>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"void");
}

template <class Tup>
struct tuple_2d { };

template <class T0, class T1, class ...Ts>
struct tuple_2d<std::tuple<T0, T1, Ts...>> {
    using type = T1;
};

TEST(TypeConversion, Tuple2D) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_2d<Tup>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"float");
}

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

TEST(TypeConversion, TupleElement) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_element<0, Tup>::type;
    using what2 = tuple_element<1, Tup>::type;
    using what3 = tuple_element<2, Tup>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"float");
    EXPECT_STREQ(cppdemangle(typeid(what3).name()).c_str(),"double");
}

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

TEST(TypeConversion, TupleIsAllIntegral) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<int, int, int>;
    using Tup3 = std::tuple<int, char, short>;

    constexpr bool all_integral1 = tuple_is_all_integral<Tup>::value;
    constexpr bool all_integral2 = tuple_is_all_integral<Tup2>::value;
    constexpr bool all_integral3 = tuple_is_all_integral<Tup3>::value;

    EXPECT_FALSE(all_integral1);
    EXPECT_TRUE(all_integral2);
    EXPECT_TRUE(all_integral3);
}
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

TEST(TypeConversion, TupleIsAll) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<int, int, int>;
    using Tup3 = std::tuple<int, char, short>;

    constexpr bool all_int1 = tuple_is_all<CheckInt, Tup>::value;
    constexpr bool all_int2 = tuple_is_all<CheckInt, Tup2>::value;
    constexpr bool all_int3 = tuple_is_all<CheckInt, Tup3>::value;

    using Tup4 = std::tuple<float, float, float>;

    constexpr bool all_float1 = tuple_is_all<CheckFloat, Tup>::value;
    constexpr bool all_float2 = tuple_is_all<CheckFloat, Tup4>::value;

    EXPECT_FALSE(all_int1);
    EXPECT_TRUE(all_int2);
    EXPECT_TRUE(all_int3);
    EXPECT_FALSE(all_float1);
    EXPECT_TRUE(all_float2);
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}