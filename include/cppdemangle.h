#ifndef LEARN_CPP_CPPDEMANGLE_H
#define LEARN_CPP_CPPDEMANGLE_H

#include <typeinfo>
#include <type_traits>
#include <string>
#include <memory>
#if (defined(__GNUC__) || defined(__clang__)) && __has_include(<cxxabi.h>)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace _cppdemangle_details {

static std::string cppdemangle(const char *name) {
#if (defined(__GNUC__) || defined(__clang__)) && __has_include(<cxxabi.h>)
    // int status;
    // char *p = abi::__cxa_demangle(name, 0, 0, &status);
    // std::string s = p ? p : name;
    // std::free(p);

    int status = 0;
    std::unique_ptr<char[], decltype(&std::free)> res {
            abi::__cxa_demangle(name, nullptr, nullptr, &status),
            std::free
    };
    std::string s = (status == 0) ? res.get() : name;
#else
    std::string s = name;
#endif
    return s;
}

static std::string cppdemangle(std::type_info const &type) {
    return cppdemangle(type.name());
}

template <class T>
static std::string cppdemangle() {
    std::string s{cppdemangle(typeid(std::remove_cv_t<std::remove_reference_t<T>>))};
    if (std::is_const_v<std::remove_reference_t<T>>)
        s += " const";
    if (std::is_volatile_v<std::remove_reference_t<T>>)
        s += " volatile";
    if (std::is_lvalue_reference_v<T>)
        s += " &";
    if (std::is_rvalue_reference_v<T>)
        s += " &&";
    return s;
}

}

using _cppdemangle_details::cppdemangle;

// Usage:
//
// cppdemangle<int>()
// => "int"
//
// int i;
// cppdemangle<decltype(i)>()
// => "int"
//
// int i;
// cppdemangle<decltype(std::as_const(i))>()
// => "int const &"
#endif // LEARN_CPP_CPPDEMANGLE_H