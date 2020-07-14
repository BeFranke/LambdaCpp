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
    //str = _trim_string(str);
    if(str.size() == 1) {
        // single variable detected
        if(auto x = has_been_bound(str[0], bound)) return x;
        else return std::make_shared<Variable>(str[0], false);
    }
    else if(str[0] == LAMBDA) {
        // lambda expression detected
        Lambda_ptr res = std::make_shared<Lambda>();
        // index for O(1) checks if vars are bound
        long var_index[26];
        std::fill_n(var_index, 26, -1);
        // iterate head
        int i = 1;
        for(char c = str[i]; c != BODY_START; c = str[++i]) {
            if(isspace(c)) continue;
            else if(!islower(c)) throw SyntaxException(str);
            else if(var_index[index(c)] > -1) throw ReDeclarationException(str);
            else {
                res->append_head(std::make_shared<Variable>(c, true));
                var_index[index(c)] = res->n_head() - 1;
            }
        }
        if(res->n_head() == 0) throw EmptyException(str);
        size_t old_n_bound = bound.size();
        bound.insert(bound.end(), res->get_head_all().begin(), res->get_head_all().end());
        //iterate tail and build lambda recursively
        for(char c = str[++i]; i < str.size(); c = str[++i]) {
            if(isspace(c)) continue;
            else if(islower(c)) {
                // if there is an entry != -1 for c in var_index, c is a variable thats been declared in teh head -> bound
                if(var_index[index(c)] > -1)
                    res->append_tail(res->get_head(static_cast<unsigned long>(var_index[index(c)])));
                else if(Variable_ptr x = has_been_bound(c, bound)) res->append_tail(x);
                else res->append_tail(std::make_shared<Variable>(c, false));
            }
            else if(c == BRCK_OPN) {
                // brackets in lambda expression must contain a valid expression themselfs
                unsigned int begin = static_cast<unsigned int>(++i);
                c = str[i];
                for(char brckt_debt = 0; (c != BRCK_CLS || brckt_debt > 0); c = str[++i]){
                    if(i == str.size()) throw EndException(str);
                    else if(c == BRCK_OPN) ++brckt_debt;
                    else if(c == BRCK_CLS) --brckt_debt;
                }
                // for the expression in brackets we can just use a recursive call
                res->append_tail(from_string_rec(str.substr(begin, i - begin), bound));
                ++i;
            }
            else throw SyntaxException(str);
        }
        // take away the bound variables that were added for this branch
        bound.resize(old_n_bound);
        if(res->n_tail() == 0) throw EmptyException(str);
        return res;
    }
    else if(isalpha(str[0]) || str[0] == BRCK_OPN) {
        // expression of multiple variables, no lambda
        Application_ptr res = std::make_shared<Application>("anonymous");
        for(int i = 0; i < str.size(); ++i) {
            char c = str[i];
            if(isspace(c)) continue;
            if(islower(c)) res->append(std::make_shared<Variable>(c, false));
            else if(c == LAMBDA) {
                res->append(from_string_rec(str.substr(static_cast<unsigned long>(i)), bound));
                break;
            }
            else if(c == BRCK_OPN) {
                unsigned int begin = static_cast<unsigned int>(++i);
                c = str[i];
                for(char brckt_debt = 0; (c != BRCK_CLS || brckt_debt > 0); c = str[++i]){
                    if(i == str.size()) throw EndException(str);
                    else if(c == BRCK_OPN) ++brckt_debt;
                    else if(c == BRCK_CLS) --brckt_debt;
                }
                res->append(from_string_rec(str.substr(begin, i - begin), bound));
            }
            else {
                throw SyntaxException(str);
            }
        }
        return res;
    }
    else {
        // ERROR
        throw StartException(str);
    }
}

inline Expression_ptr from_string(std::string str) {
    /**
     * initial call for from_string_rec, so not every call needs to init the bound-vector
     */
    std::vector<Variable_ptr> bound;
    return from_string_rec(str, bound);
}