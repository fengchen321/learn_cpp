#include "gtest_prompt.h"
#include "ast_builder.h"
#include "env.h"
#include "parser.h"
#include "scanner.h"
#include <cmath>
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

double ParseAndEvaluate(const std::string& expression, IAstBuilder& builder, Env& env) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner, builder, env);
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

TEST(ParserFunctionTest, SupportsBuiltinFunctionInsideExpression) {
    Env env;

    EXPECT_NEAR(ParseAndEvaluate("4 + log(e * e)", env), 6.0, 1e-12);
}

TEST(ParserFunctionTest, EvaluatesAllBuiltinFunctions) {
    Env env;
    constexpr double kTolerance = 1e-12;

    struct FunctionCase {
        const char* expression;
        double expected;
    };

    const FunctionCase cases[] = {
        {"log(e * e)", 2.0},
        {"log10(1000)", 3.0},
        {"exp(1)", std::exp(1.0)},
        {"sqrt(81)", 9.0},
        {"sin(pi / 6)", std::sin(std::acos(-1.0) / 6.0)},
        {"cos(0)", 1.0},
        {"tan(pi / 4)", std::tan(std::acos(-1.0) / 4.0)},
        {"asin(0.5)", std::asin(0.5)},
        {"acos(0.5)", std::acos(0.5)},
        {"atan(1)", std::atan(1.0)},
        {"sinh(0)", 0.0},
        {"cosh(0)", 1.0},
        {"tanh(0)", 0.0},
    };

    for (const auto& testCase : cases) {
        SCOPED_TRACE(testCase.expression);
        EXPECT_NEAR(ParseAndEvaluate(testCase.expression, env), testCase.expected, kTolerance);
    }
}

TEST(ParserFunctionTest, WorksWithBinaryAndNaryBuilders) {
    BinaryAstBuilder binaryBuilder;
    NaryAstBuilder naryBuilder;
    Env binaryEnv;
    Env naryEnv;

    EXPECT_NEAR(ParseAndEvaluate("4 + log(e * e)", binaryBuilder, binaryEnv), 6.0, 1e-12);
    EXPECT_NEAR(ParseAndEvaluate("4 + log(e * e)", naryBuilder, naryEnv), 6.0, 1e-12);
}

TEST(ParserFunctionTest, RejectsUnknownFunctionCalls) {
    Env env;

    EXPECT_EQ(EvaluateError("unknown(1)", env), "Unknown function: unknown");
}

TEST(ParserFunctionTest, RejectsBuiltinFunctionWithoutParentheses) {
    Env env;

    EXPECT_EQ(EvaluateError("log e", env), "Expected '(' after function name");
}

TEST(ParserFunctionTest, RejectsFunctionCallWithMissingClosingParenthesis) {
    Env env;

    EXPECT_EQ(EvaluateError("log(e * e", env), "Expected ')'");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
