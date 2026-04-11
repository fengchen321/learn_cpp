#include "env.h"

unsigned int Env::addSymbol(const std::string& name) {
    return symTbl_.add(name);
}
unsigned int Env::findSymbol(const std::string& name) const {
    return symTbl_.find(name);
}

FuncPtr Env::findFunc(const std::string& name) const {
    const unsigned int id = findSymbol(name);
    if (id >= funcTbl_.size()) {
        return nullptr;
    }
    return funcTbl_.getFunc(id);
}
