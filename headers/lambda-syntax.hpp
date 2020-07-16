#include "lambda-exceptions.hpp"
#include "lambda-struct.hpp"
#include "tokenizer.hpp"

Expression_ptr build_syntax_tree(std::vector<token>& ts) {
    token& t = ts[0];
    Expression_ptr result;

}