#include <cassert>
#include "scanner.h"
#include "env.h"
#include "commandParser.h"
#include "exception.h"

const std::string kVersion = "1.0.0";
CommandParser::CommandParser(Scanner& scanner, Env& env)
    : scanner_(scanner), env_(env) {
    assert(scanner_.isCommand());
    scanner_.accept();
    commandstr_ = scanner_.getSymbol();
    if (commandstr_.compare("help") == 0 || commandstr_.compare("h") == 0) {
        command_ = ECommand::CMD_HELP;
    } else if (commandstr_.compare("list_vars") == 0 || commandstr_.compare("v") == 0) {
        command_ = ECommand::CMD_LIST_VARS;
    } else if (commandstr_.compare("list_funcs") == 0 || commandstr_.compare("f") == 0) {
        command_ = ECommand::CMD_LIST_FUNCS;
    } else if (commandstr_.compare("load") == 0 || commandstr_.compare("l") == 0) {
        command_ = ECommand::CMD_LOAD;
    } else if (commandstr_.compare("save") == 0 || commandstr_.compare("s") == 0) {
        command_ = ECommand::CMD_SAVE;
    } else if (commandstr_.compare("quit") == 0 || commandstr_.compare("q") == 0) {
        command_ = ECommand::CMD_QUIT;
    } else {
        command_ = ECommand::CMD_UNKNOWN;
    }
}

EStatus CommandParser::execute() {
    EStatus status = EStatus::STATUS_SUCCESS;
    scanner_.acceptCommand();
    std::string fileName;
    switch (command_) {
        case ECommand::CMD_HELP:
            help();
            break;
        case ECommand::CMD_LIST_VARS:
            listVariables();
            break;
        case ECommand::CMD_LIST_FUNCS:
            listFunctions();
            break;
        case ECommand::CMD_LOAD:
            fileName = scanner_.getSymbol();
            status = load(fileName);
            break;
        case ECommand::CMD_SAVE:
            fileName = scanner_.getSymbol();
            status = save(fileName);
            break;
        case ECommand::CMD_QUIT:
            status = EStatus::STATUS_QUIT;
            printf("Goodbye!\n");
            break;
        case ECommand::CMD_UNKNOWN:
        default:
            printf("Unknown command: %s\n", commandstr_.c_str());
            status = EStatus::STATUS_ERROR;
    }
    return status;
}

void CommandParser::help() const {
    printf("Available commands:\n");
    printf("!help - Show this help message\n");
    printf("!list_vars - List all variables\n");
    printf("!list_funcs - List all functions\n");
    printf("!load <filename> - Load variables and functions from a file\n");
    printf("!save <filename> - Save variables and functions to a file\n");
    printf("!quit - Exit the calculator\n");
}

void CommandParser::listVariables() const {
    printf("Variable list:\n");
    env_.listVariables();
}

void CommandParser::listFunctions() const {
    printf("Function list:\n");
    env_.listFunctions();
}

EStatus CommandParser::load(const std::string& filename) {
    printf("Loading from file: %s\n", filename.c_str());
    EStatus status = EStatus::STATUS_SUCCESS;
    try {
        DeSerializer deserializer(filename);
        std::string version;
        deserializer >> version;
        if (version != kVersion) {
            throw RuntimeError("Version mismatch: expected " + kVersion + ", got " + version);
        }
        env_.deserialize(deserializer);
    } catch (const std::exception& e) {
        printf("Failed to load file: %s\n", e.what());
        status = EStatus::STATUS_ERROR;
    }
    return status;
}

EStatus CommandParser::save(const std::string& filename) const {
    printf("Saving to file: %s\n", filename.c_str());
    EStatus status = EStatus::STATUS_SUCCESS;
    try {
        Serializer serializer(filename);
        serializer << kVersion; // Write version for compatibility check
        env_.serialize(serializer);
    } catch (const std::exception& e) {
        printf("Failed to save file: %s\n", e.what());
        status = EStatus::STATUS_ERROR;
    }
    return status;
}