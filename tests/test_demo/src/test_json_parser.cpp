#include "gtest_prompt.h"
#include "json_parser.h"

TEST(JSONParse, StringViewTest) {
    std::string_view str1 = "123456789";
    ASSERT_EQ(str1.substr(0, 3), "123");
    ASSERT_EQ(str1.substr(4, 3), "567");
    ASSERT_EQ(str1.substr(4, 10000000), "56789");
}

TEST(JSONParse, DoPrintTest) {
    JSONObject obj1;
    ASSERT_LOGS_STDOUT(obj1.do_print(), "null");

    JSONObject obj2{bool{true}};
    ASSERT_LOGS_STDOUT(obj2.do_print(), "true");

    JSONObject obj3{int{42}};
    ASSERT_LOGS_STDOUT(obj3.do_print(), "42");

    JSONObject obj4{double{3.14}};
    ASSERT_LOGS_STDOUT(obj4.do_print(), "3.14");

    JSONObject obj5{std::string{"hello"}};
    ASSERT_LOGS_STDOUT(obj5.do_print(), "\"hello\"");


    JSONList list;
    list.push_back(JSONObject{std::string("item1")});
    list.push_back(JSONObject{std::string("item2")});
    JSONObject obj6{std::move(list)};
    ASSERT_LOGS_STDOUT(obj6.do_print(), "[\"item1\", \"item2\"]");

    JSONDict dict;
    dict["key"] = std::make_unique<JSONObject>(std::string("value"));
    JSONObject obj7{std::move(dict)};
    ASSERT_LOGS_STDOUT(obj7.do_print(), "{\"key\": \"value\"}");
}

TEST(JsonParse, TryParseNum) {
    EXPECT_EQ(*(try_parse_num<int>("42")), 42);
    EXPECT_EQ(*(try_parse_num<int>("-456")), -456);

    EXPECT_FALSE(try_parse_num<int>("abc").has_value());
    EXPECT_FALSE(try_parse_num<int>("").has_value());

    EXPECT_EQ(*(try_parse_num<double>("3.14")), 3.14);
    EXPECT_FALSE(try_parse_num<double>("12.34abc").has_value());
}

TEST(JSONParse, PaseNums) {
    std::string_view str1 = "42";
    auto [obj1, size1] = parse(str1);
    EXPECT_EQ(obj1.inner.index(), 2);
    EXPECT_EQ(size1, 2);
    ASSERT_LOGS_STDOUT(obj1.do_print(), "42");

    std::string_view str2 = "3.14";
    auto [obj2, size2] = parse(str2);
    EXPECT_EQ(obj2.inner.index(), 3);
    EXPECT_EQ(size2, 4);
    ASSERT_LOGS_STDOUT(obj2.do_print(), "3.14");

    std::string_view str3 = "3.14dfnkafhjasrhf";
    auto [obj3, size3] = parse(str3);
    EXPECT_EQ(obj3.inner.index(), 3);
    EXPECT_EQ(size3, 4);
    ASSERT_LOGS_STDOUT(obj3.do_print(), "3.14");
}

TEST(JSONParse, PaseString) {
    std::string_view str = "";
    auto [obj, size] = parse(str);
    EXPECT_EQ(obj.inner.index(), 0);
    EXPECT_EQ(size, 0);
    ASSERT_LOGS_STDOUT(obj.do_print(), "");

    std::string_view str1 = "\"hello\"";
    auto [obj1, size1] = parse(str1);
    EXPECT_EQ(obj1.inner.index(), 4);
    EXPECT_EQ(size1, 7);
    ASSERT_LOGS_STDOUT(obj1.do_print(), "\"hello\"");

    std::string_view str2 = R"("hello\tas\n\"s")";
    auto [obj2, size2] = parse(str2);
    EXPECT_EQ(obj2.inner.index(), 4);
    EXPECT_EQ(size2, 16);
    ASSERT_LOGS_STDOUT(obj2.do_print(), "\"hello\tas\n\"s\"");
}

TEST(JSONParse, PaseList) {
    std::string_view str1 = "[1, 2, 3]";
    auto [obj1, size1] = parse(str1);
    EXPECT_EQ(obj1.inner.index(), 5);
    EXPECT_EQ(size1, 9);
    ASSERT_LOGS_STDOUT(obj1.do_print(), "[1, 2, 3]");

    std::string_view str2 = "[1, 2, [4, 5, 6], 3,]";
    auto [obj2, size2] = parse(str2);
    EXPECT_EQ(obj2.inner.index(), 5);
    EXPECT_EQ(size2, 21);
    ASSERT_LOGS_STDOUT(obj2.do_print(), "[1, 2, [4, 5, 6], 3]");
    // 漏写情况
    std::string_view str3 = "[1, 2, [4, 5, 6], 3,";
    auto [obj3, size3] = parse(str3);
    EXPECT_EQ(obj3.inner.index(), 5);
    EXPECT_EQ(size3, 20);
    ASSERT_LOGS_STDOUT(obj3.do_print(), "[1, 2, [4, 5, 6], 3]");

    std::string_view str4 = R"([1, 2, [4, "hello", 6], 3])";
    auto [obj4, size4] = parse(str4);
    EXPECT_EQ(obj4.inner.index(), 5);
    EXPECT_EQ(size4, 26);
    ASSERT_LOGS_STDOUT(obj4.do_print(), "[1, 2, [4, \"hello\", 6], 3]");

    // 混淆情况
    std::string_view str5 = R"([1, 2, [4, "hello]", 6], 3,])";
    auto [obj5, size5] = parse(str5);
    EXPECT_EQ(obj5.inner.index(), 5);
    EXPECT_EQ(size5, 28);
    ASSERT_LOGS_STDOUT(obj5.do_print(), "[1, 2, [4, \"hello]\", 6], 3]");
}

