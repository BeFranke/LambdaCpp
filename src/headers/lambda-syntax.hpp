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
 * (see program.hpp). While parsing, it builds one lambda-syntax tree (see lambda-struct.hpp) per encountered
 * lambda expression, it also splits away assignments and beta-reduction / alpha conversion commands that then
 * get handled by the Program object.
 * The class Parser defines one method per non-terminal symbol in the grammar, as well as the utility method
 * Parser::next_token, which updates the two lookaheads by one step, and Parser::reset for re-using the object.
 */

// TODO: modify grammar to allow name_get with reduction

// recursive descent parser inspired by https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// adapted to build a syntax tree while parsing
// I built a LL(1) grammar for this, hence we only need one lookahead (here, this is cur)
class Parser {
  public:
    Parser(std::istream& in) : tz(in), bound() {}
    Program program() {
        cur = tz.get();
        if(cur.tok == NAME) {
            res = name_get_or_set();
        }
        else {
            res = rvalue();
        }
        if(cur.tok != SEPARATOR) throw SyntaxException("Malformed lambda!");
        //next_token();
        return Program(known_symbols, res);
    }
    Command name_get_or_set() {
        if(cur.tok != NAME) throw SyntaxException("Assignment may only assign to variable");
        std::string name = cur.str;
        cur = tz.get();
        if(cur.tok == ASSIGNMENT) {
            return assignment(name);
        }
        return name_get(name);
    }
    Command name_get(std::string& name) {
        if(known_symbols.find(name) == known_symbols.end()) throw SyntaxException("Unknown symbol!");
        return known_symbols[name];
    }
    Command assignment(std::string& name) {
        if(cur.tok != ASSIGNMENT) throw SyntaxException("Assignment needs \"=\" symbol!");
        cur = tz.get();
        Command e = rvalue();
        known_symbols[name] = e;
        return e;
    }
    Command rvalue() {
        Expression_ptr e = expression();
        std::shared_ptr<Conversion> c;
        if(cur.tok == CONV_START || cur.tok == CONV_END) {
           c = convert();
        }
        else {
            c = std::make_shared<Conversion>();
        }
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
        else if(cur.tok == NAME){
            auto key = cur.str;
            cur = tz.get();
            // TODO: this weakens the boundary between building the syntax tree and execution,
            // can this be done better?
            return known_symbols[key].execute();
        }
        else if(cur.tok == LITERAL) {
            auto num = stoi(cur.str);
            cur = tz.get();
            return church_encode(num);
        }
        else throw SyntaxException();
    }
    std::shared_ptr<Conversion> convert() {
        if(cur.tok == CONV_START) {
            cur = tz.get();
            if(cur.tok == IDENTIFIER) return alpha();
            else return beta();
        }
        else if(cur.tok == CONV_END) {
            cur = tz.get();
            return std::make_shared<BetaReduction>(1);
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
        if(cur.tok == CONV_END) {
            cur = tz.get();
            return std::make_shared<BetaReduction>(0);
        }
        else if(cur.tok == LITERAL) {
            unsigned int cnt = std::stol(cur.str);
            cur = tz.get();
            try {
                return std::make_shared<BetaReduction>(cnt);
            }
            catch(std::invalid_argument&) {
                throw SyntaxException("Malformed reduction!");
            }
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
};
