#include "gtest_prompt.h"
#include "cppdemangle.h"
#include <iostream>

TEST(PtrTest, Unique_ptr) {
    std::unique_ptr<int> p(new int);
    auto x = p.get();

    std::unique_ptr<int> p2 = std::move(p);
    EXPECT_EQ(nullptr, p.get());
    EXPECT_EQ(x, p2.get()); 

    std::unique_ptr<int> p3 = static_cast<std::unique_ptr<int> &&>(p2);
    EXPECT_EQ(nullptr, p2.get());
    EXPECT_EQ(x, p3.get());
}

TEST(PtrTest, Shared_ptr) {
    std::shared_ptr<int> p(new int);
    auto x = p.get();
    EXPECT_EQ(1, p.use_count());
    
    std::shared_ptr<int> p2 = p;
    EXPECT_EQ(x, p2.get());
    EXPECT_EQ(2, p.use_count());
    EXPECT_EQ(p.use_count(), p2.use_count());
   
    std::shared_ptr<int> p3 = p2;
    EXPECT_EQ(x, p3.get());
    EXPECT_EQ(3, p.use_count());
    EXPECT_EQ(p.use_count(), p2.use_count());
    EXPECT_EQ(p3.use_count(), p2.use_count());
   
    p.reset();
    EXPECT_EQ(nullptr, p.get());
    EXPECT_EQ(p3.get(), p2.get());
    EXPECT_EQ(2, p2.use_count());
    EXPECT_EQ(p3.use_count(), p2.use_count());
}

TEST(PtrTest, Weak_ptr) {
    std::shared_ptr<int> p(new int);
    auto x = p.get();
    EXPECT_EQ(1, p.use_count());
    
    std::shared_ptr<int> p2 = p;
    EXPECT_EQ(x, p2.get());
    EXPECT_EQ(2, p.use_count());
    EXPECT_EQ(p.use_count(), p2.use_count());

    std::weak_ptr<int> wp = p2;
    EXPECT_EQ(x,  wp.lock().get());
    EXPECT_EQ(2, wp.use_count());
    
    p.reset();
    EXPECT_EQ(nullptr,  p.get());
    EXPECT_EQ(0, p.use_count());
    EXPECT_EQ(1, p2.use_count());
    EXPECT_EQ(wp.use_count(), p2.use_count());
   
    p2.reset();
    EXPECT_EQ(nullptr,  p2.get());
    EXPECT_EQ(0, p2.use_count());
    EXPECT_EQ(wp.use_count(), p2.use_count());
}

TEST(PtrTest, Weak_ptr1) {
    struct Parent {
        std::shared_ptr<struct Child> child;
    };
    struct Child {
        std::weak_ptr<Parent> parent;
    };
    std::shared_ptr<Parent> parent = std::make_shared<Parent>();
    std::shared_ptr<Child> child = std::make_shared<Child>();

    parent->child = child;
    child->parent = parent;
    EXPECT_TRUE(child->parent.lock());
    // 父对象被重置时，子对象的弱引用失效
    parent.reset();
    EXPECT_FALSE(child->parent.lock());
    child.reset();
}

namespace life_cycle {
struct Test {
    std::string m_name;

    explicit Test(std::string name = "") : m_name(std::move(name)) {
        printf("Test() %s\n", m_name.c_str());
    }

    Test(Test const &other) : m_name(other.m_name) {
        m_name += "_copy";
        printf("Test(Test const &) %s\n", m_name.c_str());
    }

    Test &operator=(Test const &other) {
        m_name = other.m_name;
        m_name += "_copy";
        printf("Test &operator=(Test const &) %s\n", m_name.c_str());
        return *this;
    }

    Test(Test &&other) noexcept : m_name(other.m_name) {
        other.m_name = "null";
        m_name += "_move";
        printf("Test(Test &&) %s\n", m_name.c_str());
    }

    Test &operator=(Test &&other) noexcept {
        if (this != &other) {
            m_name += "_move";
            printf("Test &operator=(Test &&) %s\n", m_name.c_str());
            m_name = other.m_name;
        }
        return *this;
    }

