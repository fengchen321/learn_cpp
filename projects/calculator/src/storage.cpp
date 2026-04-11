#include "storage.h"
#include "symbol_table.h"

#include <cmath>
#include <stdexcept>

namespace {

void ValidateSymbolId(unsigned int id) {
    if (id == SymbolTable::kInvalidSymbolId) {
        throw std::out_of_range("Invalid symbol id");
    }
}

} // namespace

Storage::Storage(SymbolTable& tbl) {
    addConstant(tbl, "pi", 2.0 * std::acos(0.0));
    addConstant(tbl, "e", std::exp(1.0));
}

bool Storage::isInit(unsigned int id) const {
    return id != SymbolTable::kInvalidSymbolId &&
           id < cells_.size() &&
           inits_[id];
}

void Storage::addConstant(SymbolTable& tbl, const std::string& name, double value) {
    const unsigned int id = tbl.add(name);
    addValue(id, value);
}

double Storage::getValue(unsigned int id) const {
    ValidateSymbolId(id);
    if (!isInit(id)) {
        throw std::runtime_error("Value not initialized");
    }
    return cells_[id];
}

void Storage::setValue(unsigned int id, double value) {
    addValue(id, value);
}

void Storage::addValue(unsigned int id, double value) {
    ValidateSymbolId(id);
    if (id >= cells_.size()) {
        cells_.resize(id + 1);
        inits_.resize(id + 1, false);
    }
    cells_[id] = value;
    inits_[id] = true;
}

void Storage::clear() {
    cells_.clear();
    inits_.clear();
}
