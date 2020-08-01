#pragma once
#include <algorithm>
#include <cassert>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

// TODO: TEST
// TODO: build the syntax tree while parsing

// recursive descent parser inspired by https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// I built a LL(2) grammar for this, hence we need two lokaheads
class Parser {
  public:
    Parser(std::istream& in) : tz(in) {}
    void program() {
        la1 = tz.get();
        la2 = tz.get();
        if(la1.tok == IDENTIFIER && la2.tok == ASSIGNMENT) {
            assignment();
        }
        else {
            rvalue();
        }
        if(la1.tok != SEPARATOR) throw SyntaxException();
    }
    void assignment() {
        if(la1.tok != IDENTIFIER) throw SyntaxException();
        next_token();
        if(la1.tok != ASSIGNMENT) throw SyntaxException();
        next_token();
        rvalue();
    }
    void rvalue() {
        expression();
        if(la1.tok == CONV_START) {
            if(la2.tok == IDENTIFIER) {
                alpha();
            }
            else {
                beta();
            }
        }
    }
    void expression() {
        if(la1.tok == LAMBDA) {
            next_token();
            if(la1.tok != IDENTIFIER) throw SyntaxException();
            next_token();
            if(la1.tok != BODY_START) throw SyntaxException();
            next_token();
            expression();
        }
        else if(la1.tok == BRACKET_OPEN) {
            next_token();
            expression();
            if(la1.tok != BRACKET_CLOSE) throw SyntaxException();
            next_token();
            expression();
        }
        else if(la1.tok == IDENTIFIER) {
            // TODO: do something with the variable
        }
        else if(la1.tok == LITERAL) {
            // TODO: do something with the literal
        }
        else throw SyntaxException();
    }
    void alpha() {
        auto tokens = {CONV_START, IDENTIFIER, CONV_END, IDENTIFIER};
        for(auto t: tokens) {
            if(la1.tok != t) throw SyntaxException();
            next_token();
        }
    }
    void beta() {
        if(la1.tok == CONV_END) {
            next_token();
            return;
        }
        else if(la1.tok == CONV_START) {
            next_token();
            if(la1.tok == CONV_END) {
                next_token();
                return;;
            }
            else if(la1.tok == LITERAL && la2.tok == CONV_END) {
                next_token();
                next_token();
                return;
            }
            else throw SyntaxException();
        }
        else throw SyntaxException();
    }
    bool next_token() {
        la1 = la2;
        la2 = tz.get();
    }
  private:
    // lookaheads
    Token la1;
    Token la2;
    Tokenizer tz;
    Expression_ptr res;
};

