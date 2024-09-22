#include "gtest_prompt.h"
#include "template_SFINAE.h"

TEST(FuncTest, DiffInput) {
    int i = 0;
    EXPECT_EQ(func(i), 1);
    double d = 0.0;
    EXPECT_EQ(func(d), 1.0);
    std::string s = "hello";
    EXPECT_EQ(func(s), "hello world");
}

TEST(InvokeTest, ReturnTypeInt) {
    int result = invoke([]() -> int {
        return 42;
    });
    EXPECT_EQ(result, 42);
}

TEST(InvokeTest, ReturnTypeVoid) {
    bool called = false;
    invoke([&]() -> void {
        called = true;
    });
    EXPECT_TRUE(called);
}

TEST(SeparateInvokeTest, ReturnTypeInt) {
    int result = separate_invoke([]() -> int {
        return 42;
    });
    EXPECT_EQ(result, 42);
}

TEST(SeparateInvokeTest, ReturnTypeVoid) {
    bool called = false;
    separate_invoke([&]() -> void {
        called = true;
    });
    EXPECT_TRUE(called);
}

struct mystudent {
    void dismantle() {
        printf("%s\n", "student dismantle");
    }
    void rebel(int i) {
        printf("%s%d\n", "student rebel ", i);
    }
};
struct myteacher {
    void rebel(int i) {
        printf("%s%d\n", "teacher rebel ", i);
    }
};
struct myclass {
    void dismantle() {
        printf("%s\n", "class dismantle");
    }
};
struct myvoid {};

TEST(JudgeMemberFunc, Gench) {
    mystudent s;
    myteacher t;
    myclass c;
    myvoid v;
    ASSERT_LOGS_STDOUT(gench(s), "student dismantle");
    ASSERT_LOGS_STDOUT(gench(t), "teacher rebel 1\nteacher rebel 2\nteacher rebel 3\nteacher rebel 4");
    ASSERT_LOGS_STDOUT(gench(c), "class dismantle");
    ASSERT_LOGS_STDOUT(gench(v), "no any method supported!");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}