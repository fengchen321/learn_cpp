#include <iostream>
#include "cppdemangle.h"
#include "gtest_prompt.h"

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
TEST(ValueCategory, ValueTest) {
    int a = 1;
    EXPECT_STREQ(cppdemangle<decltype("ssss")>().c_str(),"char [5] const &"); // lvaue
    EXPECT_STREQ(cppdemangle<decltype(1)>().c_str(),"int");       // prvalue
    EXPECT_STREQ(cppdemangle<decltype(std::move(a))>().c_str(),"int &&");  // xvalue
    EXPECT_STREQ(cppdemangle<decltype((a))>().c_str(),"int &");  // lvalue decltype((a)):decltype套一层括号测试表达式类
    ASSERT_REGEX_STDOUT(func(1), R"(void.*func\(int\s*&&\))");  // prvalue
    ASSERT_REGEX_STDOUT(func(std::move(a)), R"(void.*func\(int\s*&&\))");  // xvalue
    ASSERT_REGEX_STDOUT(func(a), R"(void.*func\(const\s*int\s*&\))");  // lvalue  void func(const int&)
}

struct S {
    int a;
    int &b;
    int &&c;
};

TEST(ValueCategory, StructTest) {
    int a = 1, b = 1, c = 1;
    S s{a, b, std::move(c)};

    EXPECT_STREQ(cppdemangle<decltype(s.a)>().c_str(),"int"); // prvalue
    EXPECT_STREQ(cppdemangle<decltype((s.a))>().c_str(),"int &"); // lvalue
    EXPECT_STREQ(cppdemangle<decltype(s.b)>().c_str(),"int &"); // lvalue
    EXPECT_STREQ(cppdemangle<decltype((s.b))>().c_str(),"int &"); // lvalue
    EXPECT_STREQ(cppdemangle<decltype(s.c)>().c_str(),"int &&"); // xvalue
    EXPECT_STREQ(cppdemangle<decltype((s.c))>().c_str(),"int &"); // lvalue
    ASSERT_REGEX_STDOUT(func(s.c), R"(void.*func\(const\s*int\s*&\))");  // （s.c）左值表达式 "void func(const int&)
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

TEST(ValueCategory, Struct2Test) {
    EXPECT_STREQ(cppdemangle<decltype(S2().a)>().c_str(),"int"); // // 临时对象的成员变量 prvalue
    EXPECT_STREQ(cppdemangle<decltype((S2().a))>().c_str(),"int &&"); // //xvalue
    ASSERT_REGEX_STDOUT(S2().getA(), R"(int.*S2::getA\(.*\)\s*&&)");  // int S2::getA() &&

    S2 s;
    ASSERT_REGEX_STDOUT(s.getA(), R"(int.*S2::getA\(.*\)\s*&)");  // int S2::getA() &
    ASSERT_REGEX_STDOUT(std::move(s).getA(), R"(int.*S2::getA\(.*\)\s*&&)"); // int S2::getA() &&

    const S2 cs2{};
    ASSERT_REGEX_STDOUT(cs2.getA();, R"(int.*S2::getA\(.*\)\s*const\s*&)");  // int S2::getA() const &
    ASSERT_REGEX_STDOUT(std::move(cs2).getA(), R"(int.*S2::getA\(.*\)\s*const\s*&&)"); // int S2::getA() const &&

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}