TEST(JSONParse, PaseDict) {
    // insert_or_assign 和 try_empolace区别：insert_or_assign会覆盖已有键值对，try_empolace不会
    std::string_view str1 = R"({"work": 996, "school": [985,211], "school": "xxx"})";
    auto [obj1, size1] = parse(str1);
    EXPECT_EQ(obj1.inner.index(), 6);
    EXPECT_EQ(size1, 51);
    ASSERT_LOGS_STDOUT(obj1.do_print(), "{\"school\": [985, 211], \"work\": 996}");

    std::string_view str2 = R"({"work": 996, "school": {"hello":985, "world":211}})";
    auto [obj2, size2] = parse(str2);
    EXPECT_EQ(obj2.inner.index(), 6);
    EXPECT_EQ(size2, 51);
    ASSERT_LOGS_STDOUT(obj2.do_print(), "{\"school\": {\"world\": 211, \"hello\": 985}, \"work\": 996}");
}
// 判断对象类型
TEST(JSONParse, IsPObject) {
    std::string_view str1 = "123";
    auto [obj, size] = parse(str1);
    EXPECT_TRUE(obj.is<int>());
    EXPECT_FALSE(obj.is<double>());

    std::string_view str2 = "3.14";
    auto [obj2, size2] = parse(str2);
    EXPECT_TRUE(obj2.is<double>());

    std::string_view str3 = "\"hello\"";
    auto [obj3, size3] = parse(str3);
    EXPECT_TRUE(obj3.is<std::string>());
    
    std::string_view str4 = "[1, 2, 3]";
    auto [obj4, size4] = parse(str4);
    EXPECT_TRUE(obj4.is<JSONList>());

    std::string_view str5 = R"({"work": 996, "school": [985,211]})";
    auto [obj5, size5] = parse(str5);
    EXPECT_TRUE(obj5.is<JSONDict>());

    std::string_view str6 = "";
    auto [obj6, size6] = parse(str6);
    EXPECT_TRUE(obj6.is<std::nullptr_t>());

}
// 获取指定类型对象
TEST(JSONParse, GetPObject) {
    std::string_view str1 = "123";
    auto [obj, size] = parse(str1);
    EXPECT_EQ(obj.get<int>(), 123);

    std::string_view str2 = "3.14";
    auto [obj2, size2] = parse(str2);
    EXPECT_EQ(obj2.get<double>(), 3.14);

    std::string_view str5 = R"({"work": 996, "school": [985,[211, 1]})";
    auto [obj5, size5] = parse(str5);
    auto const &dict = obj5.get<JSONDict>();
    EXPECT_EQ(dict.at("work")->get<int>(), 996);
    auto const &school = dict.at("school");
    auto dovisit = [&] (auto &dovisit, JSONObject const &school) -> void {
        std::visit([&] (auto const &school) {
            using T = std::decay_t<decltype(school)>;
            if constexpr (std::is_same_v<T, JSONList>) {
                for (auto const &item : school)
                    dovisit(dovisit, item); // 递归调用
            } else if constexpr (std::is_same_v<T, JSONDict>) {
                for (auto const &pair : school) {
                    std::cout << "\"" << pair.first << "\": ";
                    dovisit(dovisit, *pair.second);
                }
            } else {
                std::cout << school << "\n"; // 打印其他类型的值
            }
        }, school.inner);
    };
    ASSERT_LOGS_STDOUT(dovisit(dovisit, *school), "985\n211\n1\n");
}

TEST(JSONParse, Overloaded) {
    std::string_view str = R"JSON({"number": 985.211, "text": "hello"})JSON";
    auto [obj, eaten] = parse(str);
    std::visit(
        overloaded{
            [](std::nullptr_t) {
                std::cout << "null" << std::endl;
            },
            [](bool b) {
                std::cout << (b ? "true" : "false") << std::endl;
            },
            [](int i) {
                std::cout << i << std::endl;
            },
            [](double d) {
                std::cout << d << std::endl;
            },
            [](const std::string& str) {
                std::cout << "\"" << str << "\"" << std::endl;
            },
            [](const JSONList& list) {
                std::cout << "[";
                for (size_t i = 0; i < list.size(); ++i) {
                    list[i].do_print();
                    if (i < list.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]" << std::endl;
            },
            [](const JSONDict& dict) {
                std::cout << "{";
                auto it = dict.begin();
                while (it != dict.end()) {
                    std::cout << "\"" << it->first << "\": ";
                    it->second->do_print();
                    ++it;
                    if (it != dict.end()) {
                        std::cout << ", ";
                    }
                }
                std::cout << "}" << std::endl;
            }
        },
        obj.inner);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}