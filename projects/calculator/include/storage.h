#pragma once
/*
store variable values and Constants.
*/
#include <string>
#include <vector>

class SymbolTable;
class Storage {
public:
    Storage() = delete;
    explicit Storage(SymbolTable& tbl);
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    const Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;
    ~Storage() = default;
public:
    bool isInit(unsigned int id) const;
    void addConstant(SymbolTable& tbl, const std::string& name, double value);
    double getValue(unsigned int id) const;
    void setValue(unsigned int id, double value);
    void clear();
    
private:
    void addValue(unsigned int id, double value);
    std::vector<double> cells_;
    std::vector<bool> inits_;
};
