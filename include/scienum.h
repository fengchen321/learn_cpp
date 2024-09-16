#ifndef LEARN_CPP_SCIENUM_H
#define LEARN_CPP_SCIENUM_H

#include <string>

namespace scienum {

namespace details {

template <class T, T N>
const char *get_enum_name_static() {
#if defined(_MSC_VER)
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}
#if __cplusplus <= 201402L
template <bool Cond>
struct my_enable_if {};

template <>
struct my_enable_if<true> {
    typedef void type;
};

template <size_t Beg, size_t End, class F>
typename my_enable_if<Beg == End>::type static_for(F const &func) {
}

template <size_t Beg, size_t End, class F>
typename my_enable_if<Beg != End>::type static_for(F const &func) {
    func.template call<Beg>();
    static_for<Beg + 1, End>(func);
}


template <class T>
struct get_enum_name_functor {
    size_t n;
    std::string &s;

    get_enum_name_functor(size_t n, std::string &s) : n(n), s(s) {}

    template <size_t I>
    void call() const {
        if (n == I) s = details::get_enum_name_static<T, (T)I>();
    }
};
#else
template <size_t Beg, size_t End, class Lambda>
void static_for(Lambda lambda) {
    if constexpr (Beg < End) {
        lambda(std::integral_constant<size_t, Beg>{});
        static_for<Beg + 1, End>(lambda);
    }
}

template <class T>
struct get_enum_name_functor {
    size_t n;
    std::string &s;

    get_enum_name_functor(size_t n, std::string &s) : n(n), s(s) {}

    template <size_t I>
    void operator()(std::integral_constant<size_t, I>) const {
        if (n == I) {
            s = details::get_enum_name_static<T, static_cast<T>(I)>();
        }
    }
};
#endif

}

template <class T, T Beg, T End>
std::string get_enum_name(T n) {
    std::string s;
    details::static_for<static_cast<size_t>(Beg), static_cast<size_t>(End) + 1>(
        details::get_enum_name_functor<T>(static_cast<size_t>(n), s)
    );
    if (s.empty())
        return "";

#if defined(_MSC_VER)
    size_t pos = s.find(',');
    pos += 1;
    size_t pos2 = s.find('>', pos);
#else
    size_t pos = s.find("N = ");
    pos += 4;
    size_t pos2 = s.find_first_of(";]", pos);
#endif
    s = s.substr(pos, pos2 - pos);
    size_t pos3 = s.find("::");
    if (pos3 != s.npos)
        s = s.substr(pos3 + 2);
    return s;
}

template <class T>
std::string get_enum_name(T n) {
    return get_enum_name<T, static_cast<T>(0), static_cast<T>(256)>(n);
}

template <class T, T Beg, T End>
T enum_from_name(const std::string &s) {
    for (size_t i = static_cast<size_t>(Beg); i < static_cast<size_t>(End); ++i) {
        if (s == get_enum_name(static_cast<T>(i))) {
            return static_cast<T>(i);
        }
    }
    throw std::invalid_argument("Invalid enum name");
}

template <class T>
T enum_from_name(const std::string &s) {
    return enum_from_name<T, static_cast<T>(0), static_cast<T>(256)>(s);
}

}

#endif /* LEARN_CPP_SCIENUM_H */
