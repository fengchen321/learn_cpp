#pragma once

#include <type_traits>
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

template <size_t ...Is, class Lambda>
void _static_for_impl(Lambda lambda, std::index_sequence<Is...>){
    (lambda(std::integral_constant<size_t, Is>{}), ...);
}

template <size_t N, class Lambda>
void static_for_2(Lambda lambda) {
    _static_for_impl(lambda, std::make_index_sequence<N>{});
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

template <class Lambda, class Tup, size_t ...Is>
auto _tup_apply_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return lambda(std::get<Is>(tup)...);
}

template <class Lambda, class Tup>
auto tup_apply(Lambda lambda, Tup tup) {
    return _tup_apply_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
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