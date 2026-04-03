#include "gtest_prompt.h"
#define _GLIBCXX_DEBUG
/*
* 空指针类
*/
TEST(UNDefTEST, nullpointer) {
    int *x = nullptr;
    // *x;          //未定义行为
    // &*x;         //未定义行为
    // *x = 0;      //未定义行为
    // int i = *x;  //未定义行为

    std::unique_ptr<int> p = nullptr;
    // std::cout << &*p << std::endl; //未定义行为
    std::cout << "p = " << p.get() << std::endl; // 合理
}

TEST(UNDefTEST, end_iteration) {
    std::vector<int> v = {1, 2, 3, 4};
    int *begin = &*v.begin();
    int *end = &*v.end(); //未定义行为

    v = {};
    begin = &*v.begin(); //未定义行为
    end = &*v.end();     //未定义行为
    // 合理
    v = {1, 2, 3, 4};
    begin = v.data();
    end = v.data() + v.size();
}

TEST(UNDefTEST, this_ptr) {
    struct C {
        void print() {
            if (this == nullptr) { // this 指针不能为空, 可能会被优化为 if (0)
                std::cout << "this is nullptr\n";
            }
        }
    };
    auto func = []() {
        C *c = nullptr;
        c->print(); // 未定义行为 (*c).print() 空指针不能调用成员函数
    };
    func(); 
}

/*
* 指针别名类
*/

// reinterpret_cast 后以不兼容的类型访问
TEST(UNDefTEST, reinterpret_cast) {
    int i;
    float f = *(float *)&i; // 未定义行为

    // char signed char、unsigned char 和 std::byte 总是兼容任何类型
    // int 和 unsigned int 互相兼容

    char *buf = (char *)&i; // 可以
    buf[0] = 1;             // 可以
    unsigned int u_i = *(unsigned int *)&i; // 可以
}

// union 访问不是激活的成员
#if __cplusplus >= 202002L
#include <bit>
#endif
TEST(UNDefTEST, union) {
    auto bitCast = [](int i) {
        union {
            int i;
            float f;
        } u;
        u.i = i;
        return u.f; // 未定义行为
    };
    std::cout << bitCast(2) << std::endl;

    // 在 float 和 int 之间按位转换合理方式
    auto bitCast_1 = [](int i) {
        float f;
        memcpy(&f, &i, sizeof(i));
        return f;
    };
    std::cout << bitCast_1(2) << std::endl;
#if __cplusplus >= 202002L
    auto bitCast_2 = [](int i) {
        float f = std::bit_cast<float>(i);
        return f;
    };
    std::cout << bitCast_2(2) << std::endl;
#endif
}

// T 类型指针必须对齐到 alignof(T)
TEST(UNDefTEST, alignof) {
    struct alignas(64) C {
        int i;
        char c;
    };

    C *p = (C *)malloc(sizeof(C)); // 错！malloc 产生的指针只保证对齐到 max_align_t（GCC 上是 16 字节）大小，并不保证对齐到 C 所需的 64 字节
    C *x = new C;  // 可以，new T 总是保证对齐到 alignof(T)
}

// 从父类 static_cast 到不符合的子类后访问
TEST(UNDefTEST, static_cast) {
    struct Base {};
    struct Derived : Base {};
    {
        Base b;
        Derived d1 = *(Derived *)&b;              // 错！
        Derived d2 = *static_cast<Derived *>(&b); // 错！
        Derived d3 = static_cast<Derived &>(b);   // 错！
    }

    {
        Derived obj;
        Base *bp = &obj;
        Derived d1 = *(Derived *)bp;              // 可以
        Derived d2 = *static_cast<Derived *>(bp); // 可以
        Derived d3 = static_cast<Derived &>(*bp); // 可以
    }
}

// bool 类型不得出现 0 和 1 以外的值
TEST(UNDefTEST, bool) {
    char c = 2;
    bool b = *(bool *)&c; // 未定义行为
}

/*
* 算数类
*/
// 有符号整数的加减乘除模不能溢出
#include <climits>
TEST(UNDefTEST, overflow) {
    int i = INT_MAX;
    i += 1; // 未定义行为
    EXPECT_EQ(i, INT_MIN); // x86

    unsigned int u_i = UINT_MAX;
    u_i += 1; // 合理
    EXPECT_EQ(u_i, 0);
}
// shift 操作符的右操作数必须小于等于左操作数的位数
TEST(UNDefTEST, shift) {
    int i = 1 << 32; // 未定义行为
    std::cout << i << std::endl;
}
// 除以 0
TEST(UNDefTEST, divide_zero) {
    // int i = 1 / 0; // 未定义行为
}

/*
*函数类
*/
// 返回类型不为 void 的函数，必须有 return 语句
TEST(UNDefTEST, void_return) {
    auto func = []() -> int {
        int i = 42;
    };
    // 开启-Werror=return-type 选项，将不写返回语句的警告转化为错误
    EXPECT_DEATH(func(), ".*");
}

// 函数指针被调用时，不能为空
static void func() {
    printf("func called\n");
}
typedef void (*func_t)();
static func_t fp = nullptr;
extern void set_fp() {
    fp = func;
}
TEST(UNDefTEST, null_funcptr) {
    EXPECT_DEATH(fp(), ".*");// 未定义行为
}

