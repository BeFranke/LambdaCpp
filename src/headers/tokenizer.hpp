#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include "lambda-exceptions.hpp"

// TODO: add operations such as ==, =a (alpha equality)

// syntactic constants
// using '\' as replacement for "lambda" is stolen from Haskell
enum class Symbol {
    // (\ x . x) a > invokes beta reduction (one step)
    // (\ x . x) a -n> invokes n steps of beta reduction
    // ( \ x. x) a -> invokes beta reduction until convergence
    // (\ x . x) a -x>y invokes alpha conversion
    // Y = (\ x . x) a binds lambda expression to name 'Y'
    LAMBDA = '\\',
    BODY_START = '.',
    BRCKT_OPN = '(',
    BRCKT_CLS = ')',
    SEP = ';',
    COMMENT = '#',
    ASSIGNMENT = '=',
    CONVERSION_END = '>',
    CONVERSION_START = '-',
};

/**
 * possible types of tokens for easier syntactical parsing later on
 * "LITERAL" is included so integers can be used in the lambda expression
 * that could then get expanded by Church encoding
 * "COMMAND" is meant to represent non-lambda calculus symbols that are included in the mini-language, i.e. assignments
 * and reduction commands
 */
enum TOKEN_TYPE {
    IDENTIFIER,
    OPERATOR,
    SEPARATOR,
    LITERAL,
    COMMAND,
    UNDEF
};

class Token {
  public:
    Token() : str(""), tok(UNDEF) {}
    operator bool() const {return tok != UNDEF;}
    std::string str;
    TOKEN_TYPE tok;
};

inline bool valid_conversion_cmd(const Token& result) noexcept {
    if(result.tok != COMMAND) return false;
    std::regex rgx(R"(((-[a-zA-Z]+>[a-zA-Z]+)|((-\d*)?>)))");
    return std::regex_match(result.str, rgx);
}

inline bool reserved_symbol_start(char c) noexcept {
    switch(c) {
        case static_cast<int>(Symbol::LAMBDA):
        case static_cast<int>(Symbol::BODY_START):
        case static_cast<int>(Symbol::BRCKT_OPN):
        case static_cast<int>(Symbol::BRCKT_CLS):
        case static_cast<int>(Symbol::SEP):
        case static_cast<int>(Symbol::COMMENT):
        case static_cast<int>(Symbol::ASSIGNMENT):
        case static_cast<int>(Symbol::CONVERSION_START):
        case static_cast<int>(Symbol::CONVERSION_END):
            return true;
        default:
            return false;
    }
}

class Tokenizer {
  public:
    Tokenizer(std::istream& is) : is(is) {}
    Token get() {
        Token result = Token();
        char c;
        unsigned short count = 0;
        auto init_token = [&result, &c](TOKEN_TYPE tt) { result.tok = tt; result.str += c;};
        auto update_token = [&result, &c]() {result.str += c;};
        bool comment = false;
        while((is >> c)) {
            if(comment) continue;
            if(count == 0) {
                if(isspace(c)) continue;
                else if(isalpha(c)) {
                    // variable name
                    init_token(IDENTIFIER);
                }
                else if(c == static_cast<char>(Symbol::LAMBDA) || c == static_cast<char>(Symbol::BODY_START)) {
                    init_token(OPERATOR);
                    break;
                }
                else if(c == static_cast<char>(Symbol::BRCKT_OPN) || c == static_cast<char>(Symbol::BRCKT_CLS) ||
                        c == static_cast<char>(Symbol::SEP)) {
                    init_token(SEPARATOR);
                    break;
                }
                else if(c == static_cast<char>(Symbol::COMMENT)) {
                    comment = true;
                    continue;
                }
                else if(c == static_cast<char>(Symbol::ASSIGNMENT)) {
                    init_token(COMMAND);
                    break;
                }
                else if(c == static_cast<char>(Symbol::CONVERSION_START)) {
                    init_token(COMMAND);
                }
                else if(c == static_cast<char>(Symbol::CONVERSION_END)) {
                    init_token(COMMAND);
                    break;
                }
                else if(isdigit(c)) {
                    init_token(LITERAL);
                }
                else if(c == static_cast<char>(Symbol::CONVERSION_END)) {
                    // one step of beta reduction
                    init_token(COMMAND);
                    break;
                }
                else {
                    throw SyntaxException();
                }
            }
            else {
                if(isspace(c)) break;
                else if(comment && c == '\n') {
                    comment = false;
                }
                else if(isalpha(c) && result.tok == IDENTIFIER || isdigit(c) && result.tok == LITERAL
                    || result.tok == COMMAND) {
                    update_token();
                }
                else if(reserved_symbol_start(c)) {
                    is.unget();
                    break;
                }
                else {
                    throw SyntaxException();
                }
            }
            ++count;
        }
        if(result.tok == COMMAND && !valid_conversion_cmd(result)
            && result.str[0] != static_cast<char>(Symbol::ASSIGNMENT))
            throw SyntaxException();
        return result;
    }
  private:
    std::istream& is;
};