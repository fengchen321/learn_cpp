#include "symbol_table.h"
#include <algorithm>
#include <stdexcept>

unsigned int SymbolTable::add(const std::string& name) {
    const auto it = dictionary_.find(name);
    if (it != dictionary_.end()) {
        return it->second;
    }

    const unsigned int id = currentId_++;
    dictionary_.emplace(name, id);
    return id;
}

unsigned int SymbolTable::find(const std::string& name) const {
    const auto it = dictionary_.find(name);
    if (it == dictionary_.end()) {
        return kInvalidSymbolId;
    }
    return it->second;
}

void SymbolTable::clear() {
    dictionary_.clear();
    currentId_ = 0;
}

std::string SymbolTable::getSymbolName(unsigned int id) const {
    auto it = std::find_if(dictionary_.begin(), dictionary_.end(),
                           [id](const auto& pair) { return pair.second == id; });
    if (it == dictionary_.end()) {
        throw std::runtime_error("Symbol ID not found");
    }
    return it->first;
}
