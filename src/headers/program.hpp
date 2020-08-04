#pragma once
#include <unordered_map>
#include "lambda-struct.hpp"

/**
 * ABSTRACT:
 * This header defines several classes to encapsulate the relation between lambda expressions and
 * commands (assignments, alpha conversion, beta reduction).
 * The most important class is Program, which is a container for a command (in multi-line input,
 * this should be the last command the user specified) and a map of known_symbols (saving all commands that got
 * assigned a name by the user, e.g. by entering "ID = \\ x. x".
 *
 * The Command-class is itself a container for an Expression (see lambda-struct.hpp) and a Conversion, which can
 * therefore be used to represent an Expression that should be converted in a certain way.
 *
 * The class Conversion is a polymorphic class, where the base class represents the identity conversion
 * (i.e. no conversion), and its child classes represent alpha conversion and beta reduction.
 *
 * Every Conversion defines the polymorphic method "execute", which takes an Expression_ptr
 * (shared pointer to Expression) as argument, applies itself to this Expression and returns the resulting Expression
 * as Expression_ptr.
 */

class Conversion {
    /**
     * Base class, represents identity conversion
     */
    public:
    /**
     * @param ex Expression to convert
     * @return converted Expression as Expression_ptr (may be identical to ex)
     */
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
    /**
     * n-fold beta-reduction, where n is num_steps. Reduction may terminate early if convergence is reached.
     */
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
    /**
     * Container for an Expression_ptr and a Command
     */
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
    /**
     * Container for a std::unordered_map of symbols that have been assigned a name,
     * and a command that is the "resulting" (usually last) command of the input
     */
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
