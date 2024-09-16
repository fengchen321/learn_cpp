#include <iostream>
#include <string>
#include "scienum.h"

// https://github.com/Neargye/magic_enum/
#if defined(_MSC_VER)
    #define FUNC_SIGNATURE __FUNCSIG__
#else
    #define FUNC_SIGNATURE __PRETTY_FUNCTION__
#endif

#define LOG(x) (std::cout << __FILE__ << ":" << __LINE__ << " " << FUNC_SIGNATURE << ": " << (x) << std::endl)

#if __cplusplus >= 202002L
#include <source_location>
void log(std::string msg, std::source_location sl = std::source_location::current()) {
    std::cout << sl.file_name() << ":" << sl.line() << " " 
        << sl.function_name() << ": " <<  msg << std::endl;
}
#else
void log(std::string msg) {
    std::cout << "not supported source_location\n";
}
#endif

template <class T>
std::string get_type_name() {
    std::string s =  FUNC_SIGNATURE;
    auto pos = s.find("T = ");
    pos += 4;
    auto pos2 = s.find_first_of(" ;]", pos);
    return s.substr(pos, pos2 - pos);
}

enum Color {
    RED =1 ,
    GREEN = 2,
    BLUE = 3
};
template <class T, T N>
std::string get_int_name() {
    std::string s =  FUNC_SIGNATURE;
    auto pos = s.find("N = ");
    pos += 4;
    auto pos2 = s.find_first_of(" ;]", pos);
    return s.substr(pos, pos2 - pos);
}
// 编译期for循环 static_for  -> https://www.bilibili.com/video/BV1NM4y1e7Hn?t=3931.9 demo/src/template_compile_for_loop.cpp
template <int X>
struct int_constant {
    static constexpr int value = X;
};

template <int Beg, int End, class F>
void static_for_1(F const &func) {
    if constexpr (Beg == End) {
        return;
    } else {
        func(int_constant<Beg>());
        static_for_1<Beg + 1, End>(func);
    }
}

template <class T>
std::string get_int_name_dynamic(T value) {
    // if (value == (T)1) return get_int_name<T, (T)1>();
    // if (value == (T)2) return get_int_name<T, (T)2>();
    // if (value == (T)3) return get_int_name<T, (T)3>();

    std::string ret;
    static_for_1<1, 4>([&](auto ic) {
        constexpr auto i = ic.value;
        if (value == (T)i) {
            ret = get_int_name<T, (T)i>();
        }
    });
    return ret;
}


void test() {
    Color c = RED;
    std::cout << get_int_name_dynamic(c) << std::endl;
}

void test_scienum() {
    std::cout << scienum::get_enum_name(BLUE) << std::endl;
    std::cout << scienum::enum_from_name<Color>("BLUE") << std::endl;
    std::cout << scienum::get_enum_name<Color, (Color)1, (Color)3>(BLUE) << std::endl;
    // std::cout << scienum::enum_from_name<Color>("xxx") << std::endl;
}

int main() {
    // LOG("Hello world!");
    // log("Hello world!");
    // test();
    test_scienum();
    return 0;
}