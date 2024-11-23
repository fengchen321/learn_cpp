#include "gtest_prompt.h"
#define STL_STANDARD 0
#if STL_STANDARD
#include <optional>
#define Optional std::optional
#define nullopt std::nullopt
#define inplace std::in_place
#else
#include "Optional.h"
#endif

struct C {
    C(int x, int y) :m_x(x), m_y(y){}
    C(C const &) { puts(__PRETTY_FUNCTION__);}

    C(C &&) { puts(__PRETTY_FUNCTION__);}

    C &operator=(C const &) {
        puts(__PRETTY_FUNCTION__);
        return *this;
    }

    C &operator=(C &&) {
        puts(__PRETTY_FUNCTION__);
        return *this;
    }

    ~C() { puts(__PRETTY_FUNCTION__); }

    int value() const {
        return m_x;
    }
    int m_x;
    int m_y;
};

TEST(OptionalTest, ValueThrowsBadOptionalAccess) {
    Optional<C> opt(nullopt);
    ASSERT_THROW_EXCEPTION(opt.value().m_x, "bad optional access");
}

TEST(OptionalTest, HasValue) {
    Optional<int> opt(1);
    EXPECT_EQ(opt.value(), 1);
    EXPECT_TRUE(opt.has_value());
}

TEST(OptionalTest, ValueConstRef) {
    const Optional<int> opt(42);
    EXPECT_EQ(opt.value(), 42);
}

TEST(OptionalTest, ValueRef) {
    Optional<int> opt(42);
    EXPECT_EQ(opt.value(), 42);
}

TEST(OptionalTest, ValueConstRvalue) {
    const Optional<int> opt(42);
    EXPECT_EQ(std::move(opt).value(), 42);
}

TEST(OptionalTest, ValueRvalue) {
    Optional<int> opt(42);
    EXPECT_EQ(std::move(opt).value(), 42);
}

TEST(OptionalTest, ValueOrConstRef) {
    const Optional<int> opt(42);
    EXPECT_EQ(opt.value_or(10), 42);

    const Optional<int> opt1;
    EXPECT_EQ(opt1.value_or(10), 10);
}

TEST(OptionalTest, ValueOrRvalueWithValue) {
    Optional<int> opt(42);
    EXPECT_EQ(std::move(opt).value_or(10), 42);

    Optional<int> opt1;
    EXPECT_EQ(std::move(opt1).value_or(10), 10);
}

TEST(OptionalTest, CopyConstructor) {
    Optional<int> opt1(42);
    Optional<int> opt2(opt1);
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);

    Optional<int> opt3;
    Optional<int> opt4(opt3);
    EXPECT_FALSE(opt4.has_value());
}

TEST(OptionalTest, MoveConstructor) {
    Optional<int> opt1(42);
    Optional<int> opt2(std::move(opt1));
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);

    Optional<int> opt3;
    Optional<int> opt4(std::move(opt3));
    EXPECT_FALSE(opt4.has_value());
}

TEST(OptionalTest, InPlaceConstructor) {
    Optional<int> opt(inplace, 42);
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 42);
}

TEST(OptionalTest, InPlaceConstructorWithInitializerList) {
    Optional<std::vector<int>> opt(inplace, {1, 2, 3});
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), std::vector<int>({1, 2, 3}));
}

TEST(OptionalTest, AssignNullopt) {
    Optional<int> opt(42);
    opt = nullopt;
    EXPECT_FALSE(opt.has_value());

    Optional<int> opt1;
    opt1 = nullopt;
    EXPECT_FALSE(opt1.has_value());
}
TEST(OptionalTest, AssignRvalue) {
    Optional<int> opt(42);
    opt = 10;
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 10);

    Optional<int> opt1;
    opt1 = 10;
    EXPECT_TRUE(opt1.has_value());
    EXPECT_EQ(opt1.value(), 10);
}


