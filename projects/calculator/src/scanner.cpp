#include <cctype>
#include <istream>
#include "scanner.h"
#include "exception.h"

Scanner::Scanner(std::istream& input)
    : input_(input), value_(0), symbol_(), curPos_(0), token_(EToken::TOKEN_ERROR) {
    accept();
}


void Scanner::accept() {
    symbol_.clear();
    readChar();
    switch (curPos_) {
        case '+':
            token_ = EToken::TOKEN_PLUS;
            break;
        case '-':
            token_ = EToken::TOKEN_MINUS;
            break;
        case '*':
            token_ = EToken::TOKEN_MULTIPLY;
            break;
        case '/':
            token_ = EToken::TOKEN_DIVIDE;
            break;
        case '(':
            token_ = EToken::TOKEN_LPAREN;
            break;
        case ')':
            token_ = EToken::TOKEN_RPAREN;
            break;
        case '=':
            token_ = EToken::TOKEN_ASSIGN;
            break;
        case EOF:
        case '\0':
        case '\n':
        case '\r':
            token_ = EToken::TOKEN_END;
            break;
        default:
            if (std::isdigit(curPos_) || curPos_ == '.') {
                input_.putback(static_cast<char>(curPos_));
                input_ >> value_;
                token_ = EToken::TOKEN_NUMBER;
            } else if (std::isalpha(curPos_)) {
                symbol_ = std::string(1, static_cast<char>(curPos_));
                while (input_.peek() != EOF && (std::isalnum(input_.peek()) || input_.peek() == '_')) {
                    symbol_ += static_cast<char>(input_.get());
                }
                token_ = EToken::TOKEN_IDENTIFIER;
            } else {
                throw InvalidTokenError(static_cast<char>(curPos_));
            }
            break;
    }
    isEmpty_ = (token_ == EToken::TOKEN_END);
}

void Scanner::readChar() {
    // 读取下一个字符并更新 curPos_
    curPos_ = input_.get();
    while(curPos_ == ' ' || curPos_ == '\t' || curPos_ == '\n') {
        curPos_ = input_.get();
    }
}
