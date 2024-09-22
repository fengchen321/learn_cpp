#ifndef LEARN_CPP_GTEST_PROMPT_H
#define LEARN_CPP_GTEST_PROMPT_H

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <regex>

#define ASSERT_LOGS_EXCEPTION(expression, expected_message) \
    do {                                                      \
        try {                                                \
            testing::internal::CaptureStderr();              \
            expression;                                     \
            FAIL() << "Expected exception not thrown";       \
        } catch (const std::exception& e) {                \
            std::string output = testing::internal::GetCapturedStderr(); \
            std::cout << output << std::endl;                   \
            ASSERT_NE(output.find(expected_message), std::string::npos) \
                << "Error message not found in stderr output."; \
        }                                                   \
    } while (0)

#define ASSERT_THROW_EXCEPTION(expression, expected_message) \
    do {                                                        \
        try {                                                  \
            expression;                                       \
            FAIL() << "Expected exception not thrown";         \
        } catch (const std::exception& e) {                    \
            ASSERT_STREQ(expected_message, e.what()) <<        \
                "Exception message does not match";            \
        }                                                       \
    } while (0)

#define ASSERT_LOGS_STDOUT(expression, expected_output) \
    do { \
        try { \
            testing::internal::CaptureStdout(); \
            expression; \
            std::string output = testing::internal::GetCapturedStdout(); \
            std::cout << output << std::endl; \
            ASSERT_NE(output.find(expected_output), std::string::npos) \
                << "Expected output not found in stdout."; \
        } catch (...) { \
            FAIL() << "Exception thrown while executing the expression."; \
        } \
    } while (0)

#define EXPECT_IN_RANGE(value, lower, upper) \
    EXPECT_GE(value, lower) << "Value " << value << " is less than " << lower << "."; \
    EXPECT_LE(value, upper) << "Value " << value << " is greater than " << upper << ".";

// 自定义宏来检查多个子字符串
#define EXPECT_CONTAINS_ALL(actual, ...) do {                    \
    std::vector<std::string> expected_substrings = {__VA_ARGS__}; \
    for (const auto& substr : expected_substrings) {             \
        EXPECT_NE(actual.find(substr), std::string::npos);      \
    }                                                              \
} while (0)

#define EXPECT_REGEX_MATCH(actual, pattern) do {                     \
    std::regex regex_pattern(pattern);                                \
    EXPECT_TRUE(std::regex_search(actual, regex_pattern));           \
} while (0)

#define ASSERT_REGEX_STDOUT(expression, pattern) \
    do { \
        try { \
            testing::internal::CaptureStdout(); \
            expression; \
            std::string output = testing::internal::GetCapturedStdout(); \
            std::cout << output << std::endl;    \
            EXPECT_REGEX_MATCH(output, pattern);  \
        } catch (...) { \
            FAIL() << "Exception thrown while executing the expression."; \
        } \
    } while (0)

#endif //LEARN_CPP_GTEST_PROMPT_H