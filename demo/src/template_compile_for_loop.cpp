#include <array>
#include <type_traits>
#include <iostream>
#include <variant>
#include <vector>
#include "prompt.h"
#include "print.h"

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

void test_static_for() {
    static_for<0, 4>([&]<size_t I>{
        print(I);
    });
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

void test_static_for_1() {
    static_for_1<0, 4>([&](auto i){
        print(i.value);
    });
    printf("--------------------\n");
    std::tuple<int, float, double, char> tup{0, 1.0, 2.0, 'c'};
    static_for_1<0, 4>([&](auto i){
        print(std::get<i.value>(tup));
        print(std::get<i>(tup));
        print(std::get<i()>(tup)); // 编译期隐式类型转换
    });
    printf("--------------------\n");
    std::variant<int, float, double, char> var = {'c'};
    std::visit([&](auto const &v){
        print(v);
    }, var); // 推荐
    static_for_1<0, 4>([&](auto i){
        if (i.value == var.index()) {
            print(std::get<i.value>(var));
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

template <class Lambda, class Tup, size_t ...Is>
auto _tup_apply_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return lambda(std::get<Is>(tup)...);
}

template <class Lambda, class Tup>
auto tup_apply(Lambda lambda, Tup tup) {
    return _tup_apply_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
}

void test_static_for_2() {
    static_for_2<4>([&](auto i){
        print(i.value);
    });
    printf("--------------------\n");
    std::tuple<int, float, double, char> tup{0, 1.0, 2.0, 'c'};
    static_for_2<4>([&](auto i){
        print(std::get<i.value>(tup));
        print(std::get<i>(tup));
        print(std::get<i()>(tup)); // 编译期隐式类型转换
    });
    printf("--------------------\n");
    std::variant<int, float, double, char> var = {'c'};
    std::visit([&](auto const &v){
        print(v);
    }, var); // 推荐
    static_for_2<4>([&](auto i){
        if (i.value == var.index()) {
            print(std::get<i.value>(var));
            return true;
        }
        return false;
    });
    printf("--------------------\n");
    std::tuple<int, float> tup1 = {42, 3.14f};
    auto res_tupmap = tup_map([](auto x){
        return x + 1;
    }, tup1);
    print(res_tupmap);

    auto res_tupapply = tup_apply([] (auto ...xs) { 
        return (xs + ...); 
    }, tup1);
    print(res_tupapply);

    auto res_tupapply2 = tup_apply([] (int i, float f) { 
        return (i + f); 
    }, tup1);
    print(res_tupapply2);

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

void test_static_for_3() {
    std::variant<int, float, double, char> var = {'b'};
    static_for_3<0, 4>([&](auto i, auto ctrl){
        if (i.value == var.index()) {
            print(std::get<i.value>(var));
            ctrl.static_break();
        }
    });

    static_for_3<0, 4>([&](auto i){
        if (i.value == var.index()) {
            print(std::get<i.value>(var));
        }
    });

    static_for_3<0, 4>([&](auto i) {
        print(i.value);
    });

    // 打印到 2 时提前退出
    static_for_3<0, 4>([&](auto i, auto ctrl) {
        print(i.value);
        if (i.value == 2) {
            ctrl.static_break();
        }
    });
}
int main() {
    prompt::display_cpp_version();
    // test_static_for();
    // test_static_for_1();
    test_static_for_2();
    // test_static_for_3();
    return 0;
}