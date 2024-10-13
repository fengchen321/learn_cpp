#include <iostream>
#include "gtest_prompt.h"

TEST(APIDesignTest, StructualParameter) {
    struct FooOptions {
        std::string name;
        int age;
        int phone;
        int address;
    };
    auto foo = [](FooOptions opts) {
        printf("name: %s, age: %d, phone: %d, address: %d\n", 
            opts.name.c_str(), opts.age, opts.phone, opts.address);
    };
    foo({.name = "A", .age = 12, .phone = 3456, .address = 7890});
}

// 强类型转换
struct FileHandle {
    int _handle;
    explicit FileHandle(int handle) : _handle(handle) {}
};

TEST(APIDesignTest, StrongTypeEncapsulation) {
    auto read = [](FileHandle handle, char *buf, size_t len) -> ssize_t {
        return len / 2;
    };

    constexpr auto TEST_FD = 123;
    FileHandle fh(TEST_FD);
    char buffer[64];
    ssize_t result = read(fh, buffer, sizeof(buffer));
    ASSERT_EQ(result, sizeof(buffer) / 2);
}

#if __cplusplus >= 202002L
// 支持任何具有 data 和 size 成员的各种标准库容器
template <typename Arr>
concept has_data_size = requires (Arr arr) {
    { arr.data() } -> std::convertible_to<char *>;
    { arr.size() } -> std::same_as<size_t>;
};
#else 
#include <type_traits>
template <typename Arr, typename = void>
struct has_data_size : std::false_type {};

template <typename Arr>
struct has_data_size<Arr, 
    std::void_t<decltype(std::declval<Arr>().data()), decltype(std::declval<Arr>().size())>> 
    : std::conjunction<
        std::is_convertible<decltype(std::declval<Arr>().data()), char*>,
        std::is_same<decltype(std::declval<Arr>().size()), size_t>
    > {};
#endif

 struct Span {
    char* data;
    size_t len;

    explicit Span(char *data, size_t len) : data(data), len(len) {}
    explicit Span(const char *data, size_t len) : data(const_cast<char*>(data)), len(len) {}
    template <size_t N>
    Span(char (&buf)[N]) : data(buf), len(N) {}
    // 允许 char [N] 隐式转换为 Span, 自动推导长度N。数组引用作为参数,不会退化为指针

#if __cplusplus >= 202002L
    template <has_data_size Arr>
    Span(Arr &arr) : data(arr.data()), len(arr.size()) {}
#else
    template <typename Arr, typename std::enable_if<has_data_size<Arr>::value, int>::type = 0>
    Span(Arr &arr) : data(arr.data()), len(arr.size()) {}
#endif

    Span subspan(size_t start, size_t length = (size_t)-1) const {
        if (start > len) {
            throw std::out_of_range("start > len");
        }
        auto resetSize = len - start; // 超出范围则截断
        if (length > resetSize) {
            length = resetSize;
        }
        return Span(data + start, length);
    }
};

// 封装指针和长度
TEST(APIDesignTest, SpanTest) {
    // Span 本身就是一个对原缓冲区的引用，直接传入 read 内部一样可以修改缓冲区
    auto read = [](FileHandle handle, Span buf) -> ssize_t {
        return buf.len / 2;
    };

    constexpr auto TEST_FD = 123;
    FileHandle fh(TEST_FD);
    char buffer[64];
    ssize_t result = read(fh, Span{buffer});
    ASSERT_EQ(result, sizeof(buffer) / 2);

    char buf1[30];
    Span span1 = buf1;
    ASSERT_EQ(span1.len, sizeof(buf1));

    std::array<char, 30> buf2;
    Span span2 = buf2;
    ASSERT_EQ(span2.len, buf2.size());

    std::vector<char> buf3(30);
    Span span3 = buf3;
    ASSERT_EQ(span3.len, buf3.size());

    const char *str = "hello";
    Span span4 = Span(str, strlen(str));
    ASSERT_EQ(span4.len, strlen(str));
}

