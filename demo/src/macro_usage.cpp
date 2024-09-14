//
// Created by lsfco on 2024/9/7.
//
#include <iostream>
#include "prompt.h"
/*
 * # 字符串化和 ##连接符
 */
#define ERR_FILE_NOT_FOUND 1
#define ERR_INVALID_PARAMETER 2
#define ERR_OUT_OF_MEMORY 3

// 宏定义，用于生成错误码处理
#define ERRCODE(x) case ERR_##x: return #x;

// 错误处理函数
const char* errorToString(int errorCode) {
    switch (errorCode) {
        ERRCODE(FILE_NOT_FOUND)
        ERRCODE(INVALID_PARAMETER)
        ERRCODE(OUT_OF_MEMORY)
        default: return "Unknown error";
    }
}

/*
 * 显示行号和文件名
 */
#define GetError() 1
void check_error(const char *filename, int lineno, const char *expr) {
    auto err = GetError();
    if(err != 0) {
        std::cerr << filename << ":" << lineno << ": " << expr << " failed: " << err << std::endl;
        std::terminate();
    }
}
#if NDEBUG
#define CHECK_CODE(x) do {               \
    (x);                                 \
}   while (0)
#else
#define CHECK_CODE(x) do {               \
    (x);                                 \
    check_error(__FILE__, __LINE__, #x); \
}   while (0)
#endif
int test(int i) {
    return 5;
}

/*
 * assert
 */
#define ASSERT(x) do {                                \
    if (!(x)) {                                       \
        std::cerr << "Assert failed: " << #x << "\n"; \
        std::terminate();                             \
    }                                                 \
}   while (0)
#if NDEBUG
#define ASSERT_GT(x, y)
#else
#define ASSERT_GT(x, y) do {                          \
    decltype(x) __x = x;                              \
    decltype(y) __y = y;                              \
    if (!((__x) > (__y))) {                           \
        std::cerr << "Assert failed: " << #x <<       \
        " (" << (__x) << ")" << " > " << #y << "\n";  \
        std::terminate();                             \
    }                                                 \
}   while (0)
#endif

/*
 * 跨平台、版本、编译器使用优化
 */
#if defined(__linux__)
#elif defined(__APPLE__)
#elif defined(__WIN32__)
#else
#endif

#if __cplusplus >= 202002L
template <class T>
requires(std::is_signed_v<T>)
#else
template <class T, std::enable_if_t<std::is_signed_v<T>, int> = 0>
#endif
void foo(T i) {
    std::cout << i << std::endl;
}

#if defined(__MSC_VER)
#define NOINLINE __declspec(noinline)
#define LIKELY(x) __assume(x)
#define UNLIKELY(x) __assume(!x)
#elif defined(__GNUC__) || defined(__clang__)
#define NOINLINE __attribute__((noinline))
#define LIKELY(x) (__builtin_expect(!!(x), 1))
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))  // !!可以把任意类型变为bool  c++20: [[unlikely]]
#else
#define NOINLINE
#define LIKELY(x)
#define UNLIKELY(x)
#endif

NOINLINE void foo() {
    std::cout << "foo" << std::endl;
}

/*
 * print 变参打印  c++20: __VA_OPT__
 */
#define STR2(X) #X
#define STR(X) STR2(X)
#if __cplusplus >= 202002L
#define PRINT(X, ...) do {                                                \
    printf(__FILE__ ":" STR(__LINE__) ": " X  __VA_OPT__(,) __VA_ARGS__); \
} while (0)
#else
#define PRINT(X, ...) do {                                    \
    printf(__FILE__ ":" STR(__LINE__) ": " X, ##__VA_ARGS__); \
} while (0)
#endif

int main() {
    display_cpp_version();
    int code = ERR_INVALID_PARAMETER;
    printf("Error code %d: %s\n", code, errorToString(code));
//    ASSERT(0 == 2);
//    CHECK_CODE(test(6));
//    int i = 1;
//    ASSERT_GT(i << 1 , 2);
//    ASSERT_GT(i++ , 1);
    foo(1);
//    foo();
//    PRINT("hello\n");
//    PRINT("hello %d + %d ", 9, 2);
    return 0;
}