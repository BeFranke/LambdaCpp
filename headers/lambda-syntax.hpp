#include <algorithm>
#include <cassert>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

/**
 * WARNING: This file is a mess right now
 */

enum TYPE {LAMBDA, VARIABLE, APPLICATION, UNKNOWN};

// predicates for parts of the syntax
inline auto is_bracket_open = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == BRCK_OPN;};
inline auto is_bracket_close = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == BRCK_CLS;};
inline auto statement_end = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == SEP;};
inline auto is_lambda = [](const token& t) {return std::get<0>(t) == OPERATOR && std::get<1>(t)[0] == LAMBDA;};

inline auto match_bracket(std::vector<token>::iterator open_bracket, std::vector<token>::iterator end) {
    unsigned int bracket_depth = 0;
    for(token current = *(++open_bracket); open_bracket != end; current = *(++open_bracket)) {
        if(is_bracket_open(current)) {
            ++bracket_depth;
        }
        else if(is_bracket_close(current)) {
            if(!bracket_depth) return open_bracket;
            else --bracket_depth;
        }
    }
    throw SyntaxException();
}


Expression_ptr build_syntax_tree(std::vector<token>::iterator start, std::vector<token>::iterator end,
                                 std::vector<Variable_ptr>& bound, std::string name = "") {
    /**
     * builds a single Expression from the provided iterators
     * assignments have to be cut off beforehand
     */
    // check for brackets around the whole expression
    std::vector<token>::iterator current = start;
    if(is_bracket_open(*current)) {
        auto close = match_bracket(current, end);
        if(current == start && close == end) {
            if(start + 1 >= close) throw SyntaxException();
            // whole expression is in brackets
            return build_syntax_tree(start + 1, end - 1, bound, name);
        }
    }
    if(is_lambda(*current)) {
        // lambda
        Variable_ptr head = build_syntax_tree(start + 1, start + 2, bound);
        auto is_in = std::find(bound.begin(), bound.end(), head);
        if(is_in == bound.end()) {
            bound.push_back(head);
        }
        return std::make_shared<Lambda>(
            name,
            head,
            build_syntax_tree(start + 3, end, bound)
        );
    }
    else if(start + 1 == end) {
        // variable
        assert(std::get<0>(*start) == IDENTIFIER);
        auto is_in = std::find(bound.begin(), bound.end(), head);
        if(is_in == bound.end()) {
            bound.push_back(head);
        }
        return std::make_shared<Variable>(std::get<1>(*start), )

    }
    else {
        // expression or completly wrong syntax
    }

}
