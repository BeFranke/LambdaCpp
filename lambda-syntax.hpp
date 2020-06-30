#pragma once
#include <cassert>
#include <string>
#include <vector>

/*
 * TODO: senseful error messages
 */

using TYPES = enum {VARIABLE, LAMBDA};

class Expression {
  public:
    Expression() = delete;
    Expression(TYPES t) : t(t) {};
    TYPES get_type() const {
        return t;
    }
    virtual std::string to_string() const = 0;
  private:
    TYPES t;
};

class Variable : public Expression {
  public:
    Variable(char name, bool bound) : Expression(VARIABLE), name(name), bound(bound) {}
    Variable() = delete;
    Variable(Variable &v) : Expression(VARIABLE), name(v.name), bound(v.bound) {}
    void set_bound(bool bound) { this->bound = bound; }
    bool is_bound() const { return bound; }
    char get_name() const { return name; }
    void rename(char name) { this->name = name; }
    std::string to_string() const {
        return "" + name;
    }
  private:
    char name;
    bool bound;
};

class Lambda : public Expression {
  public:
    Lambda() : Expression(LAMBDA), n(1), vars({new Variable('x', true)}) {
        /*
         * default constructor gives \x.x
         */
        assert(vars);
    }
    // Lambda(Lambda &l) : Expression(LAMBDA),  {};
    Lambda(Expression* vars, size_t n) : Expression(LAMBDA), vars(vars), n(n) {};
    ~Lambda() {
        delete vars;
    }
    size_t get_n() const {
        return n;
    }
    Expression* get_vars() const {
        return vars;
    }
    std::string to_string() const {
        std::string head = "\\";
        std::string tail = ".";
        Expression* e = vars;
        for(size_t i = 0; i < n; ++e, ++i) {
            tail += e->to_string() + " ";
            if(e->get_type() == VARIABLE) head += static_cast<Variable*>(e)->get_name() + " ";
        }
        return head + tail;
    }
  private:
    size_t n;
    Expression* vars;
};

Expression* from_string(std::string str) {
    if(str.length() == 1) return new Variable(str[0], false);
    else {
        bool taken[26] = {0};
        if(str[0] != '\\') throw "Error: Lambda expressions must begin with \\!";
        // iterate the head, e.g. \x y z .
        size_t i;
        for(i = 0; i < str.length(), str[i] != '.'; ++i) {
            if(str[i] == ' ') continue;                         // TODO: other whitespace
            else if(!(str[i] >= 'a' && str[i] <= 'z'))
                throw "Error: only lower-case characters are allowed as variable names!";
            else if(taken[str[i] - 'a']) throw "Error: Variable re-declared!";
            else {
                taken[str[i] - 'a'] = true;
            }
        }

        // iterate the tail
        if(++i == str.length()) throw "Error: Empty lambda tail not allowed!";
        std::vector<Expression*> expr = std::vector();
        for(; i < str.length(); ++i) {
            if(str[i] == ' ') continue;
            // lambda associates to the end
            if(str[i] == '\\') expr.push_back(from_string(str.substr(i, std::string::npos)));       // TODO: can this be done in constant time with pointers instead of substr
            else if(str[i] == '(' || str[i] == '[') {
                char c = str[i] == '(' ? ')' : ']';
                std::size_t end = str.find_first_of(c, i);
                expr.push_back(from_string(str.substr(i, end)));
                i = end;
            }
            else {
                // bound or unbound variable, determined by taken
                expr.push_back(new Variable(str[i], taken[str[i] - 'a']));
            }
        }
        // TODO: just use a memory field that gets reallocated when necessary...
        Expression* e = new Expression[expr.size()];
        return new Lambda(expr[0], expr.size());
    }
}