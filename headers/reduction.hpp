#pragma once

#include "lambda-syntax.hpp"

Expression* reduce(Expression* ex) {
    if(ex->get_type() == VAR) return ex;
    else if(ex->get_type() == LBD) {
        auto l = static_cast<Lambda*>(ex);
        int i = 0;
        for(Expression* e1 = l->get_body(i), *e2 = l->get_body(i + 1);
            i < l->n_tail() - 1; e1 = e1 = l->get_body(i+2), e2 = l->get_body(i+3), i+=2) {
            if(e1->get_type() == LBD) {
                Lambda* lnew = new Lambda();

            }
        }
    }
}
