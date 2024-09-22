#pragma once

#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>
#include <optional>
#include <charconv>
#include <memory>

struct JSONObject;
using JSONList = std::vector<JSONObject>;
using JSONDict = std::unordered_map<std::string, std::unique_ptr<JSONObject>>;
// std::unordered_map<std::string, JSONObject> gcc11.4.0还不行
// 自己引用自己，只能存在堆上；std::unique_ptr<JSONObject>可以
// struct内JSONObject或std::tuple<int, JSONObject>存在栈上不行
struct JSONObject {
    std::variant
    < std::nullptr_t    // null
    , bool              // true
    , int               // 42
    , double            // 3.14
    , std::string       // "hello"  
    , JSONList          // [42, "hello"]
    , JSONDict         // {"key": 42}
    > inner;

    // Constructors
    JSONObject();
    JSONObject(std::nullptr_t);
    JSONObject(bool);
    JSONObject(int);
    JSONObject(double);
    JSONObject(const std::string&);
    JSONObject(JSONList);
    JSONObject(JSONDict);

    void do_print() const;

    template <class T>
    bool is() const {
        return std::holds_alternative<T>(inner);
    }

    template <class T>
    T const& get() const {
        return std::get<T>(inner);
    }

    template <class T>
    T& get() {
        return std::get<T>(inner);
    }
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
    T value;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
        return value;
    }
    return std::nullopt;
}

char unescaped_char(char);

std::pair<JSONObject, size_t> parse(std::string_view);

template <class ...Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

template <class ...Fs>
overloaded(Fs...) -> overloaded<Fs...>;