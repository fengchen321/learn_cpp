#include "ast_builder.h"

#include "node.h"

std::unique_ptr<Node> BinaryAstBuilder::makeNumber(double value) const {
    return std::make_unique<NumberNode>(value);
}

std::unique_ptr<Node> BinaryAstBuilder::makeNegate(std::unique_ptr<Node> child) const {
    return std::make_unique<NegateNode>(std::move(child));
}

std::unique_ptr<Node> BinaryAstBuilder::makeAssign(
    std::unique_ptr<Node> left,
    std::unique_ptr<Node> right) const {
    return std::make_unique<AssignNode>(std::move(left), std::move(right));
}

std::unique_ptr<Node> BinaryAstBuilder::makeAdditive(
    std::unique_ptr<Node> first,
    std::vector<AdditivePart> rest) const {
    std::unique_ptr<Node> node = std::move(first);
    for (auto& part : rest) {
        if (part.op == EAdditiveOp::Add) {
            node = std::make_unique<AddNode>(std::move(node), std::move(part.node));
        } else {
            node = std::make_unique<SubtractNode>(std::move(node), std::move(part.node));
        }
    }
    return node;
}

std::unique_ptr<Node> BinaryAstBuilder::makeMultiplicative(
    std::unique_ptr<Node> first,
    std::vector<MultiplicativePart> rest) const {
    std::unique_ptr<Node> node = std::move(first);
    for (auto& part : rest) {
        if (part.op == EMultiplicativeOp::Multiply) {
            node = std::make_unique<MultiplyNode>(std::move(node), std::move(part.node));
        } else {
            node = std::make_unique<DivideNode>(std::move(node), std::move(part.node));
        }
    }
    return node;
}

std::unique_ptr<Node> NaryAstBuilder::makeNumber(double value) const {
    return std::make_unique<NumberNode>(value);
}

std::unique_ptr<Node> NaryAstBuilder::makeNegate(std::unique_ptr<Node> child) const {
    return std::make_unique<NegateNode>(std::move(child));
}

std::unique_ptr<Node> NaryAstBuilder::makeAssign(
    std::unique_ptr<Node> left,
    std::unique_ptr<Node> right) const {
    return std::make_unique<AssignNode>(std::move(left), std::move(right));
}

std::unique_ptr<Node> NaryAstBuilder::makeAdditive(
    std::unique_ptr<Node> first,
    std::vector<AdditivePart> rest) const {
    if (rest.empty()) {
        return first;
    }

    auto node = std::make_unique<SumNode>(std::move(first));
    for (auto& part : rest) {
        node->addTerm(std::move(part.node), part.op == EAdditiveOp::Add);
    }
    return node;
}

std::unique_ptr<Node> NaryAstBuilder::makeMultiplicative(
    std::unique_ptr<Node> first,
    std::vector<MultiplicativePart> rest) const {
    if (rest.empty()) {
        return first;
    }

    auto node = std::make_unique<ProductNode>(std::move(first));
    for (auto& part : rest) {
        node->addFactor(std::move(part.node), part.op == EMultiplicativeOp::Multiply);
    }
    return node;
}
