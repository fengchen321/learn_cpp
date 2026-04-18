#include <cassert>
#include <cmath>
#include <stdexcept>
#include "func_table.h"
#include "symbol_table.h"

namespace {

struct FuncEntry {
    const char* name;
    FuncPtr func;
};

const FuncEntry kEntries[] = {
    {"log", static_cast<FuncPtr>(std::log)},
    {"log10", static_cast<FuncPtr>(std::log10)},
    {"exp", static_cast<FuncPtr>(std::exp)},
    {"sqrt", static_cast<FuncPtr>(std::sqrt)},
    {"sin", static_cast<FuncPtr>(std::sin)},
    {"cos", static_cast<FuncPtr>(std::cos)},
    {"tan", static_cast<FuncPtr>(std::tan)},
    {"asin", static_cast<FuncPtr>(std::asin)},
    {"acos", static_cast<FuncPtr>(std::acos)},
    {"atan", static_cast<FuncPtr>(std::atan)},
    {"sinh", static_cast<FuncPtr>(std::sinh)},
    {"cosh", static_cast<FuncPtr>(std::cosh)},
    {"tanh", static_cast<FuncPtr>(std::tanh)},
};

constexpr unsigned int kFunctionCount = sizeof(kEntries) / sizeof(kEntries[0]);

} // namespace

FuncTable::FuncTable(SymbolTable& tbl)
    : size_(kFunctionCount) {
    init(tbl);
}

void FuncTable::init(SymbolTable& tbl) {
    funcs_ = std::make_unique<FuncPtr[]>(size_);
    for (unsigned int i = 0; i < size_; ++i) {
        funcs_[i] = kEntries[i].func;
        const unsigned int j = tbl.add(kEntries[i].name);
        assert(j == i);
    }
}

FuncPtr FuncTable::getFunc(unsigned int id) const {
    if (id >= size_) {
        throw std::out_of_range("Invalid function id");
    }
    return funcs_[id];
}