TEST(APIDesignTest, SubSpanTest) {
    char buf1[30] = "Hello, this is a test buffer.";
    Span subspan1 = Span(buf1).subspan(5, 10);
    ASSERT_EQ(subspan1.len, 10);
    ASSERT_STREQ(subspan1.data, &(buf1[5]));
#if __cplusplus >= 202002L
    #include <span>
    std::span<char> subspan2 = std::span(buf1).subspan(5, 10);
    ASSERT_EQ(subspan2.size(), 10);
    ASSERT_STREQ(subspan2.data(), &(buf1[5]));
#endif
}

#if __cplusplus >= 202002L
template <class Arr, class T>
concept has_data_size_T = requires(Arr arr) {
    { std::data(arr) } -> std::convertible_to<T*>;
    { std::size(arr) } -> std::same_as<size_t>;
    // 使用 std::data 而不是 .data() 的好处：
    // 标准库容器还是普通数组（如 char buf[N]），都可以使用 std::data 和 std::size 来获取数据指针和大小。
    // 避免特定类型(char*)的构造函数,简化代码。
    // 如果 buf 是一个普通的 int* 指针，std::data(buf) 和 std::size(buf) 会重载失败，直接报错，从而避免了潜在的安全隐患
};

template <class T>
struct SpanT {
    T* data;
    size_t size;

    template <has_data_size_T<T> Arr>
    SpanT(Arr&& arr) : data(std::data(arr)), size(std::size(arr)) {}
};

template <typename Arr>
using DataPointerType = std::remove_pointer_t<decltype(std::data(std::declval<Arr&&>()))>;

template <typename Arr>
    requires has_data_size_T<Arr, DataPointerType<Arr>>
SpanT(Arr&& t) -> SpanT<DataPointerType<Arr>>;
#else
template <typename Arr, typename T, typename = void>
struct has_data_size_T : std::false_type {};

template <typename Arr, typename T>
struct has_data_size_T<Arr, T, 
    std::void_t<decltype(std::data(std::declval<Arr>())), decltype(std::size(std::declval<Arr>()))>> 
    : std::conjunction<
        std::is_convertible<decltype(std::data(std::declval<Arr>())), T*>,
        std::is_same<decltype(std::size(std::declval<Arr>())), size_t>
    > {};

template <class T>
struct SpanT {
    T* data;
    size_t size;

    template <typename Arr, typename std::enable_if<has_data_size_T<Arr, T>::value, int>::type = 0>
    SpanT(Arr&& arr) : data(std::data(arr)), size(std::size(arr)) {}
};

template <typename Arr>
SpanT(Arr&& arr) -> SpanT<std::remove_pointer_t<decltype(std::data(std::declval<Arr&&>()))>>;
#endif

TEST(APIDesignTest, SpanTTest) {
    char buf1[30] = "Hello, World!";
    SpanT span1(buf1);
    ASSERT_EQ(span1.size, 30);

    std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
    SpanT span2(vec);
    ASSERT_EQ(span2.size, vec.size());
    ASSERT_STREQ(span2.data, vec.data());

    std::array<char, 5> arr = {'W', 'o', 'r', 'l', 'd'};
    SpanT span3(arr);
    ASSERT_EQ(span3.size, arr.size());
    ASSERT_STREQ(span3.data, arr.data());

    std::vector<int> vec_int = {1, 2, 3, 4, 5};
    SpanT span4(vec_int);
    ASSERT_EQ(span4.size, vec_int.size());
    ASSERT_EQ(span4.data, vec_int.data());

    std::array<int, 5> arr_int = {10, 20, 30, 40, 50};
    SpanT span5(arr_int);
    ASSERT_EQ(span5.size, arr_int.size());
    ASSERT_EQ(span5.data, arr_int.data());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
