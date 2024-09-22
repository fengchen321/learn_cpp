#include <iostream>
#include "minilog.h"
#include "gtest_prompt.h"

#if __has_include(<format>) && __cplusplus >= 202002L

TEST(FormatPrint, FormatTest) {
    auto s = std::format("{:0>+10.4f}", 13.14);
    EXPECT_EQ(s, "00+13.1400");
}

TEST(FormatPrint, MiniLogTest) {
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
TEST(FormatPrint, FormatTest) {
    std::cout << "not support std::format" << "\n";
}

TEST(FormatPrint, MiniLogTest) {
    std::cout << "not support std::format" << "\n";
}

#endif

TEST(FormatPrint, SourceLocationTest) {
    auto sl = std::source_location::current();
    printf("line = %d, colum=%d, file=%s,func=%s\n", 
        sl.line(), sl.column(), sl.file_name(), sl.function_name()
    );
    EXPECT_EQ(sl.line(), __LINE__ - 4); 
    EXPECT_STREQ(sl.file_name(), __FILE__);
}

void log(std::string msg, std::source_location sl = std::source_location::current()) {
    printf("line = %d, colum=%d, file=%s,func=%s\n", 
        sl.line(), sl.column(), sl.file_name(), sl.function_name()
    );
}

TEST(FormatPrint, LogFuncTest) {
    log("hello world"); // 默认参数计算的位置在调用者位置
    log("hello world");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}