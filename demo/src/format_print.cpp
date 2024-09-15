#include <iostream>
#include "minilog.h"
#if __has_include(<format>) && __cplusplus >= 202002L
void test_format() {
    auto s = std::format("{:0>+10.4f}", 13.14);
    std::cout << s << "\n";
}

void test_log() {
    // log_info("hello, the answer is {}", 42);
    minilog::log_Info("hello, the answer is {}", 42);
    minilog::log_Critical("this is right-aligned [{:>+10.04f}]", 3.14);

    minilog::log_Warn("good job, {1:.5s} for making {0}", "minilog", "archibate");
    minilog::set_log_level(minilog::log_level::Trace); // default log level is info
    // minilog::set_log_file("mini.log"); // uncomment to dump log to a specific file

    int my_variable = 42;
    MINILOG_P(my_variable); // shown when log level lower than debug

    minilog::log_Trace("below is the color show :)");
#define _FUNCTION(name) minilog::log_##name(#name);
    MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
}
#else
void test_format() {
    std::cout << "not support std::format" << "\n";
}
void test_log() {
    std::cout << "not support std::format" << "\n";
}
#endif
void test_source_location() {
    auto sl = std::source_location::current();
    printf("line = %d, colum=%d, file=%s,func=%s\n", 
        sl.line(), sl.column(), sl.file_name(), sl.function_name()
    );
}

void log(std::string msg, std::source_location sl = std::source_location::current()) {
    printf("line = %d, colum=%d, file=%s,func=%s\n", 
        sl.line(), sl.column(), sl.file_name(), sl.function_name()
    );
}

int main() {
    test_format();
    test_source_location();
    log("hello world"); // 默认参数计算的位置在调用者位置
    log("hello world");
    test_log();
    return 0;
}