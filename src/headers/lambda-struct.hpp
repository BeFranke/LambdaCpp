#pragma once
#include <memory>
#include <sstream>
#include "lambda-exceptions.hpp"

/**
 * ABSTRACT:
 * This file contains a class-polymorphism to represent lambda expressions.
 * User-supplied commands like "beta reduce this expression" are intentionally
 * split away into the file "statement.hpp", so that this header can be re-used
 * for other purposes more directly.
 * This header defines a base class for all valid lambda expressions, called
 * "Expression". Child classes are "Lambda" (e.g. \\ x. x), "Application"
 * (e.g. ((f) x) and "Variable" (e.g. x).
 * The important polymorphic methods are Expression::beta_reduce for invoking
 * one step of beta reduction, Expression::alpha_convert for alpha conversion.
 * The methods Expression::check_for_name_clash and Expression::substitute are
 * for internal use.
 */

// forward declarations
class Expression;
class Variable;
class Application;
class Lambda;
typedef std::shared_ptr<const Expression> Expression_ptr;
typedef std::shared_ptr<const Application> Application_ptr;
typedef std::shared_ptr<const Variable> Variable_ptr;
typedef std::shared_ptr<const Lambda> Lambda_ptr;

class Expression: public std::enable_shared_from_this<Expression> {
    /**
     * abstract base class for all valid expressions
     */
  public:
    /** @return true iff the expression contains a variable with a name equal
     * to the given string */
    virtual bool check_for_name_clash(const std::string&) const noexcept = 0;

    /** @return Expression after one step of beta reduction*/
    virtual Expression_ptr beta_reduce() const = 0;

    /** @return Expression where first argument was replaced by second
     * argument*/
    virtual Expression_ptr substitute(Variable_ptr, Expression_ptr) const = 0;

    /** @return Expression where bound variables with a name equal to the first
     * argument were renamed to the second
     * if third argument is true, throws Exception on name clash */
    virtual Expression_ptr alpha_convert(const std::string&,
                                         const std::string&) const = 0;

    virtual ~Expression() {}

    virtual std::ostream& print(std::ostream&) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Expression& ex) {
    return ex.print(os);
}

class Variable final : public Expression {
    /**
     * Variables have a name and can be bound or not
     * e.g. x
     */
  public:
    Variable(std::string name, bool bound) : Expression(), name(name),
        bound(bound) {}
    bool is_bound() const noexcept {
        return bound;
    }
    std::string get_name() const noexcept {
        return name;
    }
    Expression_ptr beta_reduce() const override {
        /**
         * beta reduction of variable is identity
         */
        return shared_from_this();
    }
    Expression_ptr alpha_convert(const std::string& old_name,
                                 const std::string& new_name) const override {
        /**
         * identity, actual renaming is done via Variable::substitute()
         */
        return shared_from_this();
    }
    Expression_ptr substitute(Variable_ptr e1, Expression_ptr e2) const override {
        /**
         * returns e2 if e1 matches itself, else returns copy of itself
         */
        if(this == e1.get()) return e2;
        return shared_from_this();
    }
    bool check_for_name_clash(const std::string& new_name) const noexcept
        override {
        return this->name == new_name;
    }
    std::ostream& print(std::ostream& os) const override{
        return os << name;
    }

  private:
    std::string name;
    bool bound;
};

