#pragma once
#include <cassert>
#include <string>

using TYPES = enum {VARIABLE, LAMBDA};

class Expression {
  public:
    Expression() = delete;
    Expression(TYPES t) : t(t) {}
    Expression(std::string &str) : Expression(from_string(str)) {};
  private:
    TYPES t;
    Expression from_string(std::string str) {

    }
};

class Variable : public Expression {
  public:
    Variable(char name, bool bound) : Expression(VARIABLE), name(name), bound(bound) {}
    Variable() = delete;
  private:
    char name;
    bool bound;
};

class Lambda : public Expression {
  public:
    Lambda() : Expression(LAMBDA), n(1), vars(new Variable('x', true)) {
        /*
         * default constructor gives \x.x
         */
        assert(vars);
    }
    ~Lambda() {
        delete vars;
    }
  private:
    unsigned int n;
    Expression* vars;
};
