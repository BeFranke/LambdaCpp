#include <unordered_map>
#include <iostream>
#include "headers/lambda-struct.hpp"
#include "headers/tokenizer.hpp"
#include "headers/lambda-syntax.hpp"

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
                try {
                    std::cout << expr->beta_reduce_full()->to_string() << std::endl;
                }
                catch(MaxIterationsExceeded) {
                    std::cout << "Error: MAX_ITER exceeded!" << std::endl;
                }
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
