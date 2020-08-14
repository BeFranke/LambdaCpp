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

void help() {
    std::cout << std::endl << "USAGE:" << std::endl;
    std::cout << R"(Input any lambda-expression, e.g. "\x . (y) x;")"
              << std::endl;
    std::cout << R"(You can beta reduce an expression by "1>"
                    (1 step) or ">" (until convergence))" << std::endl;
    std::cout << "Assignments are possible if the assigned symbol begins with "
                 "uppercase and is enclosed in"
                 "single quotes." << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  \\ x . x;" << std::endl;
    std::cout << "  (\\ x . x) y >;" << std::endl;
    std::cout << "  'ID' = \\x . x;" << std::endl;
}

int main() {
    Parser parser(std::cin, MAX_ITER);
    std::cout << "This is a REPL for lambda expressions." << std::endl;
    std::cout << "To exit, type \"exit\"." << std::endl;
    std::cout << "For help, type \"?\"." << std::endl;
    parser.register_symbol("?", help);
    parser.register_symbol("exit", []() {exit(0);});
    while(true) {
        std::cout << ">> ";
        try {
            Program p = parser.statement();
            if(!bool(p)) {
		// if an empty program was returned, the user entered "?"
                flush();
                continue;
            }
            auto com = p.last_command();
            auto ex = com.execute();
	    // register last command as "Ans"
            parser.program["Ans"] = com;
            std::cout << *ex << std::endl;
        }
        catch (SyntaxException& e) {
            std::cout << e.what() << std::endl;
            flush();
        }
        catch (MaxIterationsExceeded&) {
            std::cout << "Error: Maximum iterations exceeded. "
                         "Expression does not seem to have a normal form."
                         << std::endl;
            flush();
        }
    }
}
