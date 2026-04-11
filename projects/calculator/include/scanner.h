#pragma once
#include <iosfwd>
#include <string>
enum class EToken {
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
    double getValue() const { return value_; }
    EToken getToken() const { return token_; }
    bool isEmpty() const { return isEmpty_; }
    bool isDone() const { return token_ == EToken::TOKEN_END; }
private:
    void readChar();
private:
    std::istream& input_;
    double value_;
    int curPos_;
    bool isEmpty_;
    EToken token_;
};
