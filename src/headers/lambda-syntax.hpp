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
 * It receives Tokens from the Tokenizer (see tokenizer.hpp) and parses
 * them into an object of class Program (see statement.hpp).
 * While parsing, it builds one lambda-syntax tree (see lambda-struct.hpp) per
 * encountered lambda expression, it also splits away assignments and
 * beta-reduction / alpha conversion commands that then get handled by the
 * Program object.
 * The class Parser defines one method per non-terminal symbol in the grammar.
 */

// recursive descent parser inspired by
// https://en.wikipedia.org/wiki/Recursive_descent_parser
// and https://www.geeksforgeeks.org/recursive-descent-parser/
// adapted to build a syntax tree while parsing
// I built a LL(1) grammar for this, hence we only need one lookahead
// (here, this is cur)
template <typename Container = std::set<std::string>>
class Parser {
  public:
    Parser(std::istream& in, Container& reserved, unsigned long max_iter=0)
        : tz(in, reserved), bound(), max_iter(max_iter) {}
    Parser(std::istream& in, unsigned long max_iter=0)
            : tz(in), bound(), max_iter(max_iter) {}
    Program statement() {
        cur = tz.get();
        if(cur.tok == name_define) {
            res = assignment();
        }
        else {
            res = rvalue();
        }
        if(cur.tok != separator)
            throw SyntaxException("Missing semicolon!");
        //next_token();
        return Program(known_symbols, res);
    }
  private:
    Command assignment() {
        assert(cur.tok == name_define);
        cur = tz.get();
        if(cur.tok != name)
            throw SyntaxException("Only variables starting with an "
                                  "uppercase letter may be assigned to.");
        auto name = cur.str;
        cur = tz.get();
        if(cur.tok != name_define)
            throw SyntaxException("Unclosed definition!");

        cur = tz.get();
        if(cur.tok != assignment)
            throw SyntaxException("defined symbol must be assigned to!");

        cur = tz.get();
        Command e = rvalue();
        known_symbols[name] = e;
        return e;
    }
    Command rvalue() {
        auto e = expression();
        auto c = conversion();
        return Command(e, c);
    }
    Expression_ptr expression() {
        if(cur.tok == lambda) {
            // backup the bound-map, because there could already be a variable
            // with the same name as the new head
            std::unordered_map<std::string, Variable_ptr> backup_bound = bound;
            cur = tz.get();
            if(cur.tok != identifier) throw SyntaxException();
            // build head variable
            Variable_ptr head = std::make_shared<Variable>(cur.str, true);
            bound[cur.str] = head;
            cur = tz.get();
            if(cur.tok != body_start)
                throw SyntaxException("Malformed lambda!");
            cur = tz.get();
            Expression_ptr body = expression();
            bound = backup_bound;
            return std::make_shared<Lambda>(head, body);
        }
        else if(cur.tok == bracket_open) {
            cur = tz.get();
            Expression_ptr fst;
            fst = expression();
            if(cur.tok != bracket_close)
                throw SyntaxException("unmatched bracket!");
            cur = tz.get();
            auto snd = expression();
            return std::make_shared<Application>(fst, snd);
        }
        else if(cur.tok == identifier) {
            if(auto v = bound.find(cur.str); v != bound.end()) {
                cur = tz.get();
                return v->second;
            }
            auto name = cur.str;
            cur = tz.get();
            return std::make_shared<Variable>(name, false);
        }
        else if(cur.tok == literal) {
            try {
                auto num = stoi(cur.str);
                cur = tz.get();
                return church_encode(num);
            } catch(std::invalid_argument&) {
                // literal was boolean
                auto val = cur.str;
                cur = tz.get();
                if(val == "true") return church_true();
                else return church_false();
            }
        }
        else if(cur.tok == name) {
            auto name = cur.str;
            cur = tz.get();
            if(known_symbols.find(name) == known_symbols.end())
                throw SyntaxException("Undefined symbol: " + name + "!");
            return known_symbols[name].execute();
        }
        else throw SyntaxException();
    }
    std::shared_ptr<Conversion> conversion() {
        if(cur.tok == literal || cur.tok == conv_end)
            return beta();
        else if(cur.tok == identifier)
            return alpha();
        else return std::make_shared<Conversion>();
    }
    std::shared_ptr<AlphaConversion> alpha() {
        auto tokens = {identifier, conv_end, identifier};
        std::string names[2];
        unsigned short i = 0;
        for(auto t: tokens) {
            if(cur.tok != t) throw SyntaxException();
            if(cur.tok == identifier) names[i++] = cur.str;
            cur = tz.get();
        }
        return std::make_shared<AlphaConversion>(names[0], names[1]);
    }
    std::shared_ptr<BetaReduction> beta() {
        assert(cur.tok == literal || cur.tok == conv_end);
        if(cur.tok == literal) {
            try {
                unsigned int iters = std::stol(cur.str);
                cur = tz.get();
                if(cur.tok != conv_end)
                    throw SyntaxException("Malformed beta reduction!");
                cur = tz.get();
                return std::make_shared<BetaReduction>(
                        iters > max_iter && max_iter != 0 ? max_iter : iters,
                        max_iter
                        );
            }
            catch (std::invalid_argument&) {
                throw SyntaxException("Beta Reduction can only be specified with numbers!");
            }

        }
        else {
            cur = tz.get();
            return std::make_shared<BetaReduction>(max_iter, max_iter);
        }
    }
    //lookahead
    Token cur;
    Tokenizer<Container> tz;
    Command res;
    std::unordered_map<std::string, Variable_ptr> bound;
    std::unordered_map<std::string, Command> known_symbols;
    unsigned long max_iter;
};
