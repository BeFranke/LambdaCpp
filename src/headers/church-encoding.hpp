#pragma once

#include "lambda-struct.hpp"

Lambda_ptr church_encode(unsigned int n) {
    Variable_ptr f = std::make_shared<Variable>("f", true);
    Variable_ptr x = std::make_shared<Variable>("x", true);
    Expression_ptr bdy = x;
    for(int i = 0; i < n; ++i) bdy = std::make_shared<Application>(f, bdy);
    auto tmp = std::make_shared<Lambda>(x, bdy);
    return std::make_shared<Lambda>(f, tmp);
}
