#include "church-encoding.hpp"

/**
 * ABSTRACT:
 * This file is used for encoding natural numbers and booleansand booleans into lambda calculus.
 * This also represents a possible extension to the framework: simple
 * mathematical operations could be supported
 */

/**
 * @param n natural number to encode
 * @return Expression pointer that represents this natural in church encoding
 */
Lambda_ptr church_encode(unsigned int n) {
    Variable_ptr f = std::make_shared<Variable>("f", true);
    Variable_ptr x = std::make_shared<Variable>("x", true);
    Expression_ptr bdy = x;
    for(unsigned int i = 0; i < n; ++i) bdy =
            std::make_shared<Application>(f, bdy);
    auto tmp = std::make_shared<Lambda>(x, bdy);
    return std::make_shared<Lambda>(f, tmp);
}

/**
 * encodes true, lambda a . lambda b . a
 */
Lambda_ptr church_true() {
    Variable_ptr a = std::make_shared<Variable>("a", true);
    Variable_ptr b = std::make_shared<Variable>("b", true);
    Lambda_ptr inner = std::make_shared<Lambda>(b, a);
    Lambda_ptr outer = std::make_shared<Lambda>(a, inner);
    return outer;
}

/**
 * encodes false, lambda a . lambda b . b
 */
Lambda_ptr church_false() {
    Variable_ptr a = std::make_shared<Variable>("a", true);
    Variable_ptr b = std::make_shared<Variable>("b", true);
    Lambda_ptr inner = std::make_shared<Lambda>(b, b);
    Lambda_ptr outer = std::make_shared<Lambda>(a, inner);
    return outer;
}