TEST(OptionalTest, AssignConstReference) {
    const int value = 10;
    Optional<int> opt(42);
    opt = value;
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 10);

    Optional<int> opt1;
    opt1 = value;
    EXPECT_TRUE(opt1.has_value());
    EXPECT_EQ(opt1.value(), 10);
}


TEST(OptionalTest, CopyAssignment) {
    Optional<int> opt1(42);
    Optional<int> opt2;
    opt2 = opt1;
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);

    Optional<int> opt3;
    Optional<int> opt4(42);
    opt4 = opt3;
    EXPECT_FALSE(opt4.has_value());
}

TEST(OptionalTest, MoveAssignment) {
    Optional<int> opt1(42);
    Optional<int> opt2;
    opt2 = std::move(opt1);
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);
    EXPECT_TRUE(opt1.has_value());

    Optional<int> opt3;
    Optional<int> opt4(42);
    opt4 = std::move(opt3);
    EXPECT_FALSE(opt3.has_value());
    EXPECT_FALSE(opt4.has_value());
}

TEST(OptionalTest, Emplace) {
    Optional<int> opt;
    opt.emplace(42);
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 42);

    Optional<int> opt1(42);
    opt1.emplace(10);
    EXPECT_TRUE(opt1.has_value());
    EXPECT_EQ(opt1.value(), 10);

    Optional<C> optc = nullopt;
    // optc = C(1, 2); // 1. C(int x, int y); 2. optc.operator=(C &&value) {new (&m_value) T(std::move(vale));
    // C::C(C&&) -> C::~C() -> C::~C()
    optc.emplace(1, 2); // 1. optc.emplace(int x, int y) {new (&m_value) T(x, y);}
}

TEST(OptionalTest, EmplaceWithInitializerList) {
    Optional<std::vector<int>> opt;
    opt.emplace({1, 2, 3});
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), std::vector<int>({1, 2, 3}));
}

TEST(OptionalTest, Reset) {
    Optional<int> opt(42);
    opt.reset();
    EXPECT_FALSE(opt.has_value());

    Optional<int> opt1;
    opt1.reset();
    EXPECT_FALSE(opt1.has_value());
}

TEST(OptionalTest, SwapWithValue) {
    Optional<int> opt1(42);
    Optional<int> opt2(10);
    opt1.swap(opt2);
    EXPECT_TRUE(opt1.has_value());
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt1.value(), 10);
    EXPECT_EQ(opt2.value(), 42);
}

TEST(OptionalTest, SwapWithOneValue) {
    Optional<int> opt1(42);
    Optional<int> opt2;
    opt1.swap(opt2);
    EXPECT_FALSE(opt1.has_value());
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 42);
}

TEST(OptionalTest, SwapWithoutValue) {
    Optional<int> opt1;
    Optional<int> opt2;
    opt1.swap(opt2);
    EXPECT_FALSE(opt1.has_value());
    EXPECT_FALSE(opt2.has_value());
}

TEST(OptionalTest, DereferenceConstRef) {
    const Optional<int> opt(42);
    EXPECT_EQ(*opt, 42);
}

TEST(OptionalTest, DereferenceRef) {
    Optional<int> opt(42);
    EXPECT_EQ(*opt, 42);

    Optional<C> optc = nullopt;
    optc.emplace(1, 2);
    EXPECT_EQ((*optc).m_x, 1);
    EXPECT_EQ(optc->m_y, 2);
}

TEST(OptionalTest, DereferenceConstRvalue) {
    const Optional<int> opt(42);
    EXPECT_EQ(*std::move(opt), 42);
}

TEST(OptionalTest, DereferenceRvalue) {
    Optional<int> opt(42);
    EXPECT_EQ(*std::move(opt), 42);
}

TEST(OptionalTest, MemberAccessConst) {
    const Optional<std::string> opt("hello");
    EXPECT_EQ(opt->size(), 5);
}

TEST(OptionalTest, MemberAccess) {
    Optional<std::string> opt("hello");
    EXPECT_EQ(opt->size(), 5);
}

