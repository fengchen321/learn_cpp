#include "node.h"
#include "scanner.h"
#include "parser.h"

#include <stdexcept>

namespace {

std::runtime_error UnexpectedToken() {
    return std::runtime_error("Unexpected token");
}

} // namespace

Parser::Parser(Scanner& scanner)
    : scanner_(scanner),
      tree_(nullptr),
      status_(EStatus::STATUS_SUCCESS) {}

Parser::~Parser() = default;

EStatus Parser::parse() {
    try {
        tree_ = expr();
        if (scanner_.getToken() != EToken::TOKEN_END) {
            throw UnexpectedToken();
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
        throw std::runtime_error("Parse tree is empty");
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
    // why not use switch-case or if-else? 
    while (scanner_.getToken() == EToken::TOKEN_PLUS ||
           scanner_.getToken() == EToken::TOKEN_MINUS) {
        const EToken token = scanner_.getToken();
        scanner_.accept();
        std::unique_ptr<Node> right = term();
        if (token == EToken::TOKEN_PLUS) {
            node = std::make_unique<AddNode>(std::move(node), std::move(right));
        } else {
            node = std::make_unique<SubtractNode>(std::move(node), std::move(right));
        }
    }
    if (scanner_.getToken() == EToken::TOKEN_ASSIGN) {
        scanner_.accept();
        std::unique_ptr<Node> right = expr();
        if (!node->isLvalue()) {
            throw std::runtime_error("Cannot assign to an lvalue");
        }
        node = std::make_unique<AssignNode>(std::move(node), std::move(right));
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
    while (scanner_.getToken() == EToken::TOKEN_MULTIPLY ||
           scanner_.getToken() == EToken::TOKEN_DIVIDE) {
        const EToken token = scanner_.getToken();
        scanner_.accept();
        std::unique_ptr<Node> right = factor();
        if (token == EToken::TOKEN_MULTIPLY) {
            node = std::make_unique<MultiplyNode>(std::move(node), std::move(right));
        } else {
            node = std::make_unique<DivideNode>(std::move(node), std::move(right));
        }
    }
    return node;
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
            return std::make_unique<NumberNode>(value);
        }
        case EToken::TOKEN_LPAREN: {
            scanner_.accept();
            std::unique_ptr<Node> node = expr();
            if (scanner_.getToken() != EToken::TOKEN_RPAREN) {
                throw std::runtime_error("Expected ')'");
            }
            scanner_.accept();
            return node;
        }
        case EToken::TOKEN_MINUS:
            scanner_.accept();
            return std::make_unique<NegateNode>(factor());
        default:
            throw UnexpectedToken();
    }
}
