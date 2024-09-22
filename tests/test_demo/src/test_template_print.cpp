#include "gtest_prompt.h"
#include "prompt.h"
#include "template_print.h"

TEST(PrintTest, Sum) {
    EXPECT_EQ(sum(), 0);
    EXPECT_EQ(sum(1), 1);
    EXPECT_EQ(sum(1, 2), 3);
}

TEST(PrintTest, Print) {
    ASSERT_LOGS_STDOUT(printfun(), "{}\n");
    ASSERT_LOGS_STDOUT(printfun(1), "{1}\n");
    ASSERT_LOGS_STDOUT(printfun(1,2), "{1, 2}\n");
    ASSERT_LOGS_STDOUT(printfun(1,2,3), "{1, 2, 3}\n");
}

int main(int argc, char** argv) {
    prompt::display_cpp_version();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}