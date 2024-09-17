#include <array>
#include <type_traits>
#include <iostream>
#include <variant>
#include <vector>
#include "prompt.h"
#include "print.h"
#include "gtest_prompt.h"

/*
* 编译期 for循环
*/
template <size_t Beg, size_t End, class Lambda>
void static_for(Lambda lambda) {
    if constexpr (Beg < End) {
        lambda.template operator()<Beg>();
        static_for<Beg + 1, End>(lambda);
    }
}
std::vector<size_t> captured_indices;

TEST(StaticForTest, CorrectIndicesProcessed) {
    captured_indices.clear();

    static_for<0, 4>([&]<size_t I>() {
        captured_indices.push_back(I);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}

template <int X>
struct int_constant {
    static constexpr int value = X;
};

template <size_t Beg, size_t End, class Lambda>
void static_for_1(Lambda lambda) {
    if constexpr (Beg < End) {
        std::integral_constant<size_t, Beg> i; 
        // int_constant<Beg> i; // 或者自己实现is_constant
        if constexpr (std::is_void_v<std::invoke_result_t<Lambda, decltype(i)>>) {
            lambda(i);
        }
        else {
            if (lambda(i)) {
                return; // 提前打断
            }
        }
        static_for_1<Beg + 1, End>(lambda);
    }
}

TEST(StaticForTest, IntConstant) {
    captured_indices.clear();

    static_for_1<0, 4>([&](auto i){
        captured_indices.push_back(i.value);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}

TEST(StaticForTest, TupleSample) {
    std::tuple<int, float, double, char> tup{0, 1.0, 2.0, 'c'};
    static_for_1<0, 4>([&](auto i){
        if constexpr (i == 0) {
            EXPECT_EQ(std::get<i>(tup), 0);
            EXPECT_EQ(std::get<i>(tup), 0);
            EXPECT_EQ(std::get<i()>(tup), 0);// 编译期隐式类型转换
        } else if constexpr (i == 1) {
            EXPECT_EQ(std::get<i>(tup), 1.0f);
        } else if constexpr (i == 2) {
            EXPECT_EQ(std::get<i>(tup), 2.0);
        } else if constexpr (i == 3) {
            EXPECT_EQ(std::get<i>(tup), 'c');
        }
    });
}

TEST(StaticForTest, VariantSample) {
    std::variant<int, float, double, char> var = {'c'};
    std::visit([&](auto const &v){
        EXPECT_EQ(v, 'c');
    }, var); // 推荐

    static_for_1<0, 4>([&](auto i){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'c');
            return true;
        }
        return false;
    });
}

template <size_t ...Is, class Lambda>
void _static_for_impl(Lambda lambda, std::index_sequence<Is...>){
    (lambda(std::integral_constant<size_t, Is>{}), ...);
}

template <size_t N, class Lambda>
void static_for_2(Lambda lambda) {
    _static_for_impl(lambda, std::make_index_sequence<N>{});
}

TEST(StaticForTest, IndexSequence) {
    captured_indices.clear();
    static_for_2<4>([&](auto i){
        captured_indices.push_back(i.value);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}

// 编译器for循环应用 tup_map; 枚举反射也是
#if 1
template <class Lambda, class ...Ts>
auto tup_map(Lambda lambda, std::tuple<Ts...> tup) {
    std::tuple<std::invoke_result_t<Lambda, Ts>...> res;
    static_for_2<sizeof...(Ts)>([&] (auto i) {
        std::get<i.value>(res) = lambda(std::get<i.value>(tup));
    });
    return res;
}
#else
template <class Lambda, class Tup, size_t ...Is>
auto _tup_map_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return std::tuple<std::invoke_result_t<Lambda, std::tuple_element_t<Is, Tup>>...>{
        lambda(std::get<Is>(tup))...
    };
}
template <class Lambda, class Tup>
auto tup_map(Lambda lambda, Tup tup) {
    return _tup_map_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
    /* {lambda(std::get<Is>(tup))...} */ // ...在括号外面
    /* {lambda(get<0>(tup)), lambda(get<1>(tup)), ...} */
    /* lambda(std::get<Is>(tup)...) */ // ...在括号里面
    /* lambda(get<0>(tup), get<1>(tup), ...) */
}
#endif

TEST(StaticForTest, TupMap) {
    std::tuple<int, float> tup = {42, 3.14f};
    auto res = tup_map([](auto v){
        return v + 1;
    }, tup);

    EXPECT_EQ(std::get<0>(res), 43);
    EXPECT_NEAR(std::get<1>(res), 4.14f, 1e-5f);
}

template <class Lambda, class Tup, size_t ...Is>
auto _tup_apply_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return lambda(std::get<Is>(tup)...);
}

template <class Lambda, class Tup>
auto tup_apply(Lambda lambda, Tup tup) {
    return _tup_apply_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
}

TEST(StaticForTest, TupApply) {
    std::tuple<int, float> tup = {42, 3.14f};
    auto res_tupapply = tup_apply([] (auto ...xs) { 
        return (xs + ...); 
    }, tup);
    EXPECT_NEAR(res_tupapply, 45.14f, 1e-5f);

    auto res_tupapply2 = tup_apply([] (int i, float f) { 
        return (i + f); 
    }, tup);
    EXPECT_NEAR(res_tupapply2, 45.14f, 1e-5f);
}

/*
* 通过Breaker对象控制循环的提前终止
*/
template <size_t Beg, size_t End, class Lambda>
void static_for_3(Lambda lambda) {
    if constexpr (Beg < End) {
        std::integral_constant<size_t, Beg> i;
        struct Breaker {
            bool *m_broken;
            constexpr void static_break() const {
                *m_broken = true;
            }
        };
        if constexpr (std::is_invocable_v<Lambda, decltype(i), Breaker>) {
            bool broken = false;
            lambda(i, Breaker{&broken});
            if (broken) {
                return;
            }
        }
        else {
            lambda(i);
        }
        static_for_3<Beg + 1, End>(lambda);
    }
}
TEST(StaticForTest, StaticforBreaker) {
    std::variant<int, float, double, char> var = {'b'};
    static_for_3<0, 4>([&](auto i, auto ctrl){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'b');
            ctrl.static_break();
        }
    });
}

TEST(StaticForTest, StaticforBreaker1) {
    std::variant<int, float, double, char> var = {'b'};
    static_for_3<0, 4>([&](auto i){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'b');
        }
    });
}

TEST(StaticForTest, StaticforBreaker2) {
    std::vector<size_t> vec;
    // 打印到 2 时提前退出
    static_for_3<0, 4>([&](auto i, auto ctrl) {
        vec.push_back(i.value);
        if (i.value == 2) {
            ctrl.static_break();
        }
    });
    EXPECT_EQ(vec.size(), 3); 
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}