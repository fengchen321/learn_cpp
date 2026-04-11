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

void MultipleNode::addChild(std::unique_ptr<Node> node, bool isPositive) {
    childs_.push_back(std::move(node));
    positives_.push_back(isPositive);
}

double SumNode::calc() const {
    double result = 0;
    for (size_t i = 0; i < childs_.size(); ++i) {
        double value = childs_[i]->calc();
        if (positives_[i]) {
            result += value;
        } else {
            result -= value;
        }
    }
    return result;
}

double ProductNode::calc() const {
    double result = 1;
    for (size_t i = 0; i < childs_.size(); ++i) {
        double value = childs_[i]->calc();
        if (positives_[i]) {
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