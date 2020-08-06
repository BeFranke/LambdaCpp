#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include <algorithm>
#include "lambda-exceptions.hpp"

/**
 * ABSTRACT:
 * This file defines the class Tokenizer, which parses characters from a std::istream and parses them into Tokens.
 * Tokenizer::get can then be used to retrieve Tokens one by one
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
    CONVERSION_START = '-'
};

/**
 * possible types of tokens for easier syntactical parsing later on
 * "LITERAL" is included so integers can be used in the lambda expression
 * that could then get expanded by Church encoding
 */
enum TOKEN_TYPE {
    IDENTIFIER,
    NAME,
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
     */
  public:
    Token() : str(), tok(UNDEF) {}
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
    Tokenizer(std::istream& is, std::set<std::string> reserved={}) : is(is), reserved(reserved) {}
    /**
     * gets the next token from the input stream by parsing one or more characters from the stream
     * @return Token-object
     */
    Token get() {
        Token result = Token();
        char c;
        unsigned short count = 0;
        auto init_token = [&result, &c](TOKEN_TYPE tt) { result.tok = tt; result.str += c;};
        auto update_token = [&result, &c]() {result.str += c;};
        bool comment = false;
        while(is.get(c)) {
            if(count == 0) {
                if(isspace(c)) continue;
                else if(islower(c)) {
                    // variable name
                    init_token(IDENTIFIER);
                }
                else if(isupper(c)) {
                    // named function
                    init_token(NAME);
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
                else if(std::string s(1, c); is_reserved(s)) {
                    throw ReservedSymbol(s);
                }
                else {
                    throw SyntaxException();
                }
            }
            else {
                if(comment) {
                    if(c == '\n') {
                        comment = false;
                        count = 0;
                    }
                    else {
                        continue;
                    }
                }
                else if(isspace(c)) break;
                else if((isalpha(c) && (result.tok == IDENTIFIER || result.tok == NAME))
                    || (isdigit(c) && result.tok == LITERAL)) {
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
        if(result.tok == IDENTIFIER && is_reserved(result.str)) {
            throw ReservedSymbol(result.str);
        }
        return result;
    }
  private:
    inline bool is_reserved(const std::string& str) {
        return std::any_of(reserved.begin(), reserved.end(), [str](const std::string& e) { return e == str;});
    }
    std::istream& is;
    std::set<std::string> reserved;
};