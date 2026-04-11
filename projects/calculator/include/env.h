#pragma once

#include "symbol_table.h"
#include "func_table.h"
#include "storage.h"

class Env {
    friend class Parser;
public:
    Env(): funcTbl_(symTbl_), storage_(symTbl_) {}

    Storage& getStorage() { return storage_; }
    const Storage& getStorage() const { return storage_; }

    FuncPtr findFunc(const std::string& name) const;
    unsigned int addSymbol(const std::string& name);
    unsigned int findSymbol(const std::string& name) const;

private:
    SymbolTable symTbl_;
    FuncTable funcTbl_;
    Storage storage_;
};
