#pragma once
#include <algorithm>
#include <cassert>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

// TODO: TEST
// TODO: build the syntax tree while parsing
// TODO conversion hierarchy

// recursive descent parser inspired by https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// I built a LL(2) grammar for this, hence we need two lookaheads
class Parser {
  public:
    Parser(std::istream& in) : tz(in), bound() {}
    void program() {
        la1 = tz.get();
        la2 = tz.get();
        if(la1.tok == IDENTIFIER && la2.tok == ASSIGNMENT) {
            res = assignment();
        }
        else {
            res = rvalue();
        }
        if(la1.tok != SEPARATOR) throw SyntaxException();
    }
    Expression_ptr assignment() {
        if(la1.tok != IDENTIFIER) throw SyntaxException();
        std::string name = la1.str;
        next_token();
        if(la1.tok != ASSIGNMENT) throw SyntaxException();
        next_token();
        Expression_ptr e = rvalue();
        known_symbols[name] = res;
        return e;
    }
    Expression_ptr rvalue() {
        Expression_ptr e = expression();
        if(la1.tok == CONV_START) {
            if(la2.tok == IDENTIFIER) {
                alpha();
            }
            else {
                beta();
            }
        }
        return e;
    }
    Expression_ptr expression() {
        if(la1.tok == LAMBDA) {
            // backup the bound-map, because there could already be a variable with the same name as the new head
            std::unordered_map<std::string, Variable_ptr> backup_bound = bound;
            next_token();
            if(la1.tok != IDENTIFIER) throw SyntaxException();
            // build head variable
            Variable_ptr head = std::make_shared<Variable>(la1.str, true);
            bound[la1.str] = head;
            next_token();
            if(la1.tok != BODY_START) throw SyntaxException();
            next_token();
            Expression_ptr body = expression();
            bound = backup_bound;
            return std::make_shared<Lambda>(head, body);
        }
        else if(la1.tok == BRACKET_OPEN) {
            next_token();
            Expression_ptr fst = expression();
            if(la1.tok != BRACKET_CLOSE) throw SyntaxException();
            next_token();
            Expression_ptr snd = expression();
            return std::make_shared<Application>(fst, snd);
        }
        else if(la1.tok == IDENTIFIER) {
            if(auto v = bound.find(la1.str); v != bound.end()) {
                return v->second;
            }
            return std::make_shared<Variable>(la1.str, false);
        }
        else if(la1.tok == LITERAL) {
            throw std::runtime_error("Not implemented yet");
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
    std::unordered_map<std::string, Variable_ptr> bound;
    std::unordered_map<std::string, Expression_ptr> known_symbols;
    Token command;
};

