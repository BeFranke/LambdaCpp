#include <iostream>
#include "headers/lambda-syntax.hpp"

#ifndef MAX_ITER
#define MAX_ITER 1000
#endif

int main() {
    std::set<std::string> reserved = {"exit", "?"};
    Parser parser(std::cin, reserved, MAX_ITER);
    std::cout << "This is a REPL for lambda expressions." << std::endl << "To exit, type \"exit\"." << std::endl;
    std::cout << "For help, type \"?\"." << std::endl;
    while(true) {
        std::cout << ">> ";
        try {
            Program p = parser.statement();
            auto com = p.last_command();
            auto ex = com.execute();
            std::cout << *ex << std::endl;
        }
        catch(ReservedSymbol& r) {
            if(r.symbol == "exit") break;
            else if(r.symbol == "?") {
                std::cout << std::endl << "USAGE:" << std::endl;
                std::cout << R"(Input any lambda-expression, e.g. "\x . (y) x")" << std::endl;
                std::cout << R"(You can beta reduce an expression by ">" (one step) or "->" (until convergence))"
                    << std::endl;
                std::cout << R"(Assignments are possible if the assigned symbol begins with uppercase.)" << std::endl;
                std::cout << "Examples (try them out):" << std::endl;
                std::cout << "  \\ x . x;" << std::endl;
                std::cout << "  (\\ x . x) y >;" << std::endl;
                std::cout << "  ID = \\x . x;" << std::endl;
                continue;
            }
        }
        catch (SyntaxException& e) {
            std::cout << e.what() << std::endl;
            // flushing cin, from https://stackoverflow.com/questions/257091/how-do-i-flush-the-cin-buffer
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}
