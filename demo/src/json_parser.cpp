#include "gtest_prompt.h"
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>
#include <charconv>
#include <regex>
#include <optional>

struct JSONObject;
using JSONList = std::vector<JSONObject>;
using JSONDict = std::unordered_map<std::string, std::unique_ptr<JSONObject>>;
// std::unordered_map<std::string, JSONObject> gcc11.4.0还不行
// 自己引用自己，只能存在堆上；std::unique_ptr<JSONObject>可以
// struct内JSONObject或std::tuple<int, JSONObject>存在栈上不行
struct JSONObject {
    std::variant
    < std::nullptr_t    // null
    , bool              // true
    , int               // 42
    , double            // 3.14
    , std::string       // "hello"  
    , JSONList          // [42, "hello"]
    , JSONDict         // {"key": 42}
    > inner;

    // Constructors for different types
    JSONObject() : inner(nullptr) {}  // for null
    JSONObject(std::nullptr_t) : inner(nullptr) {}
    JSONObject(bool b) : inner(b) {}
    JSONObject(int i) : inner(i) {}
    JSONObject(double d) : inner(d) {}
    JSONObject(const std::string& str) : inner(str) {}
    JSONObject(JSONList list) : inner(std::move(list)) {}
    JSONObject(JSONDict dict) : inner(std::move(dict)) {}

    void do_print() const {
        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                std::cout << "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                std::cout << (arg ? "true" : "false");
            } else if constexpr (std::is_same_v<T, int>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, double>) {
                std::cout << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "\"" << arg << "\"";
            } else if constexpr (std::is_same_v<T, JSONList>) {
                std::cout << "[";
                for (size_t i = 0; i < arg.size(); ++i) {
                    arg[i].do_print();
                    if (i < arg.size() - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << "]";
            } else if constexpr (std::is_same_v<T, JSONDict>) {
                std::cout << "{";
                auto it = arg.begin();
                while (it != arg.end()) {
                    std::cout << "\"" << it->first << "\": ";
                    it->second->do_print();
                    ++it;
                    if (it != arg.end()) {
                        std::cout << ", ";
                    }
                }
                std::cout << "}";
            }
        }, inner);
    }

    template <class T>
    bool is() const {
        return std::holds_alternative<T>(inner);
    }

    template <class T>
    T const& get() const {
        return std::get<T>(inner);
    }

    template <class T>
    T& get() {
        return std::get<T>(inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

char unescaped_char(char c) {
    switch (c) {
        case 'n': return '\n';
        case 'r': return '\r';
        case '0': return '\0';
        case 't': return '\t';
        case 'v': return '\v';
        case 'f': return '\f';
        case 'b': return '\b';
        case 'a': return '\a';
        default: return c;
    }
}

std::pair<JSONObject, size_t> parse(std::string_view json) {
    if (json.empty()) {
        return {JSONObject{std::nullptr_t{}}, 0};
    } else if (size_t off = json.find_first_not_of(" \n\r\t\v\f\0"); off != 0 && off != json.npos) {
        auto [obj, eaten] = parse(json.substr(off));
        return {std::move(obj), eaten + off};
    } else if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) {
            std::string str = match.str();
            if (auto num = try_parse_num<int>(str)) {
                return {JSONObject{*num}, str.size()};
            }
            if (auto num = try_parse_num<double>(str)) {
                return {JSONObject{*num}, str.size()};
            }
        }
    } else if (json[0] == '"') {
        std::string str;
        enum {
            Raw,
            Escaped,
        } phase = Raw;
        size_t i;
        for (i = 1; i < json.size(); i++) {
            char ch = json[i];
            if (phase == Raw) {
                if (ch == '\\') {
                    phase = Escaped;
                } else if (ch == '"') {
                    i += 1;
                    break;
                } else {
                    str += ch;
                }
            } else if (phase == Escaped) {
                str += unescaped_char(ch);
                phase = Raw;
            }
        }
        return {JSONObject{std::move(str)}, i};
    } else if (json[0] == '[') {
        JSONList res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == ']') {
                i += 1;
                break;
            }
            auto [obj, eaten] = parse(json.substr(i));
            if (eaten == 0) {
                i = 0;
                break;
            }
            res.push_back(std::move(obj));
            i += eaten;
            if (json[i] == ',') {
                i += 1;
            }
        }
        return {JSONObject{std::move(res)}, i};
    } else if (json[0] == '{') {
        JSONDict res;
        size_t i;
        for (i = 1; i < json.size();) {
            if (json[i] == '}') {
                i += 1;
                break;
            }
            auto [keyobj, keyeaten] = parse(json.substr(i));
            if (keyeaten == 0) {
                i = 0;
                break;
            }
            i += keyeaten;
            if (!std::holds_alternative<std::string>(keyobj.inner)) {
                i = 0;
                break;
            }
            if (json[i] == ':') {
                i += 1;
            }
            std::string key = std::move(std::get<std::string>(keyobj.inner));
            auto [valobj, valeaten] = parse(json.substr(i));
            if (valeaten == 0) {
                i = 0;
                break;
            }
            i += valeaten;
            // insert_or_assign
            res.try_emplace(std::move(key), std::make_unique<JSONObject>(std::move(valobj)));
            if (json[i] == ',') {
                i += 1;
            }
        }
        return {JSONObject{std::move(res)}, i};
    }
    return {JSONObject{std::nullptr_t{}}, 0};
}

template <class ...Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

template <class ...Fs>
overloaded(Fs...) -> overloaded<Fs...>;


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