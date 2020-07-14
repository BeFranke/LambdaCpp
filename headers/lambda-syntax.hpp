#pragma once

#include <memory>
#include "lambda-struct.hpp"

// TODO everything

static Expression_ptr from_string_rec(std::string str, std::vector<std::shared_ptr<Variable>> &bound) {
    /**
     * builds a single expression from a string, so the input should previously be split on SEP
     * THIS MAY NOT INCLUDE BINDINGS, THESE HAVE TO BE SPLITTED FROM THE 'X =' part beforehand!
     * str: string that defines the lambda expression
     * bound: vector to be filled with the variables that are bound, should be empty on top-level call
     */

}

inline Expression_ptr from_string(std::string str) {
    /**
     * initial call for from_string_rec, so not every call needs to init the bound-vector
     */
    std::vector<Variable_ptr> bound;
    return from_string_rec(str, bound);
}