class Lambda final : public Expression {
    /**
     * lambda expressions have a head and a body, e.g.
     * \ x . (y x)
     * \ head . body
     */
  public:
    Lambda(Variable_ptr head, Expression_ptr body) : head(head), body(body) {}
    Expression_ptr beta_reduce() const override {
        /**
         * beta reduction needs an application, so it gets passed down to body
         */
        auto res = body->beta_reduce();
        if(res == body) return shared_from_this();
        return std::make_shared<Lambda>(head, res);
    }
    Expression_ptr substitute(Variable_ptr e1, Expression_ptr e2) const
        override {
        /**
         * if e1 matches head, binds head and returns body
         * else passes binding on to body
         */
        auto res = body->substitute(e1, e2);
        if(e1 == head) return res;
        else if(res == body) return shared_from_this();
        return std::make_shared<Lambda>(head, res);
    }
    Expression_ptr alpha_convert(const std::string& old_name,
                                 const std::string& new_name) const override {
        /**
         * if head matches the new name, calls subsitute
         * else passes conversion to body
         */
        if(head->get_name() == old_name) {
            if(this->check_for_name_clash(new_name)) throw NameClash();
            auto new_head = std::make_shared<Variable>(new_name, true);
            auto new_body = body->substitute(head, new_head);
            return std::make_shared<Lambda>(new_head, new_body);
        }
        auto new_body = body->alpha_convert(old_name, new_name);
        if(new_body == body) return shared_from_this();
        return std::make_shared<Lambda>(head, new_body);
    }
    Variable_ptr get_head() const noexcept {
        /**
         * returns head, the variable in the abstraction
         * e.g. \ x . x y -> x is head
         */
        return head;
    }
    Expression_ptr get_body() const noexcept {
        /**
         * returns body, the expression after the abstraction
         * e.g. \ x . x y -> x y is body
         */
        return body;
    }
    bool check_for_name_clash(const std::string& new_name) const noexcept
        override {
        bool clash_head = head->check_for_name_clash(new_name);
        bool clash_body = body->check_for_name_clash(new_name);
        return clash_body && !clash_head;
    }
    std::ostream& print(std::ostream& os) const override {
        return os << "\\" << *head << " . " << *body;
    }
  private:
    Variable_ptr head;
    Expression_ptr body;
};


class Application final : public Expression {
    /**
     * two Expressions after each other
     * e.g. x y or (\ x . x) y
     */
  public:
    Application(Expression_ptr fst, Expression_ptr snd) :
        Expression(), function(fst), argument(snd) {}

    Expression_ptr substitute(Variable_ptr e_old, Expression_ptr e_new)
        const override {
        /**
         * binds e_old to e_new in both expressions of this application
         * returns new Expression
         */

        auto fst_new = function->substitute(e_old, e_new);
        auto snd_new = argument->substitute(e_old, e_new);
        if(fst_new == function && snd_new == argument)
            return shared_from_this();
        return std::make_shared<Application>(fst_new, snd_new);
    }
    Expression_ptr beta_reduce() const override {
        /**
         * invokes a beta reduction:
         * if function is a lambda, binds second to the bound variable in function
         * otherwise, passes beta-reduction on to function and argument
         */
        if(Lambda_ptr lbd = std::dynamic_pointer_cast<const Lambda>(function);
            lbd)
            return lbd->substitute(lbd->get_head(), argument);

        auto res1 = function->beta_reduce();
        if(res1 == function) {
            // no changes, i.e. function is in normal form
            auto res2 = argument->beta_reduce();
            if(res2 == argument) return shared_from_this();
            return std::make_shared<Application>(function, argument->beta_reduce());
        }
        return std::make_shared<Application>(res1, argument);
    }
    Expression_ptr alpha_convert(const std::string& old_name,
                                 const std::string& new_name) const override {
        /**
         * renames bound variable old_name to new_name
         */
        auto res1 = function->alpha_convert(old_name, new_name);
        auto res2 = argument->alpha_convert(old_name, new_name);
        if(res1 == function && res2 == argument)
            return shared_from_this();
        return std::make_shared<Application>(res1, res2);

    }
    bool check_for_name_clash(const std::string& new_name) const noexcept
        override {
        return function->check_for_name_clash(new_name) ||
               argument->check_for_name_clash(new_name);
    }
    Expression_ptr get_function() const noexcept {
        return function;
    }
    Expression_ptr get_argument() const noexcept {
        return argument;
    }
    std::ostream& print(std::ostream& os) const override {
        return os << "(" << *function << ") " << *argument;
    }
  private:
    Expression_ptr function;
    Expression_ptr argument;
};


