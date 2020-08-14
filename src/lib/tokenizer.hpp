#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include <algorithm>
#include <set>
#include "lambda-exceptions.hpp"

/**
 * ABSTRACT:
 * This file defines the class Tokenizer, which reads characters from a
 * std::istream and parses them into Tokens.
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
    lambda = '\\',
    body_start = '.',
    bracket_open = '(',
    bracket_close = ')',
    separator = ';',
    comment = '#',
    assignment = '=',
    conversion_end = '>',
    name_definition = '\''
};

/**
 * possible types of tokens for easier syntactical parsing later on
 * "literal" is included so integers can be used in the lambda expression
 * that could then get expanded by Church encoding
 */
enum class TokenType {
    identifier,
    name,
    lambda,
    body_start,
    separator,
    bracket_open,
    bracket_close,
    literal,
    conv_end,
    assignment,
    name_define,
    undefined
};

class Token {
    /**
     * A container for a TOKEN_TYPE and a std::string.
     * The std::string gives details about the token, e.g. the name of an
     * identifier.
     */
  public:
    Token() : str(), tok(TokenType::undefined) {}
    operator bool() const {return tok != TokenType::undefined;}
    std::string str;
    TokenType tok;
};

template <typename SymbolClass = Symbol>
inline bool is_special_character(char c) noexcept {
    /**
     * returns true if c is a special character of the lambda-mini-language
     */
    switch(c) {
        case static_cast<int>(SymbolClass::lambda):
        case static_cast<int>(SymbolClass::body_start):
        case static_cast<int>(SymbolClass::bracket_open):
        case static_cast<int>(SymbolClass::bracket_close):
        case static_cast<int>(SymbolClass::separator):
        case static_cast<int>(SymbolClass::comment):
        case static_cast<int>(SymbolClass::assignment):
        case static_cast<int>(SymbolClass::conversion_end):
        case static_cast<int>(SymbolClass::name_definition):
            return true;
        default:
            return false;
    }
}

template <typename SymbolClass = Symbol>
class Tokenizer {
  public:
    /** @param is std::istream to read from */
    Tokenizer(std::istream& is) : is(is) {}
    /**
     * gets the next token from the input stream by parsing one or more
     * characters from the stream
     * @return Token-object
     */
    Token get() {
        Token result = Token();
        char c = 0;
        unsigned short count = 0;
        auto init_token = [&result, &c](TokenType tt) {
            result.tok = tt; result.str += c;
        };
        auto update_token = [&result, &c]() {result.str += c;};
        bool comment = false;
        while(is.get(c)) {
            if(count == 0) {
                if(isspace(c)) continue;
                else if(std::string s(1, c); is_reserved(s)) {
                    reserved_symbols[s]();
                    return Token();
                }
                else if(islower(c)) {
                    // variable name
                    init_token(TokenType::identifier);
                }
                else if(isupper(c)) {
                    // named function
                    init_token(TokenType::name);
                }
                else if(c == static_cast<char>(SymbolClass::lambda)) {
                    init_token(TokenType::lambda);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::body_start)) {
                    init_token(TokenType::body_start);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::bracket_open)) {
                    init_token(TokenType::bracket_open);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::bracket_close)) {
                    init_token(TokenType::bracket_close);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::separator)) {
                    init_token(TokenType::separator);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::comment)) {
		    // sets comment to true, which results in all further
		    // symbols being ignored until newline is encountered
                    comment = true;
                }
                else if(c == static_cast<char>(SymbolClass::assignment)) {
                    init_token(TokenType::assignment);
                    break;
                }
                else if(c == static_cast<char>(SymbolClass::conversion_end)) {
                    init_token(TokenType::conv_end);
                    break;
                }
                else if(isdigit(c)) {
                    init_token(TokenType::literal);
                }
                else if(c == static_cast<char>(SymbolClass::name_definition)) {
                    init_token(TokenType::name_define);
                    break;
                }
                else {
                    throw SyntaxException();
                }
            }
            else {
                if(comment) {
                    if(c == '\n') {
			// resets the comment variable so that charcters
			// get parsed once again
                        comment = false;
                        count = 0;
                    }
                    continue;
                }
                else if(isspace(c)) break;
                else if((isalpha(c) && (result.tok == TokenType::identifier
                    || result.tok == TokenType::name))
                    || (isdigit(c) && result.tok == TokenType::literal)) {
                    update_token();
                }
                else if(is_special_character<SymbolClass>(c)) {
		    // a special character always marks the beginning
		    // of a new token, therefore we put it back and
		    // return the current accumulator
                    is.unget();
                    break;
                }
                else {
                    throw SyntaxException();
                }
            }
            ++count;
        }
        if(result.tok == TokenType::identifier && is_reserved(result.str)) {
	    // reserved symbol hook: if an identifier has been reserved,
	    // execute the registered function and return an empty Token
            reserved_symbols[result.str]();
            return Token();
        }
        else if(result.tok == TokenType::identifier && (result.str == "true"
                                             || result.str == "false")) {
            // true and false are literals
            result.tok = TokenType::literal;
        }
        return result;
    }
    void register_symbol(std::string symbol,
                         std::function<void()> func) {
	/**
	 * registers symbol with the reserved-symbol mechanism:
	 * when symbol is encountered during the tokenization,
	 * execute func
	 */
        if(!(islower(symbol[0]) || symbol.size() == 1))
            throw InvalidReservedSymbol(
                    "Only lowercase words or single characters may be reserved"
                    );
        reserved_symbols[symbol] = func;
    }
    void unregister_symbol(std::string symbol) {
	// deletes symbol from the reserved symbols
        reserved_symbols.erase(symbol);
    }
  private:
    inline bool is_reserved(const std::string& str) {
        return reserved_symbols.find(str) != reserved_symbols.end();
    }
    std::istream& is;
    std::unordered_map<std::string, std::function<void()>>
        reserved_symbols;
};
