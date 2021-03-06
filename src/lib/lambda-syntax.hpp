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
class Parser {
  public:
    /**
     * in: std::istream to parse from
     * max_iter: maximum iterations at which beta reduction is stopped, 
     * 0 for no limit
     */
    Parser(std::istream& in, unsigned long max_iter=0)
            : program(), tz(in), bound(), max_iter(max_iter) {}
    Program statement() {
	/**
	 * tries to parse one statement from the input stream
	 * returns program-object on sucess
	 */
        cur = tz.get();
        if(!cur) return program;
        if(cur.tok == TokenType::name_define) {
            program[Program::last_key] = assignment();
        }
        else {
            program[Program::last_key] = rvalue();
        }
        if(cur.tok != TokenType::separator)
            throw SyntaxException("Missing semicolon");
        //next_token();
        return program;
    }
    void register_symbol(const std::string& symbol,
                         const std::function<void()>& func) {
	/**
	 * registers symbol as a reserved symbol. 
	 * If it is encountered during the parsing process,
	 * func is called
	 */
        tz.register_symbol(symbol, func);
    }
    void unregister_symbol(const std::string& symbol) {
	/**
	 * unregisters symbol 
	 */
        tz.unregister_symbol(symbol);
    }
    Program program;
  private:
    Command assignment() {
        assert(cur.tok == TokenType::name_define);
        cur = tz.get();
        if(cur.tok != TokenType::name)
            throw SyntaxException("Only variables starting with an "
                                  "uppercase letter may be assigned to");
        auto name = cur.str;
        cur = tz.get();
        if(cur.tok != TokenType::name_define)
            throw SyntaxException("Unclosed definition");

        cur = tz.get();
        if(cur.tok != TokenType::assignment)
            throw SyntaxException("defined symbol must be assigned to");

        cur = tz.get();
        Command e = rvalue();
        program[name] = e;
        return e;
    }
    Command rvalue() {
        auto e = expression();
        auto c = conversion();
        return Command(e, c);
    }
    Expression_ptr expression() {
        if(cur.tok == TokenType::lambda) {
            // backup the bound-map, because there could already be a variable
            // with the same name as the new head
            std::unordered_map<std::string, Variable_ptr> backup_bound = bound;
            cur = tz.get();
            if(cur.tok != TokenType::identifier) throw SyntaxException();
            // build head variable
            Variable_ptr head = std::make_shared<Variable>(cur.str, true);
            bound[cur.str] = head;
            cur = tz.get();
            if(cur.tok != TokenType::body_start)
                throw SyntaxException("Malformed lambda");
            cur = tz.get();
            Expression_ptr body = expression();
            bound = backup_bound;
            return std::make_shared<Lambda>(head, body);
        }
        else if(cur.tok == TokenType::bracket_open) {
            cur = tz.get();
            Expression_ptr fst;
            fst = expression();
            if(cur.tok != TokenType::bracket_close)
                throw SyntaxException("unmatched bracket");
            cur = tz.get();
            auto snd = expression();
            return std::make_shared<Application>(fst, snd);
        }
        else if(cur.tok == TokenType::identifier) {
            if(auto v = bound.find(cur.str); v != bound.end()) {
                cur = tz.get();
                return v->second;
            }
            auto name = cur.str;
            cur = tz.get();
            return std::make_shared<Variable>(name, false);
        }
        else if(cur.tok == TokenType::literal) {
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
        else if(cur.tok == TokenType::name) {
            auto name = cur.str;
            cur = tz.get();
            if(!program.contains(name))
                throw SyntaxException("Undefined symbol: " + name);
            return program[name].execute();
        }
        else throw SyntaxException("unexpected token: " + cur.str);
    }
    std::shared_ptr<Conversion> conversion() {
        if(cur.tok == TokenType::literal || cur.tok == TokenType::conv_end)
            return beta();
        else if(cur.tok == TokenType::identifier)
            return alpha();
        else return std::make_shared<Conversion>();
    }
    std::shared_ptr<AlphaConversion> alpha() {
        auto tokens = {TokenType::identifier, TokenType::conv_end,
                       TokenType::identifier};
        std::string names[2];
        unsigned short i = 0;
        for(auto t: tokens) {
            if(cur.tok != t) throw SyntaxException();
            if(cur.tok == TokenType::identifier) names[i++] = cur.str;
            cur = tz.get();
        }
        return std::make_shared<AlphaConversion>(names[0], names[1]);
    }
    std::shared_ptr<BetaReduction> beta() {
        assert(cur.tok == TokenType::literal || cur.tok == TokenType::conv_end);
        if(cur.tok == TokenType::literal) {
            // as "true" and "false" are also literals, we need to handle
            // them somehow. Here, true = 1, false = 0 like we are all used
            // to
            unsigned int iters;
            if(cur.str == "true") iters = 1;
            else if(cur.str == "false") iters = 0;
            else {
                // as the tokenizer only assigns numbers, true and false
                // as "literal", this should never fail.
                // However, for unexpected use cases this assertion is here
                assert(std::all_of(cur.str.begin(), cur.str.end(), ::isdigit));
                iters = std::stol(cur.str);
            }
            cur = tz.get();
            if(cur.tok != TokenType::conv_end)
                throw SyntaxException("Malformed beta reduction");
            cur = tz.get();
            return std::make_shared<BetaReduction>(
                    iters > max_iter && max_iter != 0 ? max_iter : iters,
                    max_iter
                    );
        }
        else {
            cur = tz.get();
            return std::make_shared<BetaReduction>(max_iter, max_iter);
        }
    }
    //lookahead
    Token cur;
    Tokenizer<> tz;
    std::unordered_map<std::string, Variable_ptr> bound;
    unsigned long max_iter;
};
