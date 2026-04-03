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
// 多值返回
void divide_reference(int dividend, int divisor, int& quotient, int& remainder) {
    quotient = dividend / divisor;
    remainder = dividend % divisor;
}

void divide_point(int dividend, int divisor, int* quotient, int* remainder) {
    if (quotient) *quotient = dividend / divisor;
    if (remainder) *remainder = dividend % divisor;
}

// tuple tie组合解包
std::tuple<int, int> divide_tuple(int dividend, int divisor) {
    return std::make_tuple(dividend / divisor, dividend % divisor);
}

// 结构化绑定
struct DivideResult {
    int quotient;
    int remainder;
};
DivideResult divide_struct(int dividend, int divisor) {
    return {dividend / divisor, dividend % divisor};
}

void divide_callback(int dividend, int divisor, std::function<void(int, int)> callback) {
    callback(dividend / divisor, dividend % divisor);
}
// 自定义out结构体包装，将输出参数作为结构体的成员，用于任意类型
template <class T>
struct out {
    std::function<void(T)> target;

    out(T* t)
        : target([t](T&& in) {
            if (t) *t = std::move(in);
        }) {}
    template <class... Args>
    void emplace(Args&&... args) {
        target(T(std::forward<Args>(args)...));
    }
    template <class X>
    void operator=(X&& x) {
        emplace(std::forward<X>(x));
    }
    template <class... Args>
    void operator()(Args&&... args) {
        emplace(std::forward<Args>(args)...);
    }
};
void divide_out(int dividend, int divisor, out<int>& quotient_out, out<int>& remainder_out) {
    quotient_out.emplace(dividend / divisor);
    remainder_out.emplace(dividend % divisor);
}
// 模板推导
template <typename T1, typename T2>
struct many {
    T1 quotient;
    T2 remainder;
};

template <class T1, class T2>
many(T1, T2) -> many<T1, T2>;

many<int, int> divide_template(int dividend, int divisor) {
    return many{
        dividend / divisor,
        dividend % divisor,
    };
}

TEST(APIDesignTest, MultivaluedReturn) {
    int dividend = 10;
    int divisor = 3;
    int quotient, remainder;
    divide_reference(dividend, divisor, quotient, remainder);
    ASSERT_EQ(quotient, 3);
    ASSERT_EQ(remainder, 1);

    quotient = 0; remainder = 0;
    divide_point(dividend, divisor, &quotient, &remainder);
    ASSERT_EQ(quotient, 3);
    ASSERT_EQ(remainder, 1);

    quotient = 0; remainder = 0;
    std::tie(quotient, remainder) = divide_tuple(dividend, divisor);
    ASSERT_EQ(quotient, 3);
    ASSERT_EQ(remainder, 1);

    auto result = divide_struct(dividend, divisor);
    ASSERT_EQ(result.quotient, 3);
    ASSERT_EQ(result.remainder, 1);

    quotient = 0; remainder = 0;
    auto callback = [&quotient, &remainder](int q, int r) {
        quotient = q;
        remainder = r;
    };
    divide_callback(dividend, divisor, callback);
    ASSERT_EQ(quotient, 3);
    ASSERT_EQ(remainder, 1);

    quotient = 0; remainder = 0;
    out<int> quotient_out(&quotient);
    out<int> remainder_out(&remainder);
    divide_out(dividend, divisor, quotient_out, remainder_out);
    ASSERT_EQ(quotient, 3);
    ASSERT_EQ(remainder, 1);

    auto [x, y] = divide_template(dividend, divisor);
    ASSERT_EQ(x, 3);
    ASSERT_EQ(y, 1);
}
// 强类型转换
struct FileHandle_ {
    int _handle;
    explicit FileHandle_(int handle) : _handle(handle) {}
};

