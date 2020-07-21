#include <unordered_map>
#include <iostream>
#include "headers/lambda-struct.hpp"
#include "headers/tokenizer.hpp"
#include "headers/lambda-syntax.hpp"

static std::unordered_map<std::string, Expression_ptr> known_symbols;

int main() {
    do {
        std::cout << ">>";
        int c;
        std::stringstream ss;
        while((c = std::cin.get()) != '\n') {
            ss << c;
        }
        auto tokens = parse(ss);
        // TODO: split assignments and put into known_symbols
        auto expr = build_syntax_tree(tokens.begin(), tokens.end());
        std::cout << expr->to_string() << "\n";
    } while(!std::cin.eof());
}