TEST(OptionalTest, BoolOperator) {
    Optional<int> opt(42);
    EXPECT_TRUE(static_cast<bool>(opt));
    opt.reset();
    EXPECT_FALSE(static_cast<bool>(opt));
}

TEST(OptionalTest, EqualityandInequalityWithNullopt) {
    Optional<int> opt1;
    Optional<int> opt2(42);
    EXPECT_TRUE(opt1 == nullopt);
    EXPECT_TRUE(nullopt == opt1);
    EXPECT_FALSE(opt2 == nullopt);
    EXPECT_FALSE(nullopt == opt2);
    EXPECT_FALSE(opt1 != nullopt);
    EXPECT_FALSE(nullopt != opt1);
    EXPECT_TRUE(opt2 != nullopt);
    EXPECT_TRUE(nullopt != opt2);
}

TEST(OptionalTest, EqualityandInequalityWithValue) {
    Optional<int> opt1(42);
    Optional<int> opt2(42);
    Optional<int> opt3(10);
    EXPECT_TRUE(opt1 == opt2);
    EXPECT_FALSE(opt1 == opt3);
    EXPECT_FALSE(opt1 != opt2);
    EXPECT_TRUE(opt1 != opt3);
}

TEST(OptionalTest, GreaterThan) {
    Optional<int> opt1(42);
    Optional<int> opt2(10);
    EXPECT_TRUE(opt1 > opt2);
    EXPECT_FALSE(opt2 > opt1);
}

TEST(OptionalTest, LessThan) {
    Optional<int> opt1(42);
    Optional<int> opt2(10);
    EXPECT_FALSE(opt1 < opt2);
    EXPECT_TRUE(opt2 < opt1);
}

TEST(OptionalTest, GreaterThanOrEqual) {
    Optional<int> opt1(42);
    Optional<int> opt2(42);
    Optional<int> opt3(10);
    EXPECT_TRUE(opt1 >= opt2);
    EXPECT_TRUE(opt1 >= opt3);
    EXPECT_FALSE(opt3 >= opt1);
}

TEST(OptionalTest, LessThanOrEqual) {
    Optional<int> opt1(42);
    Optional<int> opt2(42);
    Optional<int> opt3(10);
    EXPECT_TRUE(opt1 <= opt2);
    EXPECT_FALSE(opt1 <= opt3);
    EXPECT_TRUE(opt3 <= opt1);
}
#if __cpp_deduction_guides
TEST(OptionalTest, CTAD) {
    Optional opt1(42);
    EXPECT_TRUE(opt1.has_value());
    EXPECT_EQ(opt1.value(), 42);
}
#endif
#if !STL_STANDARD
TEST(OptionalTest, MakeOptional) {
    auto opt = makeOptional(42);
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 42);
}
#endif
// Monadic operations 
#if !STL_STANDARD || (STL_STANDARD && __cplusplus >= 202302L)
TEST(OptionalTest, AndThen) {
    Optional<int> opt(42);
    auto result = opt.and_then([](int value) { return Optional<int>(value * 2); });
    EXPECT_EQ(result, 84);

    Optional<int> opt1;
    auto result1 = opt1.and_then([](int value) { return Optional<int>(value * 2); });
    EXPECT_EQ(result1, nullopt);
}

TEST(OptionalTest, Transform) {
    Optional<int> opt(42);
    auto result = opt.transform([](int value) { return value * 2; });
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 84);

    Optional<int> opt1;
    auto result1 = opt1.transform([](int value) { return value * 2; });
    EXPECT_FALSE(result1.has_value());
}

TEST(OptionalTest, OrElse) {
    Optional<int> opt(42);
    auto result = opt.or_else([]() { return Optional<int>(10); });
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);

    Optional<int> opt1;
    auto result1 = opt1.or_else([]() { return Optional<int>(10); });
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), 10);
}
#endif
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}