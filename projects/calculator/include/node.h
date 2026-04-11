#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <stdexcept>
class Node {
public:
    Node() = default;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    const Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;
    virtual ~Node() = default;
public:
    virtual double calc() const = 0;
    virtual bool isLvalue() const { return false; }
    virtual void assign([[maybe_unused]] double value) { throw std::runtime_error("Not an lvalue"); }
};

class NumberNode : public Node {
public:
    NumberNode(double value): value_(value) {}
    double calc() const override;
private:
    const double value_;
};

class BinaryNode : public Node {
public:
    BinaryNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    ~BinaryNode() = default;
protected:
    std::unique_ptr<Node> left_;
    std::unique_ptr<Node> right_;
};

class UnaryNode : public Node {
public:
    UnaryNode(std::unique_ptr<Node> child): child_(std::move(child)) {}
    ~UnaryNode() = default;
protected:
    std::unique_ptr<Node> child_;
};

class Env;
class VariableNode : public Node {
public:
    VariableNode(std::string symbol, Env& env)
        : symbol_(std::move(symbol)), env_(env) {}
    double calc() const;
    bool isLvalue() const override { return true; }
    void assign(double value) override;
private:
    std::string symbol_;
    Env& env_;
};

class AddNode : public BinaryNode {
public:
    AddNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : BinaryNode(std::move(left), std::move(right)) {}
    double calc() const override;
};

class SubtractNode : public BinaryNode {
public:
    SubtractNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : BinaryNode(std::move(left), std::move(right)) {}
    double calc() const override;
};

class MultiplyNode : public BinaryNode {
public:
    MultiplyNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : BinaryNode(std::move(left), std::move(right)) {}
    double calc() const override;
};

class DivideNode : public BinaryNode {
public:
    DivideNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : BinaryNode(std::move(left), std::move(right)) {}
    double calc() const override;
};

class AssignNode : public BinaryNode {
public:
    AssignNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : BinaryNode(std::move(left), std::move(right)) {}
    double calc() const override;
};

class NegateNode : public UnaryNode {
public:
    explicit NegateNode(std::unique_ptr<Node> child): UnaryNode(std::move(child)) {}
    double calc() const override;
};

class FunNode : public UnaryNode {
public:
    explicit FunNode(std::unique_ptr<Node> child): UnaryNode(std::move(child)) {}
    double calc() const override;
};

class NaryNode : public Node {
public:
    explicit NaryNode(std::unique_ptr<Node> child);
protected:
    void appendChild(std::unique_ptr<Node> child);
    std::vector<std::unique_ptr<Node>> children_;
};

class SumNode : public NaryNode {
public:
    explicit SumNode(std::unique_ptr<Node> child);
    void addTerm(std::unique_ptr<Node> term, bool isAddition);
    double calc() const override;
private:
    std::vector<bool> additionFlags_;
};

class ProductNode : public NaryNode {
public:
    explicit ProductNode(std::unique_ptr<Node> child);
    void addFactor(std::unique_ptr<Node> factor, bool isMultiplication);
    double calc() const override;
private:
    std::vector<bool> multiplicationFlags_;
};
