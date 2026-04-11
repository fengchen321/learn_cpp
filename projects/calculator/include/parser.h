#pragma once

#include <memory>

class IAstBuilder;
class Scanner;
class Node;
class Env;

enum class EStatus {
    STATUS_SUCCESS,
    STATUS_ERROR,
    STATUS_QUIT,
};

class Parser {
public:
    Parser(Scanner& scanner);
    Parser(Scanner& scanner, IAstBuilder& builder);
    Parser(Scanner& scanner, Env& env);
    Parser(Scanner& scanner, IAstBuilder& builder, Env& env);
    ~Parser();
    EStatus parse();
    std::unique_ptr<Node> expr();
    std::unique_ptr<Node> term();
    std::unique_ptr<Node> factor();
    double calc() const;
private:
    std::unique_ptr<IAstBuilder> ownedBuilder_;
    std::unique_ptr<Env> ownedEnv_;
    IAstBuilder& builder_;
    Scanner& scanner_;
    Env& env_;
    std::unique_ptr<Node> tree_;
    EStatus status_;
};
