#include "node.h"
#include "env.h"
#include <stdexcept>

double NumberNode::calc() const {
    return value_;
}


double VariableNode::calc() const{
    const unsigned int id = env_.findSymbol(symbol_);
    if (id == SymbolTable::kInvalidSymbolId) {
        throw std::runtime_error("Undefined variable");
    }
    if (!env_.getStorage().isInit(id)) {
        throw std::runtime_error("Variable not initialized");
    }
    return env_.getStorage().getValue(id);
}

void VariableNode::assign(double value) {
    unsigned int id = env_.findSymbol(symbol_);
    if (id == SymbolTable::kInvalidSymbolId) {
        id = env_.addSymbol(symbol_);
    }
    env_.getStorage().setValue(id, value);
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
    double value = right_->calc();
    left_->assign(value);
    return value;
}

double NegateNode::calc() const {
    return -child_->calc();
}

double FunNode::calc() const {
    return (*pfunc_)(child_->calc());
}

NaryNode::NaryNode(std::unique_ptr<Node> child) {
    appendChild(std::move(child));
}

void NaryNode::appendChild(std::unique_ptr<Node> child) {
    children_.push_back(std::move(child));
}

SumNode::SumNode(std::unique_ptr<Node> child)
    : NaryNode(std::move(child)), operations_{EAdditiveOp::Add} {}

void SumNode::addTerm(std::unique_ptr<Node> term, EAdditiveOp op) {
    appendChild(std::move(term));
    operations_.push_back(op);
}

double SumNode::calc() const {
    double result = 0;
    for (size_t i = 0; i < children_.size(); ++i) {
        double value = children_[i]->calc();
        if (operations_[i] == EAdditiveOp::Add) {
            result += value;
        } else {
            result -= value;
        }
    }
    return result;
}

ProductNode::ProductNode(std::unique_ptr<Node> child)
    : NaryNode(std::move(child)), operations_{EMultiplicativeOp::Multiply} {}

void ProductNode::addFactor(std::unique_ptr<Node> factor, EMultiplicativeOp op) {
    appendChild(std::move(factor));
    operations_.push_back(op);
}

double ProductNode::calc() const {
    double result = 1;
    for (size_t i = 0; i < children_.size(); ++i) {
        double value = children_[i]->calc();
        if (operations_[i] == EMultiplicativeOp::Multiply) {
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
