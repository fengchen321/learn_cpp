#include <iostream>
#include "cppdemangle.h"

#if defined(_MSC_VER)
    #define FUNC_SIGNATURE __FUNCSIG__
#else
    #define FUNC_SIGNATURE __PRETTY_FUNCTION__
#endif

void func(int&& a) {
    printf("%s\n", FUNC_SIGNATURE);
}

void func(const int& a) {
    printf("%s\n", FUNC_SIGNATURE);
}
// https://zh.cppreference.com/w/cpp/language/value_category
void test_value() {
    int a = 1;
    printf("%s\n",cppdemangle<decltype("ssss")>().c_str());  //lvaue
    printf("%s\n",cppdemangle<decltype(1)>().c_str());       // prvalue
    printf("%s\n",cppdemangle<decltype(std::move(a))>().c_str()); // xvalue
    printf("%s\n",cppdemangle<decltype((a))>().c_str());  // lvalue decltype((a)):decltype套一层括号测试表达式类
    func(1);  // prvalue
    func(std::move(a));  // xvalue
    func(a); // lvalue

}

struct S {
    int a;
    int &b;
    int &&c;
};

void test_struct() {
    int a = 1, b = 1, c = 1;
    S s{a, b, std::move(c)};
    printf("%s\n",cppdemangle<decltype(s.a)>().c_str());    // prvalue
    printf("%s\n",cppdemangle<decltype((s.a))>().c_str());  // lvalue
    printf("%s\n",cppdemangle<decltype(s.b)>().c_str());    // lvalue
    printf("%s\n",cppdemangle<decltype((s.b))>().c_str());  // lvalue
    printf("%s\n",cppdemangle<decltype(s.c)>().c_str());    // xvalue
    printf("%s\n",cppdemangle<decltype((s.c))>().c_str());  // lvalue
    func(s.c);  // （s.c）左值表达式
}

struct S2 {
    int a;
    int getA() const & {
        printf("%s\n", FUNC_SIGNATURE);
        return a;
    }

    int getA() & {
        printf("%s\n", FUNC_SIGNATURE);
        return a;
    }

    int getA() const && {
        printf("%s\n", FUNC_SIGNATURE);
        return a;
    }

    int getA() && {
        printf("%s\n", FUNC_SIGNATURE);
        return a;
    }
};

void test_struct2() {
    printf("%s\n",cppdemangle<decltype(S2().a)>().c_str()); // 临时对象的成员变量 prvalue
    printf("%s\n",cppdemangle<decltype((S2().a))>().c_str()); //xvalue
    S2().getA();

    S2 s;
    s.getA();
    std::move(s).getA();

    const S2 cs2{};
    cs2.getA();
    std::move(cs2).getA();
}

int main() {
    test_value();
    test_struct();
    test_struct2();
}