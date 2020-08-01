#pragma once
#include <map>
#include "lambda-struct.hpp"

class Program {
  public:
    Program() : known_symbols(), last_input() {}
  private:
    std::map<std::string, Expression_ptr> known_symbols;
    Expression_ptr last_input;
};
