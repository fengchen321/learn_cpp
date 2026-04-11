#pragma once

#include <memory>

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
    ~Parser();
    EStatus parse();
    std::unique_ptr<Node> expr();
    std::unique_ptr<Node> term();
    std::unique_ptr<Node> factor();
    double calc() const;
private:
    Scanner& scanner_;
    std::unique_ptr<Node> tree_;
    EStatus status_;
};