TEST(APIDesignTest, StrongTypeEncapsulation) {
    auto read = [](FileHandle_ handle, char *buf, size_t len) -> ssize_t {
        return len / 2;
    };

    constexpr auto TEST_FD = 123;
    FileHandle_ fh(TEST_FD);
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
    auto read = [](FileHandle_ handle, Span buf) -> ssize_t {
        return buf.len / 2;
    };

    constexpr auto TEST_FD = 123;
    FileHandle_ fh(TEST_FD);
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

    int arr1[32];
    SpanT span6(arr1);
    ASSERT_EQ(span6.size, 32);

}
// 空值语义
#include <optional>
TEST(APIDesignTest, OptionalTest) {
    struct BookInfo {
        std::string title;
        std::string author;
    };

    struct ISBN {
        std::string number;
        bool operator<(const ISBN& other) const {
            return number < other.number;
        }
    };

    std::map<ISBN, BookInfo> bookDatabase = {
        {{"1"}, {"C++ Programming", "Bjarne Stroustrup"}},
        {{"2"}, {"Effective C++", "Scott Meyers"}}
    };

    auto foo = [&](ISBN isbn) -> std::optional<BookInfo> {
        auto it = bookDatabase.find(isbn);
        if (it != bookDatabase.end()) {
            return it->second;
        } else {
            return std::nullopt;
        }
    };

    ISBN isbn{"1"};
    std::optional<BookInfo> result = foo(isbn);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->title, "C++ Programming");
    EXPECT_EQ(result->author, "Bjarne Stroustrup");
    EXPECT_EQ((*result).title, "C++ Programming");
    EXPECT_EQ((*result).author, "Bjarne Stroustrup");

    BookInfo book1 = foo(isbn).value();
    EXPECT_EQ(book1.title, "C++ Programming");
    EXPECT_EQ(book1.author, "Bjarne Stroustrup");

    ASSERT_THROW(foo(ISBN{"x"}).value(), std::bad_optional_access);

    // 指定找不到时的默认值
    BookInfo book2 = foo(ISBN{"x"}).value_or(BookInfo{"default", "default"});
    EXPECT_EQ(book2.title, "default");
    EXPECT_EQ(book2.author, "default"); 
}
#include <string_view>
#include <charconv>
#include <functional>
TEST(APIDesignTest, OptionalTest1) {
    auto parseInt = [](std::string_view sv) -> std::optional<int> {
        int value;
        auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (result.ec == std::errc())
            return value;
        else
            return std::nullopt;
    };

    ASSERT_TRUE(parseInt("-1"));
    ASSERT_FALSE(parseInt("s"));

    char perfGeek[2] = {'-', '1'};
    ASSERT_TRUE(parseInt(std::string_view{perfGeek, 2}));
    EXPECT_EQ(parseInt(std::string_view{perfGeek, 2}).value(), -1);
}
// 强类型枚举
TEST(APIDesignTest, EnumTest) {
    enum class Sex : uint8_t {
        Female = 0,
        Male = 1,
        Custom = 2,
    };

    ASSERT_TRUE(sizeof(Sex) == 1);

}

// 此处使用 CRTP 模式是为了让 Typed 每次都实例化出不同的基类，阻止 object-slicing
template <class CRTP, class T>
struct Typed {
protected:
    T value;

public:
    explicit operator T() const {
        return value;
    }

    explicit Typed(T value) : value(value) {}
};

struct Meter : Typed<Meter, double> {
    explicit Meter(double value) : Typed<Meter, double>(value) {}
};

struct Kilometer : Typed<Kilometer, double> {
    explicit Kilometer(double value) : Typed<Kilometer, double>(value) {}
    operator Meter() const {
        return Meter(value * 1000);
    }
};

TEST(APIDesignTest, OtherTypeTest1) {
    Meter m = Kilometer(1.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(m), 1000.0);
}

#include <memory>       // std::shared_ptr
#include <filesystem>     // std::filesystem::path
#include <map>          // std::map
#include <cstdio>       // fopen, fclose, _wfopen
#include <optional>     // std::optional
#include <cerrno>       // std::errc
// #include <span>         // std::span
using FileHandle = std::shared_ptr<FILE>;
enum class OpenMode {
    Read,
    Write,
    Append,
};

inline OpenMode operator|(OpenMode a, OpenMode b) {
    return OpenMode(static_cast<int>(a) | static_cast<int>(b));
}

auto modeLut = std::map<OpenMode, std::string>{
    {OpenMode::Read, "r"},
    {OpenMode::Write, "w"},
    {OpenMode::Append, "a"},
    {OpenMode::Read | OpenMode::Write, "w+"},
    {OpenMode::Read | OpenMode::Append, "a+"},
};

FileHandle file_open(std::filesystem::path path, OpenMode mode) {
#ifdef _WIN32
    return std::shared_ptr<FILE>(_wfopen(path.wstring().c_str(), modeLut.at(mode).c_str()), fclose);
#else
    return std::shared_ptr<FILE>(fopen(path.string().c_str(), modeLut.at(mode).c_str()), fclose);
#endif
}

struct FileResult {
    std::optional<size_t> numElements;
    std::errc errorCode;  // std::errc 是个强类型枚举，用于取代 C 语言 errno 的 int 类型
    bool isEndOfFile;
};

template <class T>
FileResult file_read(FileHandle file, SpanT<T> elements) { // std::span<T> data() size()
    auto n = fread(elements.data, sizeof(T), elements.size, file.get());
    return {
        .numElements = (n == 0) ? std::nullopt : std::optional<size_t>(n),
        .errorCode = std::errc(ferror(file.get())),
        .isEndOfFile = (bool)feof(file.get()),
    };
}
#include <fstream>
TEST(APIDesignTest, RAII) {
    const char* filename = "a.txt";
    std::ofstream txtfile(filename);
    txtfile << "helloworldhelloworldhelloworld\n";
    txtfile.close();

    std::filesystem::path filePath("a.txt");
    ASSERT_TRUE(std::filesystem::exists(filePath));

    FileHandle file = file_open(filePath, OpenMode::Read);
    ASSERT_NE(file.get(), nullptr);

    char arr[12];
    auto result = file_read(file, SpanT(arr));

    ASSERT_TRUE(result.numElements.has_value());
    size_t numElements = result.numElements.value();
    ASSERT_EQ(numElements, 12);

    remove(filename);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
