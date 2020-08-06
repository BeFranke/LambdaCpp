#pragma once
#include <algorithm>
#include <cassert>
#include <string>
#include <set>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"
#include "program.hpp"
#include "church-encoding.hpp"

/**
 * ABSTRACT:
 * This header contains a recursive descent parser inside the class Parser.
 * It receives Tokens from the Tokenizer (see tokenizer.hpp) and parses them into an object of class Program
 * (see statement.hpp). While parsing, it builds one lambda-syntax tree (see lambda-struct.hpp) per encountered
 * lambda expression, it also splits away assignments and beta-reduction / alpha conversion commands that then
 * get handled by the Program object.
 * The class Parser defines one method per non-terminal symbol in the grammar, as well as the utility method
 * Parser::reset for re-using the object.
 */

// recursive descent parser inspired by https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// adapted to build a syntax tree while parsing
// I built a LL(1) grammar for this, hence we only need one lookahead (here, this is cur)
class Parser {
  public:
    Parser(std::istream& in, unsigned long max_iter=0, std::set<std::string> reserved={})
        : tz(in, reserved), bound(), max_iter(max_iter) {}
    Program statement() {
        cur = tz.get();
        if(cur.tok == NAME) {
            std::string name = cur.str;
            cur = tz.get();
            res = named_operation(name);
        }
        else {
            res = rvalue();
        }
        if(cur.tok != SEPARATOR) throw SyntaxException("Malformed lambda!");
        //next_token();
        return Program(known_symbols, res);
    }
    Command named_operation(std::string& name) {
        if(cur.tok == ASSIGNMENT || cur.tok == CONV_START || cur.tok == CONV_END) {
            return name_modify(name);
        }
        else {
            if(known_symbols.find(name) == known_symbols.end()) throw SyntaxException("Unknown symbol!");
            return known_symbols[name];
        }
    }
    Command name_modify(std::string& name) {
        if(cur.tok == ASSIGNMENT) {
            cur = tz.get();
            Command e = rvalue();
            known_symbols[name] = e;
            return e;
        } else {
            if(known_symbols.find(name) == known_symbols.end()) throw SyntaxException("Unknown symbol!");
            return Command(known_symbols[name].execute(), command());
        }
    }
    Command rvalue() {
        auto e = expression();
        auto c = command();
        return Command(e, c);
    }
    Expression_ptr expression() {
        if(cur.tok == LAMBDA) {
            // backup the bound-map, because there could already be a variable with the same name as the new head
            std::unordered_map<std::string, Variable_ptr> backup_bound = bound;
            cur = tz.get();
            if(cur.tok != IDENTIFIER) throw SyntaxException();
            // build head variable
            Variable_ptr head = std::make_shared<Variable>(cur.str, true);
            bound[cur.str] = head;
            cur = tz.get();
            if(cur.tok != BODY_START) throw SyntaxException("Malformed lambda!");
            cur = tz.get();
            Expression_ptr body = expression();
            bound = backup_bound;
            return std::make_shared<Lambda>(head, body);
        }
        else if(cur.tok == BRACKET_OPEN) {
            cur = tz.get();
            Expression_ptr fst = expression();
            if(cur.tok != BRACKET_CLOSE) throw SyntaxException("unmatched bracket!");
            cur = tz.get();
            Expression_ptr snd = expression();
            return std::make_shared<Application>(fst, snd);
        }
        else if(cur.tok == IDENTIFIER) {
            if(auto v = bound.find(cur.str); v != bound.end()) {
                cur = tz.get();
                return v->second;
            }
            auto name = cur.str;
            cur = tz.get();
            return std::make_shared<Variable>(name, false);
        }
        else if(cur.tok == LITERAL) {
            auto num = stoi(cur.str);
            cur = tz.get();
            return church_encode(num);
        }
        else throw SyntaxException();
    }
    std::shared_ptr<Conversion> command() {
        if(cur.tok == CONV_START || cur.tok == CONV_END) {
            return conversion();
        }
        else return std::make_shared<Conversion>();
    }
    std::shared_ptr<Conversion> conversion() {
        if(cur.tok == CONV_START) {
            cur = tz.get();
            return conversion_long();
        }
        else if(cur.tok == CONV_END) {
            cur = tz.get();
            return std::make_shared<BetaReduction>(1);
        }
        else {
            throw SyntaxException("Malformed conversion command!");
        }
    }
    std::shared_ptr<Conversion> conversion_long() {
        if(cur.tok == IDENTIFIER) {
            return alpha();
        }
        else if(cur.tok == LITERAL || cur.tok == CONV_END) {
            return beta();
        }
        else {
            throw SyntaxException("Malformed conversion command!");
        }
    }
    std::shared_ptr<AlphaConversion> alpha() {
        auto tokens = {IDENTIFIER, CONV_END, IDENTIFIER};
        std::string names[2];
        unsigned short i = 0;
        for(auto t: tokens) {
            if(cur.tok != t) throw SyntaxException();
            if(cur.tok == IDENTIFIER) names[i++] = cur.str;
            cur = tz.get();
        }
        return std::make_shared<AlphaConversion>(names[0], names[1]);
    }
    std::shared_ptr<BetaReduction> beta() {
        if(cur.tok == LITERAL) {
            try {
                unsigned int iters = std::stol(cur.str);
                cur = tz.get();
                return std::make_shared<BetaReduction>(iters > max_iter ? max_iter : iters);
            }
            catch(std::invalid_argument&) {
                throw SyntaxException("Malformed reduction!");
            }
        }
        else if(cur.tok == CONV_END) {
            cur = tz.get();
            return std::make_shared<BetaReduction>(max_iter);
        }
        else throw SyntaxException("Malformed conversion command");
    }
    void reset() {
        bound.clear();
        known_symbols.clear();
        res = Command();
    }
  private:
    //lookahead
    Token cur;
    Tokenizer tz;
    Command res;
    std::unordered_map<std::string, Variable_ptr> bound;
    std::unordered_map<std::string, Command> known_symbols;
    unsigned long max_iter;
};
