#include "gtest_prompt.h"
#include "template_compile_reflect.h"

TEST(CompileReflect, Log) {
    LOG("Hello world!");
    log("Hello world!");
}

TEST(CompileReflect, GetTypeName) {
    EXPECT_STREQ(get_type_name<double>().c_str(),"double");
    EXPECT_REGEX_MATCH(get_type_name<std::vector<int>>(),R"(.*std::vector<int.*)");
}

enum Color {
    RED =1 ,
    GREEN = 2,
    BLUE = 3
};


TEST(CompileReflect, GetTypeNameDynamic){
    Color c = RED;
    EXPECT_REGEX_MATCH(get_int_name_dynamic(c),R"(.*Color)");
}

TEST(CompileReflect, ScienumTest){
    EXPECT_STREQ(scienum::get_enum_name(BLUE).c_str(),"BLUE"); 
    EXPECT_EQ(scienum::enum_from_name<Color>("BLUE"),BLUE);
    std::string s = scienum::get_enum_name<Color, (Color)1, (Color)3>(RED);
    EXPECT_STREQ(s.c_str(),"RED");

    ASSERT_THROW_EXCEPTION(scienum::enum_from_name<Color>("xxx"), "Invalid enum name");
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}