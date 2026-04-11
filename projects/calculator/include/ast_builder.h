#pragma once

#include "node.h"

template <typename Op>
struct OperationPart {
    Op op;
    std::unique_ptr<Node> node;
};

using AdditivePart = OperationPart<EAdditiveOp>;
using MultiplicativePart = OperationPart<EMultiplicativeOp>;

class IAstBuilder {
public:
    virtual ~IAstBuilder() = default;

    virtual std::unique_ptr<Node> makeNumber(double value) const = 0;
    virtual std::unique_ptr<Node> makeVariable(std::string symbol, Env& env) const = 0;
    virtual std::unique_ptr<Node> makeFunction(std::unique_ptr<Node> child, FuncPtr func) const = 0;
    virtual std::unique_ptr<Node> makeNegate(std::unique_ptr<Node> child) const = 0;
    virtual std::unique_ptr<Node> makeAssign(std::unique_ptr<Node> left,
                                             std::unique_ptr<Node> right) const = 0;
    virtual std::unique_ptr<Node> makeAdditive(
        std::unique_ptr<Node> first,
        std::vector<AdditivePart> rest) const = 0;
    virtual std::unique_ptr<Node> makeMultiplicative(
        std::unique_ptr<Node> first,
        std::vector<MultiplicativePart> rest) const = 0;
};

class BinaryAstBuilder : public IAstBuilder {
public:
    std::unique_ptr<Node> makeNumber(double value) const override;
    std::unique_ptr<Node> makeVariable(std::string symbol, Env& env) const override;
    std::unique_ptr<Node> makeFunction(std::unique_ptr<Node> child, FuncPtr func) const override;
    std::unique_ptr<Node> makeNegate(std::unique_ptr<Node> child) const override;
    std::unique_ptr<Node> makeAssign(std::unique_ptr<Node> left,
                                     std::unique_ptr<Node> right) const override;
    std::unique_ptr<Node> makeAdditive(
        std::unique_ptr<Node> first,
        std::vector<AdditivePart> rest) const override;
    std::unique_ptr<Node> makeMultiplicative(
        std::unique_ptr<Node> first,
        std::vector<MultiplicativePart> rest) const override;
};

class NaryAstBuilder : public IAstBuilder {
public:
    std::unique_ptr<Node> makeNumber(double value) const override;
    std::unique_ptr<Node> makeVariable(std::string symbol, Env& env) const override;
    std::unique_ptr<Node> makeFunction(std::unique_ptr<Node> child, FuncPtr func) const override;
    std::unique_ptr<Node> makeNegate(std::unique_ptr<Node> child) const override;
    std::unique_ptr<Node> makeAssign(std::unique_ptr<Node> left,
                                     std::unique_ptr<Node> right) const override;
    std::unique_ptr<Node> makeAdditive(
        std::unique_ptr<Node> first,
        std::vector<AdditivePart> rest) const override;
    std::unique_ptr<Node> makeMultiplicative(
        std::unique_ptr<Node> first,
        std::vector<MultiplicativePart> rest) const override;
};
