#include "gtest_prompt.h"
#include "prompt.h"
#include "template_common_type.h"

TEST(CommonType, ArrayFunc) {
    auto a = array_func(1, 2.0, 3.0f);
    using ElementType = decltype(a)::value_type;
    EXPECT_FALSE((std::is_same_v<ElementType, int>));
    EXPECT_FALSE((std::is_same_v<ElementType, float>));
    EXPECT_TRUE((std::is_same_v<ElementType, double>));
}

TEST(CommonType, CommonTypeRecursive) {
    using what = typename common_type_two<int, long>::type;
    EXPECT_FALSE((std::is_same_v<what, int>));
    EXPECT_TRUE((std::is_same_v<what, long>));

    using what2 = typename common_type<int, long, double>::type;
    EXPECT_FALSE((std::is_same_v<what2, int>));
    EXPECT_FALSE((std::is_same_v<what2, long>));
    EXPECT_TRUE((std::is_same_v<what2, double>));
}

TEST(CommonType, CommonTypeRecursiveStruct) {
    using what = typename common_type_two<Cat, Animal>::type;
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what).name()), "Animal");

    using what2 = typename common_type<Cat, Dog, Animal>::type;
    EXPECT_CONTAINS_ALL(cppdemangle(typeid(what2).name()), "Animal");
}

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
