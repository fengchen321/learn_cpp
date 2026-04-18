#pragma once

#include "symbol_table.h"
#include "func_table.h"
#include "storage.h"
#include "serial.h"

class Env : public Serializable {
    friend class Parser;
public:
    Env(): funcTbl_(symTbl_), storage_(symTbl_) {}

    void serialize(Serializer& output) const override;
    void deserialize(DeSerializer& input) override;

    Storage& getStorage() { return storage_; }
    const Storage& getStorage() const { return storage_; }

    FuncPtr findFunc(const std::string& name) const;
    unsigned int addSymbol(const std::string& name);
    unsigned int findSymbol(const std::string& name) const;

    void listVariables() const;
    void listFunctions() const;
private:
    SymbolTable symTbl_;
    FuncTable funcTbl_;
    Storage storage_;
};
