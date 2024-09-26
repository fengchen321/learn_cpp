#include "gtest_prompt.h"

TEST(TestFunction, CppFunction) {
    int x = 2;
    std::function<void()> f = [x]() { printf("Hello %d\n", x); };
    ASSERT_LOGS_STDOUT(f(), "Hello 2\n");
}

void func_hello() {
    printf("Hello\n");
}
void func_world() {
    printf("World\n");
}
typedef void (*func_ptr)();
void repeat_func(func_ptr func) {
    func();
    func();
}
// 无参函数，使用函数指针
TEST(TestFunction, CFunc) {
    ASSERT_LOGS_STDOUT(repeat_func(func_hello), "Hello\nHello\n");
    ASSERT_LOGS_STDOUT(repeat_func(func_world), "World\nWorld\n");
}

void func_num(void  *arg) {
    int x = *(int*)arg;
    printf("num is %d\n", x);
}
typedef void (*funcparms_ptr)(void *arg);
void repeat_func1(funcparms_ptr func, void *arg) {
    func(arg);
    func(arg);
}
// 单个参数函数，使用函数指针
TEST(TestFunction, CFuncParms) {
    int x = 2;
    ASSERT_LOGS_STDOUT(repeat_func1(func_num, &x), "num is 2\nnum is 2\n");
}
struct num_args_t {
    int x;
    int y;
};
void func_num2(void  *arg) {
    num_args_t *args = (num_args_t*)arg;
    printf("num is %d, %d\n", args->x, args->y);
}
// 通过结构体获取多个参数，使用函数指针
TEST(TestFunction, CFuncMultiParms) {
    num_args_t a = {2, 3};
    ASSERT_LOGS_STDOUT(repeat_func1(func_num2, &a), "num is 2, 3\nnum is 2, 3\n");
}

struct changeable_num_args_t {
    int &x;  // 用指针也行，但引用更方便，指针需要解引用
    int &y;
};

void func_num3(void  *arg) {
    changeable_num_args_t *args = (changeable_num_args_t*)arg;
    printf("num is %d, %d\n", args->x, args->y);
}
// 可变参数， 使用函数指针
TEST(TestFunction, CFuncChangeableMultiParms) {
    int x = 2, y = 3;
    changeable_num_args_t a{x, y};
    
    ASSERT_LOGS_STDOUT(repeat_func1(func_num3, &a), "num is 2, 3\nnum is 2, 3\n");
    x = 4;
    y = 5;
    ASSERT_LOGS_STDOUT(repeat_func1(func_num3, &a), "num is 4, 5\nnum is 4, 5\n");
}

struct num_args_t2 {
    void call() {
        printf("num is %d, %d\n", x, y);
    }
    int &x;
    int &y;
};

template <class Fn>
void repeat_func2(Fn func) {
    func.call();
    func.call();
}

TEST(TestFunction, CppMultiParms) {
    int x = 2, y = 3;
    num_args_t2 a{x, y};
    
    ASSERT_LOGS_STDOUT(repeat_func2(a), "num is 2, 3\nnum is 2, 3\n");
    x = 4;
    y = 5;
    ASSERT_LOGS_STDOUT(repeat_func2(a), "num is 4, 5\nnum is 4, 5\n");
}

struct func_num_t {
    void operator()() const{
        printf("num is %d, %d\n", x, y);
    }
    int &x;
    int &y;
};

template <class Fn>
void repeat_func3(Fn const &func) {
    func();
    func();
}

TEST(TestFunction, CppMultiParms2) {
    int x = 2, y = 3;
    func_num_t a{x, y};

    ASSERT_LOGS_STDOUT(repeat_func3(a), "num is 2, 3\nnum is 2, 3\n");
    x = 4;
    y = 5;
    ASSERT_LOGS_STDOUT(repeat_func3(a), "num is 4, 5\nnum is 4, 5\n");
    ASSERT_LOGS_STDOUT(repeat_func3(func_hello), "Hello\nHello\n");
    // [&] [=] Test with a lambda capturing references
    ASSERT_LOGS_STDOUT(repeat_func3([&x, &y]() { 
        printf("num is %d, %d\n", x, y); 
    }), "num is 4, 5\nnum is 4, 5\n");
}

void repeat_func4(std::function<void()> const &func) {
    func();
    func();
}
// 使用std::function, 不用像之前使用模板，可以传入任何可调用对象
TEST(TestFunction, CppFunction1) {
    int x = 2, y = 3;
    func_num_t a{x, y};
    ASSERT_LOGS_STDOUT(repeat_func4(a), "num is 2, 3\nnum is 2, 3\n");
}

struct func_num_parms_t {
    void operator()(int i) const{
        printf("#%d num is %d, %d\n", i, x, y);
    }
    int &x;
    int &y;
};

void repeat_func5(std::function<void(int)> const &func) {
    func(1);
    func(2);
}

TEST(TestFunction, CppFunction2) {
    int x = 2, y = 3;
    func_num_parms_t a{x, y};
    ASSERT_LOGS_STDOUT(repeat_func5(a), "#1 num is 2, 3\n#2 num is 2, 3\n");

    ASSERT_LOGS_STDOUT(repeat_func5([&](int i) {
        printf("#%d num is %d, %d\n", i, x, y); 
    }), "#1 num is 2, 3\n#2 num is 2, 3\n");
}

#include "Functional.h"
void repeat_func6(Function<void()> const &func) {
    func();
    func();
}


TEST(TestFunction, CppFunction3) {
    int x = 2, y = 3;
    func_num_t a{x, y};
    ASSERT_OUTPUTS_EQUAL(repeat_func4(a), repeat_func6(a));
    ASSERT_OUTPUTS_EQUAL(repeat_func6([&x, &y]() { 
        printf("num is %d, %d\n", x, y); 
    }), repeat_func6([&x, &y]() { 
        printf("num is %d, %d\n", x, y); 
    }));
    ASSERT_OUTPUTS_EQUAL(repeat_func4(func_hello), repeat_func6(func_hello));
}

void repeat_func7(Function<void(int)> const &func) {
    func(1);
    func(2);
}


TEST(TestFunction, CppFunction4) {
    int x = 2, y = 3;
    func_num_parms_t a{x, y};
    ASSERT_OUTPUTS_EQUAL(repeat_func5(a), repeat_func7(a));
    ASSERT_OUTPUTS_EQUAL(repeat_func5([&](int i) {
        printf("#%d num is %d, %d\n", i, x, y); 
    }), repeat_func7([&](int i) {
        printf("#%d num is %d, %d\n", i, x, y); 
    }));
}

void repeat_func8(MoveOnlyFunction<void(int)> const &func) {
    func(1);
    func(2);
}

TEST(TestFunction, CppFunction5) {
    int x = 2, y = 3;
    func_num_parms_t a{x, y};
    repeat_func8(a);

    repeat_func8([&](int i) {
        printf("#%d num is %d, %d\n", i, x, y); 
    });
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
