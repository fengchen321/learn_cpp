#include <cmath>
#include <sstream>
#include "gtest_prompt.h"
#include "ast_builder.h"
#include "commandParser.h"
#include "env.h"
#include "exception.h"
#include "parser.h"
#include "scanner.h"

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
    } catch (const CalcException& error) {
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
    } catch (const CalcException& error) {
        return error.what();
    }
}

std::string EvaluateError(const std::string& expression) {
    std::istringstream input(expression);
    Scanner scanner(input);
    Parser parser(scanner);
    try {
        parser.parse();
        static_cast<void>(parser.calc());
        return "";
    } catch (const CalcException& error) {
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
    EXPECT_THROW(parser.parse(), SyntaxError);
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

    EXPECT_EQ(EvaluateError("x", env), "Undefined variable: x");
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x = 5", env), 5.0);
    EXPECT_DOUBLE_EQ(ParseAndEvaluate("x + 1", env), 6.0);
}

TEST(ParserAssignmentTest, UsingUnknownVariableInExpressionFails) {
    Env env;

    EXPECT_EQ(EvaluateError("x + 1", env), "Undefined variable: x");
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

TEST(ExceptionTest, ThrowsSyntaxErrorForUnexpectedToken) {
    EXPECT_THROW({
        std::istringstream input("1 @ 2");
        Scanner scanner(input);
        Parser parser(scanner);
        parser.parse();
    }, SyntaxError);
}

TEST(ExceptionTest, ThrowsInvalidTokenErrorForBadCharacter) {
    EXPECT_THROW({
        std::istringstream input("$");
        Scanner scanner(input);
    }, InvalidTokenError);
}

TEST(ExceptionTest, InvalidTokenErrorContainsCharacterInfo) {
    try {
        std::istringstream input("#");
        Scanner scanner(input);
    } catch (const InvalidTokenError& e) {
        EXPECT_EQ(std::string(e.what()), "Invalid character: '#'");
    }
}

TEST(ExceptionTest, ThrowsDivisionByZeroError) {
    EXPECT_THROW({
        std::istringstream input("1 / 0");
        Scanner scanner(input);
        Parser parser(scanner);
        parser.parse();
        parser.calc();
    }, DivisionByZeroError);
}

TEST(ExceptionTest, DivisionByZeroErrorMessage) {
    try {
        std::istringstream input("1 / 0");
        Scanner scanner(input);
        Parser parser(scanner);
        parser.parse();
        parser.calc();
    } catch (const DivisionByZeroError& e) {
        EXPECT_EQ(std::string(e.what()), "Division by zero");
    }
}

TEST(ExceptionTest, ThrowsUndefinedVariableError) {
    Env env;
    EXPECT_THROW({
        std::istringstream input("unknown_var");
        Scanner scanner(input);
        Parser parser(scanner, env);
        parser.parse();
        parser.calc();
    }, UndefinedVariableError);
}

TEST(ExceptionTest, UndefinedVariableErrorContainsName) {
    Env env;
    try {
        std::istringstream input("xyz");
        Scanner scanner(input);
        Parser parser(scanner, env);
        parser.parse();
        parser.calc();
    } catch (const UndefinedVariableError& e) {
        EXPECT_EQ(std::string(e.what()), "Undefined variable: xyz");
    }
}

TEST(ExceptionTest, ThrowsUnknownFunctionError) {
    Env env;
    EXPECT_THROW({
        std::istringstream input("nonexistent(1)");
        Scanner scanner(input);
        Parser parser(scanner, env);
        parser.parse();
        parser.calc();
    }, UnknownFunctionError);
}

TEST(ExceptionTest, UnknownFunctionErrorContainsName) {
    Env env;
    try {
        std::istringstream input("myfunc(1)");
        Scanner scanner(input);
        Parser parser(scanner, env);
        parser.parse();
        parser.calc();
    } catch (const UnknownFunctionError& e) {
        EXPECT_EQ(std::string(e.what()), "Unknown function: myfunc");
    }
}

TEST(ExceptionTest, ExceptionHierarchy) {
    // SyntaxError should be catchable as CalcException
    EXPECT_THROW({
        std::istringstream input("@");
        Scanner scanner(input);
    }, CalcException);

    // RuntimeError should be catchable as CalcException
    Env env;
    EXPECT_THROW({
        std::istringstream input("x");
        Scanner scanner(input);
        Parser parser(scanner, env);
        parser.parse();
        parser.calc();
    }, CalcException);
}

TEST(ExceptionTest, RuntimeErrorHierarchy) {
    // DivisionByZeroError should be catchable as RuntimeError
    EXPECT_THROW({
        std::istringstream input("1 / 0");
        Scanner scanner(input);
        Parser parser(scanner);
        parser.parse();
        parser.calc();
    }, RuntimeError);
}

// CommandParser Tests

TEST(CommandParserTest, ParsesHelpCommand) {
    Env env;
    std::istringstream input("!help\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesHelpCommandShortForm) {
    Env env;
    std::istringstream input("!h\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesQuitCommand) {
    Env env;
    std::istringstream input("!quit\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_QUIT);
}

TEST(CommandParserTest, ParsesQuitCommandShortForm) {
    Env env;
    std::istringstream input("!q\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_QUIT);
}

TEST(CommandParserTest, ParsesListVarsCommand) {
    Env env;
    std::istringstream input("!list_vars\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesListVarsCommandShortForm) {
    Env env;
    std::istringstream input("!v\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesListFuncsCommand) {
    Env env;
    std::istringstream input("!list_funcs\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesListFuncsCommandShortForm) {
    Env env;
    std::istringstream input("!f\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesLoadCommand) {
    Env env;
    std::istringstream input("!load test.txt\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesLoadCommandShortForm) {
    Env env;
    std::istringstream input("!l test.dat\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesSaveCommand) {
    Env env;
    std::istringstream input("!save output.txt\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, ParsesSaveCommandShortForm) {
    Env env;
    std::istringstream input("!s output.dat\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_SUCCESS);
}

TEST(CommandParserTest, HandlesUnknownCommand) {
    Env env;
    std::istringstream input("!unknown\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    CommandParser cmdParser(scanner, env);
    EXPECT_EQ(cmdParser.execute(), EStatus::STATUS_ERROR);
}

TEST(CommandParserTest, RecognizesCommandToken) {
    Env env;
    std::istringstream input("!help\n");
    Scanner scanner(input);
    EXPECT_TRUE(scanner.isCommand());
    EXPECT_EQ(scanner.getToken(), EToken::TOKEN_COMMAND);
}

TEST(CommandParserTest, NonCommandIsNotRecognized) {
    std::istringstream input("1 + 2\n");
    Scanner scanner(input);
    EXPECT_FALSE(scanner.isCommand());
    EXPECT_EQ(scanner.getToken(), EToken::TOKEN_NUMBER);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
