#include "env.h"

unsigned int Env::addSymbol(const std::string& name) {
    return symTbl_.add(name);
}
unsigned int Env::findSymbol(const std::string& name) const {
    return symTbl_.find(name);
}