#pragma once
#include <string>
#include <iostream>
#include <memory>
#include "lambda-exceptions.hpp"

// TODO: add operations such as ==, =a (alpha equality)

// syntactic constants
// using '\' as replacement for "lambda" is stolen from Haskell
enum class Symbol {
    // (\ x . x) a > invokes beta reduction (one step)
    // (\ x . x) a [n]> invokes n steps of beta reduction, with n = -1 meaning until convergence or max_iter
    // (\ x . x) a [x>y] invokes alpha conversion
    // Y = (\ x . x) a binds lambda expression to name 'Y'
    LAMBDA = '\\',
    BODY_START = '.',
    BRCKT_OPN = '(',
    BRCKT_CLS = ')',
    SEP = ';',
    COMMENT = '#',
    ASSIGNMENT = '=',
    CONVERSION = '>',
    BINDING_START = '[',
    BINDING_END = ']'
};

/**
 * possible types of tokens for easier sytactical parsing later on
 * "LITERAL" is included so integers can be used in the lambda expression
 * that could then get expanded by Church encoding
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
    Token(std::string str, TOKEN_TYPE tok) : str(str), tok(tok) {}
    Token() : str(""), tok(UNDEF) {}
    std::string str;
    TOKEN_TYPE tok;
};

class Tokenizer {
  public:
    Tokenizer(std::istream is) : is(is) {}
    Token get() {
        Token result();
        char c;
        while((is >> c) != EOF) {
            if(c )
        }
    }


  private:
    std::istream& is;
};