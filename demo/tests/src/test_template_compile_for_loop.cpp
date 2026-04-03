#include "gtest_prompt.h"
#include "prompt.h"
#include "template_compile_for_loop.h"
#include <vector>
#include <variant>

std::vector<size_t> captured_indices;
#if !defined(_MSC_VER)
TEST(StaticForTest, CorrectIndicesProcessed) {
    captured_indices.clear();

    static_for<0, 4>([&]<size_t I>() {
        captured_indices.push_back(I);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}
#endif

TEST(StaticForTest, IntConstant) {
    captured_indices.clear();

    static_for_1<0, 4>([&](auto i){
        captured_indices.push_back(i.value);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}

TEST(StaticForTest, TupleSample) {
    std::tuple<int, float, double, char> tup{0, 1.0, 2.0, 'c'};
    static_for_1<0, 4>([&](auto i){
        if constexpr (i == 0) {
            EXPECT_EQ(std::get<i>(tup), 0);
            EXPECT_EQ(std::get<i>(tup), 0);
            EXPECT_EQ(std::get<i()>(tup), 0);// 编译期隐式类型转换
        } else if constexpr (i == 1) {
            EXPECT_EQ(std::get<i>(tup), 1.0f);
        } else if constexpr (i == 2) {
            EXPECT_EQ(std::get<i>(tup), 2.0);
        } else if constexpr (i == 3) {
            EXPECT_EQ(std::get<i>(tup), 'c');
        }
    });
}

TEST(StaticForTest, VariantSample) {
    std::variant<int, float, double, char> var = {'c'};
    std::visit([&](auto const &v){
        EXPECT_EQ(v, 'c');
    }, var); // 推荐

    static_for_1<0, 4>([&](auto i){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'c');
            return true;
        }
        return false;
    });
}

TEST(StaticForTest, IndexSequence) {
    captured_indices.clear();
    static_for_2<4>([&](auto i){
        captured_indices.push_back(i.value);
    });

    ASSERT_EQ(captured_indices.size(), 4);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(captured_indices[i], i);
    }
}

TEST(StaticForTest, TupMap) {
    std::tuple<int, float> tup = {42, 3.14f};
    auto res = tup_map([](auto v){
        return v + 1;
    }, tup);

    EXPECT_EQ(std::get<0>(res), 43);
    // EXPECT_NEAR(std::get<1>(res), 4.14f, 1e-5f);
    ASSERT_FLOAT_EQ(std::get<1>(res), 4.14f);
}

TEST(StaticForTest, TupApply) {
    std::tuple<int, float> tup = {42, 3.14f};
    auto res_tupapply = tup_apply([] (auto ...xs) { 
        return (xs + ...); 
    }, tup);
    ASSERT_FLOAT_EQ(res_tupapply, 45.14f);

    auto res_tupapply2 = tup_apply([] (int i, float f) { 
        return (i + f); 
    }, tup);
    ASSERT_FLOAT_EQ(res_tupapply2, 45.14f);
}

TEST(StaticForTest, StaticforBreaker) {
    std::variant<int, float, double, char> var = {'b'};
    static_for_3<0, 4>([&](auto i, auto ctrl){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'b');
            ctrl.static_break();
        }
    });
}

TEST(StaticForTest, StaticforBreaker1) {
    std::variant<int, float, double, char> var = {'b'};
    static_for_3<0, 4>([&](auto i){
        if (i.value == var.index()) {
            EXPECT_EQ(std::get<i.value>(var), 'b');
        }
    });
}

TEST(StaticForTest, StaticforBreaker2) {
    std::vector<size_t> vec;
    // 打印到 2 时提前退出
    static_for_3<0, 4>([&](auto i, auto ctrl) {
        vec.push_back(i.value);
        if (i.value == 2) {
            ctrl.static_break();
        }
    });
    EXPECT_EQ(vec.size(), 3); 
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

// json解析器中使用variant和visit 展开variant内容
void show(std::variant<int, double, std::string> t) {
    std::visit([](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "int: " << arg << std::endl;
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "double: " << arg << std::endl;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "string: " << arg << std::endl;
        }
    }, t);
}

void show_1(std::variant<int, double, std::string> t) {
    std::visit([](auto &&arg) {
        std::cout << "arg: " << arg << std::endl;
    }, t);
}

void show_2(std::variant<int, double, std::string> t) {
    auto visitor = [](auto &&arg) {
        std::cout << "arg: " << arg << std::endl;
    };
    if (t.index() == 0) {  // 等价 std::holds_alterative<int>(t)
        visitor(std::get<int>(t));
    } else if (t.index() == 1) {
        visitor(std::get<double>(t));
    } else if (t.index() == 2) {
        visitor(std::get<std::string>(t));
    }
}