/*
* 生命周期类
*/
// 局部变量在初始化之前不能使用
TEST(UNDefTEST, local_var) {
    int i;
    std::cout << i << std::endl; // 未定义行为
}
// 指针的加减法不能超越数组边界
TEST(UNDefTEST, ptr_add) {
    int arr[3] = {1, 2, 3};
    int *p = &arr[0];
    p + 1;     // 可以
    p + 3;    // 可以
    std::cout << *p + 3 << std::endl; // 未定义行为
    std::cout << *p + 4 << std::endl; // 未定义行为
}
// 可以有指向数组尾部的指针（类似 end 迭代器），但不能解引用
TEST(UNDefTEST, ptr_end) {
    int arr[10];
    int *p = &arr[0];
    int *end = p + 10; // 可以
    std::cout << *end << std::endl; // 未定义行为
}
// 不能访问未初始化的指针
TEST(UNDefTEST, ptr_undef) {
    struct Dog {
        int age;
    };

    struct Person {
        Dog *dog;
    };
    Person *p = new Person;
    EXPECT_DEATH({ std::cout << p->dog->age << std::endl; }, ".*");

    p->dog = new Dog;
    std::cout << p->dog->age << std::endl;;
}

// 不能访问已释放的内存
TEST(UNDefTEST, ptr_free) {
    int *p = new int;
    std::cout << *p << std::endl;
    delete p;
    *p; // 未定义行为
    std::cout << *p << std::endl;
}

// new / new[] / malloc 和 delete / delete[] / free 必须匹配
TEST(UNDefTEST, new_delete) {
    int *p = new int[3];
    // delete p; // 错
    delete[] p; // 正确
}

// 不要访问已经析构的对象
TEST(UNDefTEST, destructed) {
    struct C {
        int i;
        ~C() { i = 0; }
    };

    C *c = (C *)malloc(sizeof(C));
    std::cout << c->i << std::endl;; // 可以
    c->~C();
    std::cout << c->i << std::endl;; // 未定义行为
    free(c);
}

// 不能把函数指针转换为普通类型指针解引用
void func1() {}

TEST(UNDefTEST, funcptr) {
    printf("*func1 = %d\n", *((int *)func1)); 
}

/*
* 库函数类
*/
// ctype.h 中一系列函数的字符参数，必须在 0~127 范围内（即只支持 ASCII 字符）
TEST(UNDefTEST, ctype) {
    std::cout << isdigit('\xef') << std::endl; // 未定义行为
    std::cout << iswdigit('\xef') << std::endl; 
}

// memcpy 函数的 src 和 dst 不能为空指针
TEST(UNDefTEST, memcpy) {
    void *dst = nullptr;
    void *src = nullptr;
    size_t size = 0;
    memcpy(dst, src, size); // 未定义行为
}
// memcpy 函数的 src 和 dst 不能重叠
TEST(UNDefTEST, memcpy_overlap) {
    char arr[10];
    memcpy(arr, arr + 1, 9); // 未定义行为
    // 需拷贝带重复区间的内存，可以用 memmove
    memmove(arr, arr + 1, 9);
}
// v.back() 当 v 为空时是未定义行为
TEST(UNDefTEST, back) {
    std::vector<int> v;
    // int i = v.back(); // 未定义行为
    // back() 并不会对 v 是否有最后一个元素做检查，此处相当于解引用了越界的指针
    int i = v.empty() ? 0 : v.back(); // 正确
}

// vector 的 operator[] 当 i 越界时
TEST(UNDefTEST, at) {
    std::vector<int> v = { 1, 2, 3 };
    // v[3]; // 错！相当于解引用了越界的指针
    ASSERT_THROW(v.at(3), std::out_of_range);
}

// 容器迭代器失效
TEST(UNDefTEST, vector_iter) {
    std::vector<int> v = { 1, 2, 3 };
    auto it = v.begin();
    v.push_back(4); // push_back 可能导致扩容，会使之前保存的 v.begin() 迭代器失效
    // *it = 0;        // 未定义行为
}

TEST(UNDefTEST, map_iter) {
    std::map<int, std::unique_ptr<int>> map_info;
    map_info[1] = std::make_unique<int>(10);
    map_info[2] = nullptr;
    map_info[3] = std::make_unique<int>(30);

    // 未定义行为：删除元素导致迭代器失效
    for (auto itor = map_info.begin(); itor != map_info.end(); itor++) {
        if (itor->second == nullptr) {
            map_info.erase(itor->first);
        }
    }

    // 正确的方式：使用 erase 的返回值来获取下一个有效的迭代器
    for (auto itor = map_info.begin(); itor != map_info.end(); ) {
        if (itor->second == nullptr) {
            itor = map_info.erase(itor);
        } else {
            ++itor;
        }
    }
}

// 容器元素引用失效
TEST(UNDefTEST, ref) {
    std::vector<int> v = {1, 2, 3};
    int &ref = v[0];
    v.push_back(4); // push_back 可能导致扩容，使元素全部移动到了新的一段内存，会使之前保存的 ref 引用失效
    // ref = 0;        // 未定义行为
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
