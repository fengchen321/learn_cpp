#pragma once

#include <memory>

class IAstBuilder;
class Scanner;
class Node;

enum class EStatus {
    STATUS_SUCCESS,
    STATUS_ERROR,
    STATUS_QUIT,
};

class Parser {
public:
    Parser(Scanner& scanner);
    Parser(Scanner& scanner, IAstBuilder& builder);
    ~Parser();
    EStatus parse();
    std::unique_ptr<Node> expr();
    std::unique_ptr<Node> term();
    std::unique_ptr<Node> factor();
    double calc() const;
private:
    std::unique_ptr<IAstBuilder> ownedBuilder_;
    IAstBuilder& builder_;
    Scanner& scanner_;
    std::unique_ptr<Node> tree_;
    EStatus status_;
};
