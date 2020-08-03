#pragma once
#include <unordered_map>
#include "lambda-struct.hpp"

class Conversion {
  public:
    virtual Expression_ptr execute(Expression_ptr ex) const {
        return ex;
    }
};
class AlphaConversion final : public Conversion {
  public:
    AlphaConversion(std::string& old_name, std::string& new_name) : old_name(old_name), new_name(new_name) {}
    Expression_ptr execute(Expression_ptr ex) const override {
        return ex->alpha_convert(old_name, new_name);
    }
    std::string old_name;
    std::string new_name;
};

class BetaReduction : public Conversion {
  public:
    BetaReduction(unsigned int num_steps) : num_steps(num_steps) {}
    Expression_ptr execute(Expression_ptr ex) const override {
        Expression_ptr newex;
        for(unsigned int i = 0; i < num_steps; ++i) {
            newex = ex->beta_reduce();
            if(newex == ex) break;
            ex = newex;
        }
        return newex;
    }
    unsigned int num_steps;
};

class Command {
  public:
    Command() {}
    Command(Expression_ptr ex, std::shared_ptr<Conversion> c) : ex(ex), c(c) {}
    Expression_ptr execute() const {
        return c->execute(ex);
    }
    Expression_ptr ex;
    std::shared_ptr<Conversion> c;
};

class Program {
  public:
    Program(std::unordered_map<std::string, Command>& ks, Command& li) :
        known_symbols(ks), last_input(li) {}
    inline std::unordered_map<std::string, Command> symbols() {
        return known_symbols;
    }
    inline const Command& last_command() {
        return last_input;
    }
  private:
    std::unordered_map<std::string, Command> known_symbols;
    Command last_input;
};