TEST(StaticForTest, Example_1_show) {
    show(42);
    show(3.14);
    show("hello");
    show_1(42);
    show_1(3.14);
    show_1("hello");
    show_2(42);
    show_2(3.14);
    show_2("hello");
}

// 使用 std::variant 存储对象
static std::vector<std::variant<int, double, std::string>> objects;
void add_object(std::variant<int, double, std::string> obj) {
    objects.push_back(obj);
}

void print_objects() {
    for (auto &obj : objects) {
        show(obj);
    }
}

TEST(StaticForTest, Example_1_1) {
    add_object(42);
    add_object(3.14);
    add_object("hello");
    print_objects();
}


// 拆分存储对象以节省字节
// std::variant 40字节 = 占用大小为内部类型中最大者 32 + index的size_t 8
static std::vector<int> int_objects;
static std::vector<double> double_objects;
static std::vector<std::string> string_objects;

void add_object_1(std::variant<int, double, std::string> obj) {
    if (std::holds_alternative<int>(obj)) {
        int_objects.push_back(std::get<int>(obj));
    } else if (std::holds_alternative<double>(obj)) {
        double_objects.push_back(std::get<double>(obj));
    } else if (std::holds_alternative<std::string>(obj)) {
        string_objects.push_back(std::get<std::string>(obj));
    }
}

void print_objects_1() {
    for (auto &obj : int_objects) {
        std::cout << "int: " << obj << std::endl;
    }
    for (auto &obj : double_objects) {
        std::cout << "double: " << obj << std::endl;
    }
    for (auto &obj : string_objects) {
        std::cout << "string: " << obj << std::endl;
    }
}

TEST(StaticForTest, Example_1_2) {
    add_object_1(42);
    add_object_1(3.14);
    add_object_1("hello");
    print_objects_1();
}
// 使用 std::tuple 存储对象, 并使用 static_for_2 遍历
static std::tuple<std::vector<int>, std::vector<double>, std::vector<std::string>> objects_1;
void add_object_2(std::variant<int, double, std::string> obj) {
    // if (obj.index() == 0) {
    //     std::get<0>(objects_1).push_back(std::get<0>(obj));
    // } else if (obj.index() == 1) {
    //     std::get<1>(objects_1).push_back(std::get<1>(obj));
    // } else if (obj.index() == 2) {
    //     std::get<2>(objects_1).push_back(std::get<2>(obj));
    // }

    // static_for_1<0, 3>([&](auto ic) {
    //     if (obj.index() == ic.value)
    //     std::get<ic.value>(objects_1).push_back(std::get<ic.value>(obj));
    // });

    static_for_2<std::variant_size_v<decltype(obj)>>([&](auto ic) {
        if (obj.index() == ic.value)
        std::get<ic.value>(objects_1).push_back(std::get<ic.value>(obj));
    });
}

void print_objects_2() {
    // for (auto &obj : std::get<0>(objects_1)) {
    //     std::cout << "int: " << obj << std::endl;
    // }
    // for (auto &obj : std::get<1>(objects_1)) {
    //     std::cout << "double: " << obj << std::endl;
    // }
    // for (auto &obj : std::get<2>(objects_1)) {
    //     std::cout << "string: " << obj << std::endl;
    // }

    // static_for_1<0, 3>([&](auto ic) {
    //     for (auto &obj : std::get<ic.value>(objects_1)) {
    //         std::cout << obj << std::endl;
    //     }
    // });

    static_for_2<std::tuple_size_v<decltype(objects_1)>>([&](auto ic) {
        for (auto &obj : std::get<ic.value>(objects_1)) {
            std::cout << obj << std::endl;
        }
    });
}

TEST(StaticForTest, Example_1_3) {
    add_object_2(42);
    add_object_2(3.14);
    add_object_2("hello");
    print_objects_2();
}

using Object = std::variant<int, double, std::string>;
// 通过Object自动生成tuple对象，可以利用之前学习到的type_conversion中 tuple_apply,tuple_map结合使用

template <class V>
struct variant_to_tuple_of_vector {};

template <class... Ts>
struct variant_to_tuple_of_vector<std::variant<Ts...>> {
    using type = std::tuple<std::vector<Ts>...>;
};

using ObjectList = variant_to_tuple_of_vector<Object>::type;
static ObjectList objects_2;

void add_object_3(Object obj) {
    static_for_2<std::variant_size_v<Object>>([&](auto ic) {
        if (obj.index() == ic.value)
        std::get<ic.value>(objects_2).push_back(std::get<ic.value>(obj));
    });
}

void print_objects_3() {
    static_for_2<std::variant_size_v<Object>>([&](auto ic) {
        for (auto &obj : std::get<ic.value>(objects_2)) {
            std::cout << obj << std::endl;
        }
    });
}

TEST(StaticForTest, Example_1_4) {
    add_object_3(42);
    add_object_3(3.14);
    add_object_3("hello");
    print_objects_3();
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
