#pragma once
#include <iosfwd>
#include <string>
enum class EToken {
    TOKEN_COMMAND,
    TOKEN_END,
    TOKEN_ERROR,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN
};

class Scanner {
public:
    explicit Scanner(std::istream& input);
    void accept();
    void acceptCommand();
    double getValue() const { return value_; }
    const std::string& getSymbol() const { return symbol_; }
    EToken getToken() const { return token_; }
    bool isEmpty() const { return isEmpty_; }
    bool isDone() const { return token_ == EToken::TOKEN_END; }
    bool isCommand() const {return token_ == EToken::TOKEN_COMMAND; }
private:
    void readChar();
private:
    std::istream& input_;
    double value_;
    std::string symbol_;
    int curPos_;
    bool isEmpty_;
    EToken token_;
};
