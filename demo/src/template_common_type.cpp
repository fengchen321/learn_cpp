//
// Created by lsfco on 2024/9/8.
//
#include <array>
#include <type_traits>
#include <iostream>
#include "prompt.h"
#include "cppdemangle.h"
#include "gtest_prompt.h"
/*
 * array 引出 common_type
 */
template <class ...Ts>
auto array_func(Ts ...ts) {
    using T = std::common_type_t<Ts...>; // Ts...中的公共类型, int long double T为 double
    return std::array<T, sizeof...(ts)>{static_cast<T>(ts)...};
}

TEST(CommonType, ArrayFunc) {
    auto a = array_func(1, 2.0, 3.0f);
    using ElementType = decltype(a)::value_type;
    EXPECT_FALSE((std::is_same_v<ElementType, int>));
    EXPECT_FALSE((std::is_same_v<ElementType, float>));
    EXPECT_TRUE((std::is_same_v<ElementType, double>));

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

TEST(CommonType, CommonTypeRecursive) {
    using what = typename common_type_two<int, long>::type;
    EXPECT_FALSE((std::is_same_v<what, int>));
    EXPECT_TRUE((std::is_same_v<what, long>));

    using what2 = typename common_type<int, long, double>::type;
    EXPECT_FALSE((std::is_same_v<what2, int>));
    EXPECT_FALSE((std::is_same_v<what2, long>));
    EXPECT_TRUE((std::is_same_v<what2, double>));
}

struct Animal{};
struct Cat : Animal {
    Cat(Cat &&) = delete; // 删除移动构造，拷贝，默认都会被删除
};
struct Dog : Animal {};

TEST(CommonType, CommonTypeRecursiveStruct) {
    using what = typename common_type_two<Cat, Animal>::type;
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what).name()), "Animal");

    using what2 = typename common_type<Cat, Dog, Animal>::type;
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what2).name()), "Animal");
}

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

TEST(CommonType, CommonTypeFunc) {
#if __cplusplus >= 202002L
    using what_Ts_func = decltype(get_common_type(std::type_identity<Cat>{}, std::type_identity<Animal>{}));
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what_Ts_func).name()), "std::type_identity", "Animal");

    using what_Ts_func2 = decltype(get_common_type(std::type_identity<Cat>{}, std::type_identity<Dog>{}, std::type_identity<Animal>{}));
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what_Ts_func2).name()), "std::type_identity", "Animal");
#else
    using what_Ts_func = decltype(get_common_type(dummy<Cat>{}, dummy<Animal>{}));
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what_Ts_func).name()), "dummy", "Animal");

    using what_Ts_func2 = decltype(get_common_type(dummy<Cat>{}, dummy<Dog>{}, dummy<Animal>{}));
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what_Ts_func2).name()), "dummy", "Animal");

#endif
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
