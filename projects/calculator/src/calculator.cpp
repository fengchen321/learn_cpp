#include <iostream>
#include <sstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "commandParser.h"
#include "env.h"
#include "exception.h"

int main() {
    Env env;
    EStatus status = EStatus::STATUS_SUCCESS;
    while (status != EStatus::STATUS_QUIT) {
        std::cout << "> " << std::flush;
        Scanner scanner(std::cin);
        if (scanner.isEmpty()) {
            status = EStatus::STATUS_QUIT;
            printf("Expression is empty. Exiting...\n");
            break;
        }

        if (scanner.isCommand()) {
            CommandParser cmdParser(scanner, env);
            status = cmdParser.execute();
            continue;
        } else {
            Parser parser(scanner, env);
            try {
                status = parser.parse();
                if (status == EStatus::STATUS_SUCCESS) {
                    std::cout << "= " << parser.calc() << '\n';
                } else {
                    std::cerr << "Error: Failed to parse expression\n";
                }
            } catch (const CalcException& error) {
                status = EStatus::STATUS_ERROR;
                std::cerr << error.what() << '\n';
                std::cerr << "Stack trace:\n" << error.stackTrace() << '\n';
            } catch (const std::exception& error) {
                status = EStatus::STATUS_ERROR;
                std::cerr << error.what() << '\n';
            } catch (...) {
                status = EStatus::STATUS_ERROR;
                std::cerr << "An unknown error occurred\n";
            }
        }
    }
    return 0;
}
