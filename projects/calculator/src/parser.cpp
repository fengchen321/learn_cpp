#include <vector>
#include "ast_builder.h"
#include "node.h"
#include "scanner.h"
#include "parser.h"
#include "env.h"
#include "exception.h"

Parser::Parser(Scanner& scanner)
    : ownedBuilder_(std::make_unique<BinaryAstBuilder>()),
      ownedEnv_(std::make_unique<Env>()),
      builder_(*ownedBuilder_),
      scanner_(scanner),
      env_(*ownedEnv_),
      tree_(nullptr),
      status_(EStatus::STATUS_SUCCESS) {}

Parser::Parser(Scanner& scanner, IAstBuilder& builder)
    : ownedBuilder_(nullptr),
      ownedEnv_(std::make_unique<Env>()),
      builder_(builder),
      scanner_(scanner),
      env_(*ownedEnv_),
      tree_(nullptr),
      status_(EStatus::STATUS_SUCCESS) {}

Parser::Parser(Scanner& scanner, Env& env)
    : ownedBuilder_(std::make_unique<BinaryAstBuilder>()),
      ownedEnv_(nullptr),
      builder_(*ownedBuilder_),
      scanner_(scanner),
      env_(env),
      tree_(nullptr),
      status_(EStatus::STATUS_SUCCESS) {}

Parser::Parser(Scanner& scanner, IAstBuilder& builder, Env& env)
    : ownedBuilder_(nullptr),
      ownedEnv_(nullptr),
      builder_(builder),
      scanner_(scanner),
      env_(env),
      tree_(nullptr),
      status_(EStatus::STATUS_SUCCESS) {}

Parser::~Parser() = default;

EStatus Parser::parse() {
    try {
        tree_ = expr();
        if (scanner_.getToken() != EToken::TOKEN_END) {
            throw SyntaxError("Unexpected token");
        }
        if (!scanner_.isDone()) {
            status_ = EStatus::STATUS_ERROR;
        }
        return status_;
    } catch (...) {
        tree_.reset();
        status_ = EStatus::STATUS_ERROR;
        throw;
    }
}

double Parser::calc() const {
    if (!tree_) {
        throw RuntimeError("Parse tree is empty");
    }
    return tree_->calc();
}

/*
expr is 
    term + expr
    term - expr
    term = expr
    term
*/
std::unique_ptr<Node> Parser::expr() {
    std::unique_ptr<Node> node = term();
    std::vector<AdditivePart> rest;
    while (scanner_.getToken() == EToken::TOKEN_PLUS ||
           scanner_.getToken() == EToken::TOKEN_MINUS) {
        const EAdditiveOp op =
            scanner_.getToken() == EToken::TOKEN_PLUS ? EAdditiveOp::Add : EAdditiveOp::Subtract;
        scanner_.accept();
        rest.push_back({op, term()});
    }
    node = builder_.makeAdditive(std::move(node), std::move(rest));
    if (scanner_.getToken() == EToken::TOKEN_ASSIGN) {
        scanner_.accept();
        std::unique_ptr<Node> right = expr();
        if (!node->isLvalue()) {
            throw SyntaxError("Cannot assign to a non-lvalue");
        }
        node = builder_.makeAssign(std::move(node), std::move(right));
    }
    return node;
}

/*
term is 
    factor * term
    factor / term
    factor
*/
std::unique_ptr<Node> Parser::term() {
    std::unique_ptr<Node> node = factor();
    std::vector<MultiplicativePart> rest;
    while (scanner_.getToken() == EToken::TOKEN_MULTIPLY ||
           scanner_.getToken() == EToken::TOKEN_DIVIDE) {
        const EMultiplicativeOp op =
            scanner_.getToken() == EToken::TOKEN_MULTIPLY ? EMultiplicativeOp::Multiply
                                                          : EMultiplicativeOp::Divide;
        scanner_.accept();
        rest.push_back({op, factor()});
    }
    return builder_.makeMultiplicative(std::move(node), std::move(rest));
}

/*
factor is 
    NUMBER
    identifier
    identifier(expr)
    (expr)
    -factor
*/
std::unique_ptr<Node> Parser::factor() {
    switch (scanner_.getToken()) {
        case EToken::TOKEN_NUMBER: {
            const double value = scanner_.getValue();
            scanner_.accept();
            return builder_.makeNumber(value);
        }
        case EToken::TOKEN_LPAREN: {
            scanner_.accept();
            std::unique_ptr<Node> node = expr();
            if (scanner_.getToken() != EToken::TOKEN_RPAREN) {
                throw SyntaxError("Expected ')'");
            }
            scanner_.accept();
            return node;
        }
        case EToken::TOKEN_IDENTIFIER: {
            std::string symbol = scanner_.getSymbol();
            scanner_.accept();
            const FuncPtr func = env_.findFunc(symbol);
            if (scanner_.getToken() == EToken::TOKEN_LPAREN) {
                scanner_.accept();
                std::unique_ptr<Node> node = expr();
                if (scanner_.getToken() != EToken::TOKEN_RPAREN) {
                    throw SyntaxError("Expected ')'");
                }
                scanner_.accept();
                if (!func) {
                    throw UnknownFunctionError(symbol);
                }
                return builder_.makeFunction(std::move(node), func);
            }
            if (func) {
                throw SyntaxError("Expected '(' after function name");
            }
            return builder_.makeVariable(std::move(symbol), env_);
        }
        case EToken::TOKEN_MINUS:
            scanner_.accept();
            return builder_.makeNegate(factor());
        default:
            throw SyntaxError("Unexpected token");
    }
}
