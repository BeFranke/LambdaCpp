#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include "lambda-exceptions.hpp"

/**
 * ABSTRACT:
 * This file defines the class Tokenizer, which parses characters from a std::istream and parses them into Tokens.
 * Tokenizer::get can then be used to retrieve Tokens one by one, yielding a Token of type "UNDEF" when the end of the
 * stream is reached.
 */

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
 */
enum TOKEN_TYPE {
    IDENTIFIER,
    LAMBDA,
    BODY_START,
    SEPARATOR,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    LITERAL,
    CONV_END,
    CONV_START,
    ASSIGNMENT,
    UNDEF
};

class Token {
    /**
     * A container for a TOKEN_TYPE and a std::string.
     * The std::string gives details about the token, e.g. the name of an identifier.
     * Tokens may have the TOKEN_TYPE "UNDEF", which means the Tokenizer has reached the end of the stream.
     * Tokens of this type evaluate to "false" in a boolean context.
     */
  public:
    Token() : str(""), tok(UNDEF) {}
    operator bool() const {return tok != UNDEF;}
    std::string str;
    TOKEN_TYPE tok;
};

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
    /** @param is std::istream to read from */
    Tokenizer(std::istream& is) : is(is) {}
    /**
     * gets the next token from the input stream by parsing one or more characters from the stream
     * @return
     */
    Token get() {
        Token result = Token();
        char c;
        unsigned short count = 0;
        auto init_token = [&result, &c](TOKEN_TYPE tt) { result.tok = tt; result.str += c;};
        auto update_token = [&result, &c]() {result.str += c;};
        bool comment = false;
        while(is >> c) {
            if(comment) continue;
            if(count == 0) {
                if(isspace(c)) continue;
                else if(isalpha(c)) {
                    // variable name
                    init_token(IDENTIFIER);
                }
                else if(c == static_cast<char>(Symbol::LAMBDA)) {
                    init_token(LAMBDA);
                    break;
                }
                else if(c == static_cast<char>(Symbol::BODY_START)) {
                    init_token(BODY_START);
                    break;
                }
                else if(c == static_cast<char>(Symbol::BRCKT_OPN)) {
                    init_token(BRACKET_OPEN);
                    break;
                }
                else if(c == static_cast<char>(Symbol::BRCKT_CLS)) {
                    init_token(BRACKET_CLOSE);
                    break;
                }
                else if(c == static_cast<char>(Symbol::SEP)) {
                    init_token(SEPARATOR);
                    break;
                }
                else if(c == static_cast<char>(Symbol::COMMENT)) {
                    comment = true;
                    continue;
                }
                else if(c == static_cast<char>(Symbol::ASSIGNMENT)) {
                    init_token(ASSIGNMENT);
                    break;
                }
                else if(c == static_cast<char>(Symbol::CONVERSION_START)) {
                    init_token(CONV_START);
                    break;
                }
                else if(c == static_cast<char>(Symbol::CONVERSION_END)) {
                    init_token(CONV_END);
                    break;
                }
                else if(isdigit(c)) {
                    init_token(LITERAL);
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
                else if(isalpha(c) && result.tok == IDENTIFIER || isdigit(c) && result.tok == LITERAL) {
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
        return result;
    }
  private:
    std::istream& is;
};