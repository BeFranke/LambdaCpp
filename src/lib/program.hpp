#pragma once
#include <unordered_map>
#include "lambda-struct.hpp"

/**
 * ABSTRACT:
 * This header defines several classes to encapsulate the relation between
 * lambda expressions and commands (assignments, alpha conversion,
 * beta reduction).
 * The most important class is Program, which is a container for a command
 * (in multi-line input, this should be the last command the user specified)
 * and a map of known_symbols (saving all commands that got assigned a name by
 * the user, e.g. by entering "'ID' = \\ x. x".
 *
 * The Command-class is itself a container for an Expression (see
 * lambda-struct.hpp) and a Conversion, which can therefore be used to represent
 * an Expression that should be converted in a certain way.
 *
 * The class Conversion is a polymorphic class, where the base class represents
 * the identity conversion (i.e. no conversion), and its child classes represent
 * alpha conversion and beta reduction.
 *
 * Every Conversion defines the polymorphic method "execute", which takes an
 * Expression_ptr (shared pointer to Expression) as argument, applies itself to
 * this Expression and returns the resulting Expression as Expression_ptr.
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
    /**
     * old_name: name of the variable that should be renamed
     * new_name: new name of this variable after conversion
     */
    AlphaConversion(std::string& old_name, std::string& new_name) :
        old_name(old_name), new_name(new_name) {}
    Expression_ptr execute(Expression_ptr ex) const override {
        return ex->alpha_convert(old_name, new_name);
    }
    std::string old_name;
    std::string new_name;
};

class BetaReduction : public Conversion {
    /**
     * n-fold beta-reduction, where n is num_steps. Reduction may terminate
     * early if convergence is reached.
     */
  public:
    BetaReduction(unsigned long num_steps, unsigned long max_iter) :
        num_steps(num_steps), max_iter(max_iter) {}
    Expression_ptr execute(Expression_ptr ex) const override {
        Expression_ptr newex;
        unsigned int i;
        for(i = 0; (i < num_steps || num_steps == 0)
            && (i < max_iter || max_iter == 0); ++i) {
            newex = ex->beta_reduce();
            if(newex == ex) break;
            ex = newex;
        }
        if(i == max_iter && max_iter != 0) {
            throw MaxIterationsExceeded();
        }
        return newex;
    }
    unsigned long num_steps;
    unsigned long max_iter;
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
     * Container for a std::unordered_map of symbols that have been assigned a
     * name. The field last_key stores the special key that is used to store the
     * last command executed
     */
  public:
    Program() : known_symbols() {}
    inline Command& last_command() {
	/**
	 * returns last encountered command
	 */
        return known_symbols[last_key];
    }
    Command& operator[](const std::string& key) {
	/**
	 * random access to all known symbols
	 */
        return known_symbols[key];
    }
    Command& operator[](std::string& key) {
        return known_symbols[key];
    }
    inline bool contains(const std::string key) const {
	/**
	 * returns true if key is the name of a named command
	 */
        return known_symbols.find(key) != known_symbols.end();
    }
    inline explicit operator bool() {
	/**
	 * simple check if any known symbols exist
	 */
        return !known_symbols.empty();
    }
    static const std::string last_key;
  private:
    std::unordered_map<std::string, Command> known_symbols;
};

const std::string Program::last_key = "last";
