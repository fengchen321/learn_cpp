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

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
