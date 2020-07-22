#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "lambda-exceptions.hpp"

// TODO: add operations such as ==, =a (alpha equality)

// syntactic constants
// using '\' as replacement for "lambda" is stolen from Haskell
constexpr char LAMBDA = '\\';
constexpr char BODY_START = '.';
constexpr char BRCK_OPN = '(';
constexpr char BRCK_CLS =')';
constexpr char SEP = '\n';
constexpr char COMMENT = '#';
constexpr char ASSIGNMENT = '=';

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
    UNDEF
};

typedef std::tuple<TOKEN_TYPE, std::string> token;

static inline token make_token(std::string value, TOKEN_TYPE ttype) {
    /**
     * asserts that token type could be inferred and constructs a token from it
     */
    if(ttype == UNDEF) throw SyntaxException(value);
    return std::make_tuple(ttype, value);
}

static inline void process_and_clear(unsigned int& count, std::stringstream& ss,
                              std::vector<token>& result, TOKEN_TYPE& curtok) {
    /**
     * clears the accumulating variables (count for char count, stringstream for building the string, token_type)
     * and puts a new token into result, if there is something to process
     */
    if(count > 0) {
        result.push_back(make_token(ss.str(), curtok));
        // reset the accumulating variables
        count = 0;
        ss.str(std::string());
        curtok = UNDEF;
    }
}

std::vector<token> parse(std::istream& in, char return_on = EOF) {
    /**
     * reads chars from the std::istream in, parses them into a vector of tokens to be passed to the syntactical
     * analysis
     */
    auto result = std::vector<token>();
    TOKEN_TYPE current_token = UNDEF;
    unsigned int count = 0;
    std::stringstream ss;
    char c;
    // lambda to avoid typing out the process_and_clear call with the same arguments over and over again
    auto clear = [&count, &ss, &result, &current_token]() {process_and_clear(count, ss, result, current_token);};
    auto except = [&ss] (char c) {std::string s = ss.str() + std::string(1, c); throw SyntaxException(s);};
    while(in.get(c)) {
        if(c == return_on) break;
        if(c == LAMBDA || c == BODY_START || c == ASSIGNMENT) {
            // operator is never part of another token, so previous token has ended
            clear();
            ss << c;
            current_token = OPERATOR;
            count++;
            // clear right away
            clear();
        }
        else if(c == BRCK_OPN || c == BRCK_CLS || c == SEP) {
            // same procedure as with operator
            clear();
            ss << c;
            current_token = SEPARATOR;
            count++;
            clear();
        }
        else if(c == COMMENT) {
            // we filter comments out here
            continue;
        }
        else if(isalpha(c)) {
            if(current_token == UNDEF || current_token == IDENTIFIER) {
                current_token = IDENTIFIER;
                ss << c;
                count++;
            }
            else except(c);
        }
        else if(isdigit(c)) {
            if(current_token == UNDEF) {
                // beginning of numeric literal
                current_token = LITERAL;
                ss << c;
                count++;
            } else if(current_token == IDENTIFIER) {
                // part of identifier
                ss << c;
                count++;
            }
            else except(c);
        }
        else if(isspace(c)) {
            if(current_token != UNDEF) clear();
        }
        else except(c);
    }
    clear();
    return result;
}
