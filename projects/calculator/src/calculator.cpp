#include <iostream>
#include <sstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "env.h"
#include "exception.h"

int main() {
    std::string line;
    Env env;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }

        try {
            std::istringstream input(line);
            Scanner scanner(input);
            if (scanner.isDone()) {
                continue;
            }

            Parser parser(scanner, env);
            parser.parse();
            std::cout << "= " << parser.calc() << '\n';
        } catch (const CalcException& error) {
            std::cerr << error.what() << '\n';
            std::cerr << "Stack trace:\n" << error.stackTrace() << '\n';
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
        }
    }
    return 0;
}
