#include "gtest_prompt.h"
#include "ast_builder.h"
#include "env.h"
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

double ParseAndEvaluate(const std::string& expression, IAstBuilder& builder) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner, builder);
    parser.parse();
    return parser.calc();
}

double ParseAndEvaluate(const std::string& expression, Env& env) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner, env);
    parser.parse();
    return parser.calc();
}

std::string EvaluateError(const std::string& expression, IAstBuilder& builder) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner, builder);
    try {
        parser.parse();
        static_cast<void>(parser.calc());
        return "";
    } catch (const std::runtime_error& error) {
        return error.what();
    }
}

std::string EvaluateError(const std::string& expression, Env& env) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner, env);
    try {
        parser.parse();
        static_cast<void>(parser.calc());
        return "";
    } catch (const std::runtime_error& error) {
        return error.what();
    }
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

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForAdditiveChain) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("10 - 3 + 2 - 4", binaryBuilder),
                     ParseAndEvaluate("10 - 3 + 2 - 4", naryBuilder));
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForMultiplicativeChain) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("48 / 3 * 2 / 4", binaryBuilder),
                     ParseAndEvaluate("48 / 3 * 2 / 4", naryBuilder));
}

TEST(ParserBuilderTest, NaryBuilderRespectsPrecedenceWithParentheses) {
    NaryAstBuilder builder;
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("2 * (3 + 4) - 5", builder), 9.0);
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForUnaryMinus) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("-5 + 2 * -3", binaryBuilder),
                     ParseAndEvaluate("-5 + 2 * -3", naryBuilder));
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForMixedPrecedence) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("10 - 2 * 3 + 8 / 4", binaryBuilder),
                     ParseAndEvaluate("10 - 2 * 3 + 8 / 4", naryBuilder));
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForLongAdditiveChain) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("1 + 2 - 3 + 4 - 5 + 6", binaryBuilder),
                     ParseAndEvaluate("1 + 2 - 3 + 4 - 5 + 6", naryBuilder));
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForLongMultiplicativeChain) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("64 / 2 / 4 * 8 / 2", binaryBuilder),
                     ParseAndEvaluate("64 / 2 / 4 * 8 / 2", naryBuilder));
}

TEST(ParserBuilderTest, BinaryAndNaryBuildersMatchForDivisionByZero) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;

    EXPECT_EQ(EvaluateError("8 / (3 - 3)", binaryBuilder),
              EvaluateError("8 / (3 - 3)", naryBuilder));
    EXPECT_EQ(EvaluateError("8 / (3 - 3)", binaryBuilder), "Division by zero");
}

TEST(ParserAssignmentTest, SupportsSimpleAssignmentAcrossStatements) {
    Env env;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x = 5", env), 5.0);
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x + 6", env), 11.0);
}

TEST(ParserAssignmentTest, SupportsChainedAssignmentAcrossStatements) {
    Env env;

    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x = y = 9", env), 9.0);
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x + 2", env), 11.0);
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("y + 3", env), 12.0);
}

TEST(ParserAssignmentTest, ReadingUnknownVariableFailsWithoutRegisteringIt) {
    Env env;

    EXPECT_EQ(EvaluateError("x", env), "Undefined variable");
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x = 5", env), 5.0);
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x + 1", env), 6.0);
}

TEST(ParserAssignmentTest, UsingUnknownVariableInExpressionFails) {
    Env env;

    EXPECT_EQ(EvaluateError("x + 1", env), "Undefined variable");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
