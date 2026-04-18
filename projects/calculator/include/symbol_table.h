#pragma once
/*
support variable and functions.
*/
#include <limits>
#include <map>
#include <string>
#include "serial.h"

class SymbolTable : public Serializable {
public:
    static constexpr unsigned int kInvalidSymbolId = std::numeric_limits<unsigned int>::max();
    SymbolTable() = default;
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    const SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;
    ~SymbolTable() = default;

    void serialize(Serializer& output) const override;
    void deserialize(DeSerializer& input) override;
public:
    unsigned int add(const std::string& name);
    unsigned int find(const std::string& name) const;
    void clear();
    std::string getSymbolName(unsigned int id) const;

    unsigned int currentId() const { return currentId_; }

private:
    std::map<std::string, unsigned int> dictionary_;
    unsigned int currentId_ = 0;
};
