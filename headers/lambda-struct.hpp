#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include "lambda-exceptions.hpp"

typedef unsigned long ID;

// counter for unique ids for variables (currently IDs are not used)
static ID _next_id = 0;

// forward definitions
class Expression;
class Variable;
class Application;
class Lambda;
typedef std::shared_ptr<Expression> Expression_ptr;
typedef std::shared_ptr<Application> Application_ptr;
typedef std::shared_ptr<Variable> Variable_ptr;

class Expression {
    /**
     * abstract base class for all valid expressions
     */
  public:
    Expression() = delete;
    Expression(std::string name) : name(name) {}
    virtual std::string to_string() const = 0;
    std::string get_name() const {
        return name;
    }
    void set_name(std::string name) noexcept {
        this->name = name;
    }
    virtual Expression_ptr beta_reduce() = 0;
    virtual Expression_ptr bind(Variable_ptr, Expression_ptr) = 0;
    virtual Expression_ptr alpha_convert(const std::string&, const std::string&) = 0;
    virtual Variable_ptr get_head() = 0;

  protected:
    std::string name;
};

class Application final : public Expression {
    /**
     * two Expressions after each other
     * e.g. x y or (\\ x . x) y
     */
  public:
    Application(std::string name) : Expression(name), fst(), snd() {}
    Application(std::string name, Expression_ptr fst, Expression_ptr snd) : Expression(name), fst(fst), snd(snd) {}
    Expression_ptr& get_first() noexcept {
        return fst;
    }
    Expression_ptr& get_second() noexcept {
        return snd;
    }
    Expression_ptr bind(Variable_ptr e_old, Expression_ptr e_new) override {
        return std::make_shared<Application>(name, fst->bind(e_old, e_new), snd->bind(e_old, e_new));
    }
    Expression_ptr beta_reduce() override {
        // if it has a head, it is a lambda
        if(auto h = fst->get_head(); h != nullptr) return fst->bind(h, snd);
        else return std::make_shared<Application>(name, fst->beta_reduce(), snd->beta_reduce());
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        Expression_ptr res = std::make_shared<Application>(name, fst->alpha_convert(old_name, new_name),
                snd->alpha_convert(old_name, new_name));
        return res;
    }
    std::string to_string() const override {
        std::stringstream ss;
        ss << "( " << fst->to_string() << " " << snd->to_string() << ") ";
        return ss.str();
    }
    Variable_ptr get_head() override {
        return std::shared_ptr<Variable>(nullptr);
    }
  private:
    Expression_ptr fst;
    Expression_ptr snd;

};

class Variable final : public Expression {
    /**
     * Variables are named as a single lower case char and can be bound or not
     * e.g. x
     */
  public:
    Variable() = delete;
    Variable(std::string name, bool bound) : Expression(name), bound(bound), id(_next_id++) {}
    Variable(std::string name, bool bound, ID id) : Expression(name), bound(bound), id(id) {}
    bool is_bound() const noexcept {
        return bound;
    }
    ID get_id() const noexcept {
        return id;
    }
    std::string to_string() const override {
        return name;
    }
    Expression_ptr beta_reduce() override {
        return std::make_shared<Variable>(name, bound);
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        std::string name;
        if(bound && this->name.compare(old_name) == 0) name = new_name;
        else name = this->name;
        return std::make_shared<Variable>(name, bound, id);

    }
    Expression_ptr bind(Variable_ptr e1, Expression_ptr e2) override {
        if(id == e1->id) return e2;
        //else return std::shared_ptr<Variable>(this);
        else return std::make_shared<Variable>(name, bound, id);
    }
    Variable_ptr get_head() override {
        return std::shared_ptr<Variable>(nullptr);
    }

  private:
    bool bound;
    ID id;
};

class Lambda : public Expression {
    /**
     * lambda expressions have a head and a body, e.g.
     * \ x . (y x)
     * \ head . body
     */
  public:
    Lambda() = delete;
    Lambda(std::string name, Variable_ptr head, Expression_ptr body) : head(head), body(body), Expression(name) {}
    std::string to_string() const override {
        std::stringstream ss;
        ss << "(\\ " << head->to_string() << " . " << body->to_string() << ")";
        return ss.str();
    }
    Expression_ptr beta_reduce() override {
        return std::make_shared<Lambda>(name, head, body->beta_reduce());
    }
    Expression_ptr bind(Variable_ptr e1, Expression_ptr e2) override {
        if(e1->get_id() == head->get_id()) return body->bind(e1, e2);
        return std::make_shared<Lambda>(name, head, body->bind(e1, e2));
    }
    Expression_ptr alpha_convert(const std::string& old_name, const std::string& new_name) override {
        return std::make_shared<Lambda>(name,
                std::static_pointer_cast<Variable>(head->alpha_convert(old_name, new_name)),
                body->alpha_convert(old_name, new_name));
    }
    Variable_ptr get_head() override {
        return head;
    }
    Expression_ptr get_body() {
        return body;
    }
  private:
    Variable_ptr head;
    Expression_ptr body;
};

typedef std::shared_ptr<Lambda> Lambda_ptr;

