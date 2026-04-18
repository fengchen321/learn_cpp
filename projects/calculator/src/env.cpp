#include "env.h"

void Env::serialize(Serializer& output) const {
    symTbl_.serialize(output);
    storage_.serialize(output);
}

void Env::deserialize(DeSerializer& input) {
    symTbl_.deserialize(input);
    storage_.deserialize(input);
}

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

void Env::listVariables() const {
    for (unsigned int i = funcTbl_.size(); i < symTbl_.currentId(); ++i) {
        std::string name = symTbl_.getSymbolName(i);
        double value = storage_.isInit(i) ? storage_.getValue(i) : std::numeric_limits<double>::quiet_NaN();
        printf("%s = %g\n", name.c_str(), value);
    }
}

void Env::listFunctions() const {
    for (unsigned int i = 0; i < funcTbl_.size(); ++i) {
        std::string name = symTbl_.getSymbolName(i);
        printf("%s()\n", name.c_str());
    }
}