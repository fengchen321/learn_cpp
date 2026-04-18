#include <algorithm>
#include <stdexcept>
#include "symbol_table.h"

void SymbolTable::serialize(Serializer& output) const {
    output << dictionary_.size();
    for (const auto& pair : dictionary_) {
        output << pair.first << pair.second;
    }
    output << currentId_;
}

void SymbolTable::deserialize(DeSerializer& input) {
    dictionary_.clear();
    size_t size;
    input >> size;
    for (size_t i = 0; i < size; ++i) {
        std::string name;
        unsigned int id;
        input >> name >> id;
        dictionary_.emplace(std::move(name), id);
    }
    input >> currentId_;
}

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
