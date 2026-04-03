#include "gtest_prompt.h"
#include "prompt.h"
#include "template_type_conversion.h"

TEST(TypeConversion, type_check) {
    EXPECT_TRUE((is_same_any<int, int, float>::value));
    EXPECT_FALSE((is_same_any<double, int, float>::value));
    EXPECT_TRUE((is_convertible_any<int, float, double>::value));
    EXPECT_FALSE((is_convertible_any<std::string, int, float>::value));
}

TEST(TypeConversion, VecInt) {
    auto it = vec_int(1, 1u);
    using ElementType = decltype(it)::value_type;
    EXPECT_TRUE((std::is_same_v<ElementType, int>));
    EXPECT_FALSE((std::is_same_v<ElementType, unsigned int>));
}

TEST(TypeConversion, TupleSize) {
    using Tup = std::tuple<int, float, double>;
    constexpr auto i = std::tuple_size<Tup>::value;
    constexpr auto j = tuple_size<Tup>::value;
    ASSERT_EQ(i, j);
    ASSERT_EQ(i, 3);
}

TEST(TypeConversion, TupleApplyCommonType) {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

    using what = tuple_apply_common_type<Tup>::type;
    using what2 = tuple_apply_common_type<Var>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"double");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"double");
}

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
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what3).name()), R"(std::variant<int\s*,\s*float\s*,\s*double>)");
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what4).name()), R"(std::tuple<int\s*,\s*float\s*,\s*double>)");

    using Object = std::variant<int, double, std::string>;
    using ObjectVec = tuple_map<vector_wrapper, Object>::type;
    using ObjectList = tuple_apply<tuple_wrapper, ObjectVec>::type;
    EXPECT_REGEX_MATCH(cppdemangle(typeid(ObjectList).name()), R"(std::tuple<std::vector<int,)");
}

TEST(TypeConversion, TupleMap) {
    using Tup = std::tuple<int, float, double>;
    using Var = std::variant<int, float, double>;

    using what = tuple_map<vector_wrapper, Tup>::type;
    using what2 = tuple_map<vector_wrapper, Var>::type;
    using what3 = tuple_map<array_wrapper<3>, Tup>::type;

    std::cout << cppdemangle(typeid(what).name()) << std::endl;
    std::cout << cppdemangle(typeid(what2).name()) << std::endl;
    std::cout << cppdemangle(typeid(what3).name()) << std::endl;
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what).name()), R"(.*std::tuple<.*std::vector)");
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what2).name()), R"(.*std::variant<.*std::vector)");
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what3).name()), R"(.*std::tuple<.*std::array)");
}

TEST(TypeConversion, TupleCat) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<std::vector<double>, std::array<int, 4>>;

    using what = tuple_cat<Tup, Tup2>::type;
    using what2 = tuple_push_front<char *, Tup>::type;
    std::cout << cppdemangle(typeid(what).name()) << std::endl;
    std::cout << cppdemangle(typeid(what2).name()) << std::endl;

    EXPECT_REGEX_MATCH(cppdemangle(typeid(what).name()), R"(.*std::tuple<int.*,.*float.*,.*double.*,.*std::vector.*)");
    EXPECT_REGEX_MATCH(cppdemangle(typeid(what2).name()), R"(.*std::tuple<char.*\*.*,.*int.*,.*float.*,.*double.*>)");
}

TEST(TypeConversion, TupleGetFirst) {
    using Tup = std::tuple<int, float, double>;
    using Tup2 = std::tuple<std::vector<double>, std::array<int, 4>>;

    using what = tuple_get_first<Tup>::type;
    using what2 = tuple_get_first<Tup2>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_NE(cppdemangle(typeid(what2).name()).find("std::vector<double"), std::string::npos)
        << "Substring not found in the full string";
}

TEST(TypeConversion, TupleFront) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_front<Tup>::type;
    using what2 = tuple_front<std::tuple<>>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"void");
}

TEST(TypeConversion, Tuple2D) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_2d<Tup>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"float");
}

TEST(TypeConversion, TupleElement) {
    using Tup = std::tuple<int, float, double>;
    using what = tuple_element<0, Tup>::type;
    using what2 = tuple_element<1, Tup>::type;
    using what3 = tuple_element<2, Tup>::type;

    EXPECT_STREQ(cppdemangle(typeid(what).name()).c_str(),"int");
    EXPECT_STREQ(cppdemangle(typeid(what2).name()).c_str(),"float");
    EXPECT_STREQ(cppdemangle(typeid(what3).name()).c_str(),"double");
}

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
