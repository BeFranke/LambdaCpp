#include <algorithm>
#include <cassert>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

/**
 * WARNING: This file is a mess right now
 */

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
     * assignments have to be cut off beforehand and may be passed as "name"
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
        if(!(std::get<0>(*(current + 2)) == OPERATOR && std::get<1>(*(current + 2))[0] == BODY_START))
            throw SyntaxException();
        Variable_ptr head = std::static_pointer_cast<Variable>(build_syntax_tree(start + 1, start + 2, bound));
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
        std::string vname = std::get<1>(*start);
        auto is_in = std::find_if(bound.begin(), bound.end(),
                [vname](Variable_ptr& v) {return v->get_name().compare(vname) == 0;});
        if(is_in == bound.end()) {
            Variable_ptr v = std::make_shared<Variable>(vname, false);
            bound.push_back(v);
            return v;
        }
        return *is_in;

    }
    else if(is_bracket_open(*start)) {
        auto close = match_bracket(start, end);
        return std::make_shared<Application>(
                name,
                build_syntax_tree(start, close, bound),
                build_syntax_tree(close + 1, end, bound)
            );
    }
    else {
        // wrong syntax
        throw SyntaxException();
    }
}

Expression_ptr build_syntax_tree(std::vector<token>::iterator start, std::vector<token>::iterator end,
        std::string name = "") {
    std::vector<Variable_ptr> bound;
    return build_syntax_tree(start, end, bound, name);
}
