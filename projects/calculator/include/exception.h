#pragma once

#include <exception>
#include <string>
#include <vector>

class CalcException : public std::exception {
public:
    explicit CalcException(const std::string& message);
    const char* what() const noexcept override;
    const char* stackTrace() const noexcept;
private:
    static std::string captureStackTrace();
protected:
    std::string message_;
    mutable std::string stackTraceCache_;
};

class SyntaxError : public CalcException {
public:
    explicit SyntaxError(const std::string& message);
};

class RuntimeError : public CalcException {
public:
    explicit RuntimeError(const std::string& message);
};

class DivisionByZeroError : public RuntimeError {
public:
    DivisionByZeroError();
};

class UndefinedVariableError : public RuntimeError {
public:
    explicit UndefinedVariableError(const std::string& name);
};

class UninitializedVariableError : public RuntimeError {
public:
    explicit UninitializedVariableError(const std::string& name);
};

class UnknownFunctionError : public SyntaxError {
public:
    explicit UnknownFunctionError(const std::string& name);
};

class InvalidTokenError : public SyntaxError {
public:
    explicit InvalidTokenError(char ch);
};