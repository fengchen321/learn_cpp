#pragma once
#include <string>
#include "parser.h"
class Scanner;
class Env;

class CommandParser {
    enum class ECommand {
        CMD_HELP,
        CMD_LIST_VARS,
        CMD_LIST_FUNCS,
        CMD_LOAD,
        CMD_SAVE,
        CMD_QUIT,
        CMD_UNKNOWN
    };
public:
    CommandParser(Scanner& scanner, Env& env);
    EStatus execute();
private:
    void help() const;
    void listVariables() const;
    void listFunctions() const;
    EStatus load(const std::string& filename);
    EStatus save(const std::string& filename) const;
private:
    Scanner& scanner_;
    Env& env_;
    ECommand command_;
    std::string commandstr_;
};


