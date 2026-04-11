#include "node.h"
#include <stdexcept>

double NumberNode::calc() const {
    return value_;
}

double VariableNode::calc() {
    return 0;
}

double AddNode::calc() const {
    return left_->calc() + right_->calc();
}

double SubtractNode::calc() const {
    return left_->calc() - right_->calc();
}

double MultiplyNode::calc() const {
    return left_->calc() * right_->calc();
}

double DivideNode::calc() const {
    double denominator = right_->calc();
    if (denominator == 0) {
        throw std::runtime_error("Division by zero");
    }
    return left_->calc() / denominator;
}

double AssignNode::calc() const {
    return 0;
}

double NegateNode::calc() const {
    return -child_->calc();
}

double FunNode::calc() const {
    return 0;
}

NaryNode::NaryNode(std::unique_ptr<Node> child) {
    appendChild(std::move(child));
}

void NaryNode::appendChild(std::unique_ptr<Node> child) {
    children_.push_back(std::move(child));
}

SumNode::SumNode(std::unique_ptr<Node> child)
    : NaryNode(std::move(child)), additionFlags_{true} {}

void SumNode::addTerm(std::unique_ptr<Node> term, bool isAddition) {
    appendChild(std::move(term));
    additionFlags_.push_back(isAddition);
}

double SumNode::calc() const {
    double result = 0;
    for (size_t i = 0; i < children_.size(); ++i) {
        double value = children_[i]->calc();
        if (additionFlags_[i]) {
            result += value;
        } else {
            result -= value;
        }
    }
    return result;
}

ProductNode::ProductNode(std::unique_ptr<Node> child)
    : NaryNode(std::move(child)), multiplicationFlags_{true} {}

void ProductNode::addFactor(std::unique_ptr<Node> factor, bool isMultiplication) {
    appendChild(std::move(factor));
    multiplicationFlags_.push_back(isMultiplication);
}

double ProductNode::calc() const {
    double result = 1;
    for (size_t i = 0; i < children_.size(); ++i) {
        double value = children_[i]->calc();
        if (multiplicationFlags_[i]) {
            result *= value;
        } else {
            if (value == 0) {
                throw std::runtime_error("Division by zero");
            }
            result /= value;
        }
    }
    return result;
}
