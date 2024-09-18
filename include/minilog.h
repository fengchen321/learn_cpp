#ifndef LEARN_CPP_MINILOG_H
#define LEARN_CPP_MINILOG_H

#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <source_location>

#if __has_include(<format>) && __cplusplus >= 202002L
#include <format>  // ubuntu -> g++ 12.2 <format>目前尚不包含 -> gcc13.1, clang17

namespace minilog {
    
#define MINILOG_FOREACH_LOG_LEVEL(f)    \
    f(Trace)                    \
    f(Debug)                    \
    f(Info)                     \
    f(Critical)                 \
    f(Warn)                     \
    f(Error)                    \
    f(Fatal)

enum class log_level : std::uint8_t{
#define _FUNCTION(name) name,
    MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

namespace details {

#if defined(__linux__) || defined(__APPLE__)
inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::Fatal + 1][8] = {
    "\E[37m",  // white
    "\E[35m",  // magenta
    "\E[32m",  // green
    "\E[34m",  // blue
    "\E[33m",  // yellow
    "\E[31m",  // red
    "\E[31;1m",// bright red
};
inline constexpr char k_reset_ansi_color[4] = "\E[m";
#define _MINILOG_IF_HAS_ANSI_COLORS(x) x
#else
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::Fatal + 1][1] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};
inline constexpr char k_reset_ansi_color[1] = "";
#endif

inline std::string log_level_name(log_level lev) {
    switch (lev) {
#define _FUNCTION(name) case log_level::name: return #name;
        MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    }
    return "Unknown";
}

inline log_level log_level_from_name(std::string lev_name) {
#define _FUNCTION(name) if (lev_name == #name) return log_level::name;
        MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    return log_level::Info;
}

template <class T>
struct with_source_location {
private:
    T inner;
    std::source_location loc;
public:
// consteval只能编译期调用
    template <class U> requires std::constructible_from<T, U>
    consteval with_source_location(U &&u, std::source_location sl = std::source_location::current())
        : inner(std::forward<U>(u)), loc(std::move(sl)) {}
    constexpr T const &format() const { return inner; }
    constexpr std::source_location const &location() const { return loc; }
};

// inline log_level g_max_level = log_level::Info;
// 和设置file路径方法一样
inline log_level g_max_level = []() -> log_level {
    auto p = getenv("MINILOG_LEVEL");
    // if (p) {
    //     return log_level_from_name(p);
    // }
    // return log_level::Info;
    return p ? log_level_from_name(p) : log_level::Info;
}();

// inline std::ofstream g_log_file{"mini.log"};
inline std::ofstream g_log_file = [] () -> std::ofstream {
    if (auto path = std::getenv("MINILOG_FILE")) {
        return std::ofstream(path, std::ios::app);
    }
    return std::ofstream();
} ();
// 使用 output_log处理线程安全问题：使用 + 放一起，避免多线程同时写
inline void output_log(log_level lev, std::string msg, std::source_location const &loc) {
    std::chrono::zoned_time now{std::chrono::current_zone(), std::chrono::system_clock::now()};
    msg = std::format("{} {}:{} [{}] {}", now, loc.file_name(), loc.line(), log_level_name(lev), msg);
    if (g_log_file) {
        g_log_file << msg + '\n';
    }
    if (lev >= g_max_level) {
        std::cout << _MINILOG_IF_HAS_ANSI_COLORS(k_level_ansi_colors[(std::uint8_t)lev] +)
                    msg _MINILOG_IF_HAS_ANSI_COLORS(+ k_reset_ansi_color) + '\n';
    }
}

} // namespace details

inline void set_log_file(std::string path) {
    details::g_log_file = std::ofstream(path, std::ios::app);
}

inline void set_log_level(log_level lev) {
    details::g_max_level = lev;
}

// template<typename ...Args>
// void log_info(details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
//     auto const &loc = fmt.location();
//     std::cout << loc.file_name() << ':' << loc.line() << " [info] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << '\n';
// }

// template<typename ...Args>
// void generic_log(log_level lev, details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
//     if (lev >= details::g_max_level) {
//         auto const &loc = fmt.location();
//         std::cout << loc.file_name() << ':' << loc.line() << " [" << details::log_level_name(lev) << "] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << '\n';
//     }
// }

template<typename ...Args>
void generic_log(log_level lev, details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
    auto const &loc = fmt.location();
    auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));
    details::output_log(lev, std::move(msg), loc);
}

#define _FUNCTION(name)     \
template<typename ...Args>  \
void log_##name(details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) { \
    return generic_log(log_level::name, std::move(fmt), std::forward<Args>(args)...);             \
}
MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

#define MINILOG_P(x) ::minilog::log_Debug(#x "={}", x)   // ::minilog 先跳到全局，再跳到命名空间

} // namespace minilog
#endif
#endif // LEARN_CPP_MINILOG_H
