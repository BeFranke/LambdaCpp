#include <algorithm>
#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

enum TYPE {LAMBDA, VARIABLE, APPLICATION, UNKNOWN};

// predicates for parts of the syntax
inline auto is_bracket_open = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == BRCK_OPN;};
inline auto is_bracket_close = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == BRCK_CLS;};
inline auto statement_end = [](const token& t) {return std::get<0>(t) == SEPARATOR && std::get<1>(t)[0] == SEP;};

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

TYPE infer_type(std::vector<token>::iterator start, std::vector<token>::iterator end) {
    /**
     * infers the type of the expression for easier parsing afterwards
     */
    std::vector<token>::iterator current = start;
    if(is_bracket_open(*current)) {
        auto close = match_bracket(current, end);
        if(current == start && close == end) {
            if(start + 1 >= close) throw SyntaxException();
            // whole expression is in brackets
            return infer_type(start + 1, end - 1);
        }
        else return APPLICATION;
    }

}

Expression_ptr build_syntax_tree(std::vector<token>::iterator start, std::vector<token>::iterator end) {

}

std::vector<Expression_ptr> process_statements(std::vector<token> input) {
    std::vector<Expression_ptr> result;
    std::vector<token>::iterator start = input.begin();
    for(std::vector<token>::iterator it = start; it < input.end(); ++it) {
        if(statement_end(*it)) {
            result.push_back(build_syntax_tree(start, it));
            if(++it < input.end()) {
                start = it;
            }
        }
    }
}
