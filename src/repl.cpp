#include <iostream>
#include "lib/lambda-syntax.hpp"

#ifndef MAX_ITER
#define MAX_ITER 1000
#endif

void flush() {
    // flushing cin, from
    // https://stackoverflow.com/questions/257091/how-do-i-flush-the-cin-buffer
    std::cin.clear();
    std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n'
    );
}

int main() {
    std::set<std::string> reserved = {"exit", "?"};
    Parser parser(std::cin, reserved, MAX_ITER);
    std::cout << "This is a REPL for lambda expressions." << std::endl;
    std::cout << "To exit, type \"exit\"." << std::endl;
    std::cout << "For help, type \"?\"." << std::endl;
    while(true) {
        std::cout << ">> ";
        try {
            Program p = parser.statement();
            auto com = p.last_command();
            auto ex = com.execute();
            parser.program["Ans"] = com;
            std::cout << *ex << std::endl;
        }
        catch(ReservedSymbol& r) {
            if(r.symbol == "exit") break;
            else if(r.symbol == "?") {
                std::cout << std::endl << "USAGE:" << std::endl;
                std::cout << R"(Input any lambda-expression, e.g. "\x . (y) x")"
                        << std::endl;
                std::cout << R"(You can beta reduce an expression by "1>"
                    (1 step) or ">" (until convergence))" << std::endl;
                std::cout << R"(Assignments are possible if the assigned symbol
                            begins with uppercase and is enclosed in
                            single quotes.)" << std::endl;
                std::cout << "Examples (try them out):" << std::endl;
                std::cout << "  \\ x . x;" << std::endl;
                std::cout << "  (\\ x . x) y >;" << std::endl;
                std::cout << "  'ID' = \\x . x;" << std::endl;
                continue;
            }
        }
        catch (SyntaxException& e) {
            std::cout << e.what() << std::endl;
            flush();
        }
        catch (MaxIterationsExceeded&) {
            std::cout << "Error: Maximum iterations exceeded! "
                         "Expression does not seem to have a normal form."
                         << std::endl;
            flush();
        }
    }
}
