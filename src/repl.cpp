#include <iostream>
#include "headers/lambda-syntax.hpp"

#ifndef MAX_ITER
#define MAX_ITER 1000
#endif

int main() {
    Parser parser(std::cin);
    while(true) {
        std::cout << ">> ";
        try {
            try {
                Program p = parser.program();
                auto com = p.last_command();
                com.set_max_iter(MAX_ITER);
                auto ex = com.execute();
                std::cout << *ex << std::endl;
            } catch(EmptyException&) {
                continue;
            }
        }
        catch (SyntaxException& e) {
            std::cout << e.what() << std::endl;
        }
    }
}
