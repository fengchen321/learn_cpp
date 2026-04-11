#pragma once

#include "symbol_table.h"
#include "storage.h"

class Env {
    friend class Parser;
public:
    Env(): storage_(symTbl_) {}

    Storage& getStorage() { return storage_; }
    const Storage& getStorage() const { return storage_; }
    unsigned int addSymbol(const std::string& name);
    unsigned int findSymbol(const std::string& name) const;
private:
    SymbolTable symTbl_;
    Storage storage_;
};