    ~Test() noexcept {
        printf("~Test() %s\n", m_name.c_str());
    }
};
} // namespace life_cycle

TEST(LifeCycleTest, defaultctor) {
    life_cycle::Test t("t");
}

void func(life_cycle::Test t) {
    puts("func(Test t)");
}

void funccref(life_cycle::Test const &t) {
    puts("funccref(Test const &t)");
}

TEST(LifeCycleTest, copyctor) {
    {
        life_cycle::Test t("t");
        func(t);
    }
    puts("--------------------");
    {
        life_cycle::Test t("t");
        life_cycle::Test t_copy(t);
        funccref(t_copy);
    }
}

TEST(LifeCycleTest, copyassignment) {
    {
        life_cycle::Test t1("t1");
        life_cycle::Test t2("t2");
        t2 = t1;
    }
    puts("--------------------");
    {
        life_cycle::Test t1("t1");
        life_cycle::Test t2 = t1;  // 等价于 Test t2(t1) 拷贝构造
    }

}

TEST(LifeCycleTest, movector) {
    {
        life_cycle::Test t("t");
        func(std::move(t));
    }
    puts("--------------------");
    {
        life_cycle::Test t("t");
        life_cycle::Test t_move(std::move(t));
        funccref(t_move);
    }
}

TEST(LifeCycleTest, moveassignment) {
    {
        life_cycle::Test t1("t1");
        life_cycle::Test t2("t2");
        t2 = std::move(t1);
    }
    puts("--------------------");
    {
        life_cycle::Test t1("t1");
        life_cycle::Test t2 = std::move(t1); //  // 等价于 Test t2(std::move(t1)) 移动构造
    }
}

void func(std::unique_ptr<life_cycle::Test> t) {
    puts("func(std::unique_ptr<Test> t)");
}

TEST(LifeCycleTest, Unique_ptr) {
    std::unique_ptr<life_cycle::Test> t(new life_cycle::Test("t"));
    func(std::move(t));
}

void funccref(life_cycle::Test &&t) {
    puts("funccref(Test &&t)");
}

TEST(LifeCycleTest, refTest) {
    life_cycle::Test t("t");
    funccref(std::move(t));
    funccref(t);
    funccref(std::as_const(t));
}

void func2_1(life_cycle::Test &&t) {
    funccref(std::move(t));
}

void func2_1(life_cycle::Test const &t) {
    funccref(t);
}

TEST(LifeCycleTest, func2_1Test) {
    life_cycle::Test t("t");
    func2_1(std::move(t));
    func2_1(t);
}

template <typename T>
void func2_2(T &&t) {
    printf("T = %s \n", cppdemangle<T>().c_str());
    if constexpr (std::is_lvalue_reference_v<T>) {
         funccref(t);
    }
    else {
        funccref(std::move(t));
    }
}

TEST(LifeCycleTest, func2_2Test) {
    life_cycle::Test t("t");
    func2_2(std::move(t));
    func2_2(t);
}

template <typename T>
void func2_3_1(T &&t) {
    // printf("T = %s \n", cppdemangle(typeid(T).name()).c_str()); // 不会考虑引用类型,只处理了 typeid(T).name() 返回的原始类型名称
    printf("T = %s \n", cppdemangle<T>().c_str());
}

TEST(LifeCycleTest, func2_3_1Test) {
    life_cycle::Test t("t");
    func2_3_1(std::move(t));
    func2_3_1(t);
}

template <typename T>
void func2_3_2(T &&t) {
    printf("T = %s \n", cppdemangle<T>().c_str());
    funccref(std::forward<decltype(t)>(t));
}

TEST(LifeCycleTest, func2_3_2Test) {
    life_cycle::Test t("t");
    func2_3_2(std::move(t));
    func2_3_2(t);
}

void funccref(int i, int j) {
    puts("funccref(int i, int j)");
}

template <typename ...T>
void func2_4(T &&...t) {
    funccref(std::forward<T>(t)...);
}

TEST(LifeCycleTest, func2_4Test) {
    life_cycle::Test t("t");
    func2_4(std::move(t));
    func2_4(t);
    func2_4(3, 4);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}