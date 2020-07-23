#include <unordered_map>
#include <iostream>
#include "headers/lambda-struct.hpp"
#include "headers/tokenizer.hpp"
#include "headers/lambda-syntax.hpp"

#ifndef MAX_ITER
#define MAX_ITER 1000
#endif

static std::unordered_map<std::string, Expression_ptr> known_symbols;

int main() {
    do {
        std::cout << ">> ";
        try {
            auto tokens = parse(std::cin, '\n');
            // TODO: split assignments and put into known_symbols
            Expression_ptr expr;
            try {
                expr = build_syntax_tree(tokens.begin(), tokens.end());
                Expression_ptr reduced;
                int i = 0;
                do {
                    if(i > 0) expr = reduced;
                    reduced = expr->beta_reduce();
                } while(!((*reduced) == (*expr)) && ++i < MAX_ITER);
                if(i == MAX_ITER) {
                    std::cout << "Error: MAX_ITER exceeded!" << std::endl;
                }
                else std::cout << expr->beta_reduce()->to_string() << std::endl;
            }
            catch (EmptyIteratorException) {
                // User pressed enter without providing an Expression
                std::cout << "\n";
            }
        }
        catch (SyntaxException e) {
            std::cout << e.what() << "\n";
        }
    } while(!std::cin.eof());
}
