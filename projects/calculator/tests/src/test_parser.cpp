#include "gtest_prompt.h"
#include "parser.h"
#include "scanner.h"

#include <sstream>

namespace {

double ParseAndEvaluate(const std::string& expression) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner);
    parser.parse();
    return parser.calc();
}

} // namespace

TEST(ParserTest, ParsesSingleNumber) {
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("42"), 42.0);
}

TEST(ParserTest, RespectsOperatorPrecedence) {
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("1 + 2 * 3"), 7.0);
}

TEST(ParserTest, ParsesParenthesizedExpressions) {
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("(1 + 2) * 3"), 9.0);
}

TEST(ParserTest, ParsesUnaryMinus) {
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("-5 + 2"), -3.0);
}

TEST(ParserTest, UsesLeftAssociativeSubtraction) {
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("10 - 3 - 2"), 5.0);
}

TEST(ParserTest, RejectsIncompleteExpression) {
    std::istringstream input("1 +");
    Scanner scanner(input);
    Parser parser(scanner);
    EXPECT_THROW(parser.parse(), std::runtime_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
