#pragma once
#include <algorithm>
#include <cassert>
#include <string>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"
#include "program.hpp"
#include "church-encoding.hpp"

/**
 * ABSTRACT:
 * This header contains a recursive descent parser inside the class Parser.
 * It receives Tokens from the Tokenizer (see tokenizer.hpp) and parses them into an object of class Program
 * (see program.hpp). While parsing, it builds one lambda-syntax tree (see lambda-struct.hpp) per encountered
 * lambda expression, it also splits away assignments and beta-reduction / alpha conversion commands that then
 * get handled by the Program object.
 * The class Parser defines one method per non-terminal symbol in the grammar, as well as the utility method
 * Parser::next_token, which updates the two lookaheads by one step, and Parser::reset for re-using the object.
 */


// recursive descent parser inspired by https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// adapted to build a syntax tree while parsing
// I built a LL(2) grammar for this, hence we need two lookaheads
class Parser {
  public:
    Parser(std::istream& in) : tz(in), bound() {}
    Program program() {
        la1 = tz.get();
        la2 = tz.get();
        do {
            if(la1.tok == IDENTIFIER && la2.tok == ASSIGNMENT) {
                res = assignment();
            }
            else {
                res = rvalue();
            }
            if(la1.tok != SEPARATOR) throw SyntaxException("Malformed lambda!");
            next_token();
        } while(la1);
        return Program(known_symbols, res);
    }
    Command assignment() {
        if(la1.tok != IDENTIFIER) throw SyntaxException("Assignment may only assign to variable");
        std::string name = la1.str;
        if(!isupper(name[0])) throw SyntaxException("Name-bindings must start with uppercase letter!");
        next_token();
        if(la1.tok != ASSIGNMENT) throw SyntaxException();
        next_token();
        Command e = rvalue();
        known_symbols[name] = e;
        return e;
    }
    Command rvalue() {
        Expression_ptr e = expression();
        std::shared_ptr<Conversion> c;
        if(la1.tok == CONV_START) {
            if(la2.tok == IDENTIFIER) {
                c = alpha();
            }
            else {
                c = beta();
            }
        }
        else if(la1.tok == CONV_END) {
            c = beta();
        }
        else {
            c = std::make_shared<Conversion>();
        }
        return Command(e, c);
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
            if(!islower(la1.str[0])) {
                if(known_symbols.find(la1.str) == known_symbols.end())
                    throw SyntaxException("Variables must start with lowercase letter!");
                else {
                    auto key = la1.str;
                    next_token();
                    // TODO: this weakens the boundary between building the syntax tree and execution,
                    // can this be done better?
                    return known_symbols[key].execute();
                }
            }
            if(auto v = bound.find(la1.str); v != bound.end()) {
                next_token();
                return v->second;
            }
            auto name = la1.str;
            next_token();
            return std::make_shared<Variable>(name, false);
        }
        else if(la1.tok == LITERAL) {
            auto num = stoi(la1.str);
            next_token();
            return church_encode(num);
        }
        else throw SyntaxException();
    }
    std::shared_ptr<AlphaConversion> alpha() {
        auto tokens = {CONV_START, IDENTIFIER, CONV_END, IDENTIFIER};
        std::string names[2];
        unsigned short i = 0;
        for(auto t: tokens) {
            if(la1.tok != t) throw SyntaxException();
            if(la1.tok == IDENTIFIER) names[i++] = la1.str;
            next_token();
        }
        return std::make_shared<AlphaConversion>(names[0], names[1]);
    }
    std::shared_ptr<BetaReduction> beta() {
        if(la1.tok == CONV_END) {
            next_token();
            return std::make_shared<BetaReduction>(1);
        }
        else if(la1.tok == CONV_START) {
            next_token();
            if(la1.tok == CONV_END) {
                next_token();
                return std::make_shared<BetaReduction>(0);
            }
            else if(la1.tok == LITERAL && la2.tok == CONV_END) {
                next_token();
                next_token();
                try {
                    return std::make_shared<BetaReduction>(std::stol(la1.str));
                }
                catch(std::invalid_argument) {
                    throw SyntaxException("Malformed reduction!");
                }
            }
            else throw SyntaxException("Malformed conversion command");
        }
        else throw SyntaxException();
    }
    void next_token() {
        la1 = la2;
        la2 = tz.get();
    }
    void reset() {
        bound.clear();
        known_symbols.clear();
        res = Command();
    }
  private:
    // lookaheads
    Token la1;
    Token la2;
    Tokenizer tz;
    Command res;
    std::unordered_map<std::string, Variable_ptr> bound;
    std::unordered_map<std::string, Command> known_symbols;
};
