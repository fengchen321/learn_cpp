#pragma once

#include <memory>

using FuncPtr = double (*)(double);

class SymbolTable;

class FuncTable {
public:
    explicit FuncTable(SymbolTable& tbl);
    ~FuncTable() = default;

    void init(SymbolTable& tbl);
    FuncPtr getFunc(unsigned int id) const;
    unsigned int size() const { return size_; }

private:
    std::unique_ptr<FuncPtr[]> funcs_;
    unsigned int size_;
};
