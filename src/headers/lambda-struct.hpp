#pragma once
#include <memory>
#include <sstream>
#include "lambda-exceptions.hpp"

// forward declarations
class Expression;
class Variable;
class Application;
class Lambda;
typedef std::shared_ptr<Expression> Expression_ptr;
typedef std::shared_ptr<Application> Application_ptr;
typedef std::shared_ptr<Variable> Variable_ptr;
typedef std::shared_ptr<Lambda> Lambda_ptr;

class Expression: public std::enable_shared_from_this<Expression> {
    /**
     * abstract base class for all valid expressions
     */
  public:
    /** @return true iff the expression contains a variable with a name equal to the given string */
    virtual bool check_for_name_clash(const std::string&) const noexcept = 0;

    /** @return Expression after one step of beta reduction*/
    virtual Expression_ptr beta_reduce() = 0;

    /** @return Expression where first argument was replaced by second argument*/
    virtual Expression_ptr substitute(Variable_ptr, Expression_ptr) = 0;

    /** @return Expression where bound variables with a name equal to the first argument were renamed to teh second
     * if third argument is true, throws Exception on name clash */
    virtual Expression_ptr alpha_convert(const std::string&, const std::string&) = 0;
    // virtual bool alpha_equals(const Expression& other) const noexcept = 0;
    virtual ~Expression() {}
  protected:
    Expression() {}
};

  class Variable final : public Expression {
    /**
     * Variables have a name and can be bound or not
     * e.g. x
     */
  public:
    Variable(std::string name, bool bound) : Expression(), name(name), bound(bound) {}
    bool is_bound() const noexcept {
        return bound;
    }
    std::string get_name() const noexcept {
        return name;
    }
    Expression_ptr beta_reduce() override {
        /**
         * beta reduction of variable is identity
         */
        return std::static_pointer_cast<Expression>(shared_from_this());
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        /**
         * renames if name matches and variable is bound
         */
        /*if(check_clashes) {
            if(this->check_for_name_clash(new_name)) throw NameClash();
        }*/
        if(bound && name.compare(old_name) == 0) {
            // TODO remove if this indeed never happens
            throw std::runtime_error("This should never happen! Code 555");
            //return std::make_shared<Variable>(new_name, bound);
        }
        return std::static_pointer_cast<Expression>(shared_from_this());
    }
    Expression_ptr substitute(Variable_ptr e1, Expression_ptr e2) override {
        /**
         * returns e2 if e1 matches itself, else returns copy of itself
         */
        if(this == e1.get()) return e2;
        return std::static_pointer_cast<Expression>(shared_from_this());
    }
    bool check_for_name_clash(const std::string& new_name) const noexcept override{
        return this->name.compare(new_name) == 0;
    }

  private:
    bool bound;
    std::string name;
};

class Lambda final : public Expression {
    /**
     * lambda expressions have a head and a body, e.g.
     * \ x . (y x)
     * \ head . body
     */
  public:
    Lambda(Variable_ptr head, Expression_ptr body) : head(head), body(body) {}
    Expression_ptr beta_reduce() override {
        /**
         * beta reduction needs an application, so it gets passed down to body
         */
        auto res = body->beta_reduce();
        if(res == body) return std::static_pointer_cast<Expression>(shared_from_this());
        return std::make_shared<Lambda>(head, res);
    }
    Expression_ptr substitute(Variable_ptr e1, Expression_ptr e2) override {
        /**
         * if e1 matches head, binds head and returns body
         * else passes binding on to body
         */
        if(e1 == head) return body->substitute(e1, e2);
        auto res = body->substitute(e1, e2);
        if(res == body) return std::static_pointer_cast<Expression>(shared_from_this());
        return std::make_shared<Lambda>(head, res);
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        /**
         * if head matches the new name, calls subsitute
         * else passes conversion to body
         */
        if(head->get_name().compare(old_name) == 0) {
            if(this->check_for_name_clash(new_name)) throw NameClash();
            auto new_head = std::make_shared<Variable>(new_name, true);
            auto new_body = body->substitute(head, new_head);
            return std::make_shared<Lambda>(new_head, new_body);
        }
        auto new_body = body->alpha_convert(old_name, new_name);
        if(new_body == body) return std::static_pointer_cast<Expression>(shared_from_this());
        return std::make_shared<Lambda>(head, new_body);
    }
    Variable_ptr get_head() noexcept {
        /**
         * returns head, the variable in the abstraction
         * e.g. \ x . x y -> x is head
         */
        return head;
    }
    Expression_ptr get_body() noexcept {
        /**
         * returns body, the expression after the abstraction
         * e.g. \ x . x y -> x y is body
         */
        return body;
    }
    bool check_for_name_clash(const std::string& new_name) const noexcept override {
        bool clash_head = head->check_for_name_clash(new_name);
        bool clash_body = body->check_for_name_clash(new_name);
        return clash_body && !clash_head;
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
    Application(Expression_ptr fst, Expression_ptr snd) : Expression(), fst(fst), snd(snd) {}

    Expression_ptr substitute(Variable_ptr e_old, Expression_ptr e_new) override {
        /**
         * binds e_old to e_new in both expressions of this application
         * returns new Expression
         */

        auto fst_new = fst->substitute(e_old, e_new);
        auto snd_new = snd->substitute(e_old, e_new);
        if(fst_new == fst && snd_new == snd) return std::static_pointer_cast<Expression>(shared_from_this());
        return std::make_shared<Application>(fst_new, snd_new);
    }
    Expression_ptr beta_reduce() override {
        /**
         * invokes a beta reduction:
         * if fst is a lambda, binds second to the bound variable in fst
         * otherwise, passes beta-reduction on to fst and snd
         */
        if(Lambda_ptr lbd = std::dynamic_pointer_cast<Lambda>(fst); lbd)
            return lbd->substitute(lbd->get_head(), snd);

        auto res1 = fst->beta_reduce();
        if(res1 == fst) {
            // no changes, i.e. fst is in normal form
            auto res2 = snd->beta_reduce();
            if(res2 == snd) return std::static_pointer_cast<Expression>(shared_from_this());
            return std::make_shared<Application>(fst, snd->beta_reduce());
        }
        return std::make_shared<Application>(res1, snd);
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        /**
         * renames bound variable old_name to new_name
         */
        auto res1 = fst->alpha_convert(old_name, new_name);
        auto res2 = snd->alpha_convert(old_name, new_name);
        if(res1 == fst && res2 == snd) return std::static_pointer_cast<Expression>(shared_from_this());
        return std::make_shared<Application>(res1, res2);

    }
    bool check_for_name_clash(const std::string& new_name) const noexcept override {
        return fst->check_for_name_clash(new_name) || snd->check_for_name_clash(new_name);
    }
    Expression_ptr get_first() noexcept {
        return fst;
    }
    Expression_ptr get_second() noexcept {
        return snd;
    }
  private:
    Expression_ptr fst;
    Expression_ptr snd;
};

std::ostream& operator<<(std::ostream& os, const Expression& ex) {
    if(typeid(ex) == typeid(Variable)) {
        auto v = static_cast<const Variable&>(ex);
        os << v.get_name();
    }
    else if(typeid(ex) == typeid(Lambda)) {
        auto l = static_cast<const Lambda&>(ex);
        os << '\\' << *l.get_head() << " . " << *l.get_body();
    }
    else if(typeid(ex) == typeid(Application)) {
        auto a = static_cast<const Application&>(ex);
        os << "(" << *a.get_first() << ") " << *a.get_second();
    }
    return os;
}
