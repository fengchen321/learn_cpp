#include "prompt.h"
#include "gtest_prompt.h"
#include "macro_usage.h"

TEST(MacroUsage, ErrorToString) {
    EXPECT_STREQ(errorToString(ERR_FILE_NOT_FOUND), "FILE_NOT_FOUND");
    EXPECT_STREQ(errorToString(ERR_INVALID_PARAMETER), "INVALID_PARAMETER");
    EXPECT_STREQ(errorToString(ERR_OUT_OF_MEMORY), "OUT_OF_MEMORY");
    EXPECT_STREQ(errorToString(999), "Unknown error");
}

TEST(MacroUsage, CheckCode) {
#if NDEBUG
    ASSERT_NO_THROW(CHECK_CODE(test_checkcode()));
#else
    ASSERT_LOGS_EXCEPTION(CHECK_CODE(test_checkcode()), "test_checkcode() failed:");
#endif
}

TEST(MacroUsage, Assert) {
#if NDEBUG
    // Assertions are disabled in NDEBUG
    ASSERT_NO_THROW(OUR_ASSERT(0 == 2));
    int i = 1;
    ASSERT_NO_THROW(OUR_ASSERT_GT(i << 1 , 2));
    ASSERT_NO_THROW(OUR_ASSERT_GT(i++ , 1));
#else
    ASSERT_LOGS_EXCEPTION(OUR_ASSERT(0 == 2), "Assert failed: 0 == 2");
    int i = 1;
    ASSERT_LOGS_EXCEPTION(OUR_ASSERT_GT(i << 1 , 2), "Assert failed: i << 1 (2) > 2");
    ASSERT_LOGS_EXCEPTION(OUR_ASSERT_GT(i++ , 1), "Assert failed: i++ (1) > 1");
#endif
}

TEST(MacroUsage, Platform) {
    ASSERT_LOGS_STDOUT(foo(1), "1");
    ASSERT_LOGS_STDOUT(foo(), "foo");
}

TEST(MacroUsage, Print) {
    ASSERT_LOGS_STDOUT(PRINT("hello\n"), "hello");
    ASSERT_LOGS_STDOUT(PRINT("hello %d + %d \n", 9, 2), "hello 9 + 2");
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}