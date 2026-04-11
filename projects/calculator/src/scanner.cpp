#include "scanner.h"
#include <cctype>
#include <istream>

Scanner::Scanner(std::istream& input)
    : input_(input), value_(0), curPos_(0), token_(EToken::TOKEN_ERROR) {
    accept();
}


void Scanner::accept() {
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
                // 处理标识符
                std::string identifier(1, static_cast<char>(curPos_));
                while (input_.peek() != EOF && (std::isalnum(input_.peek()) || input_.peek() == '_')) {
                    identifier += static_cast<char>(input_.get());
                }
                token_ = EToken::TOKEN_IDENTIFIER; // 这里可以进一步区分不同的标识符
            } else {
                token_ = EToken::TOKEN_ERROR; // 无效字符
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
