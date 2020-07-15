#pragma once
#include <memory>

// TODO: methods for literal equivalence and alpha equivalence

typedef unsigned long ID;

// counter for unique ids for variables
static ID _next_id = 0;

// forward declarations
class Expression;
class Variable;
class Application;
class Lambda;
typedef std::shared_ptr<Expression> Expression_ptr;
typedef std::shared_ptr<Application> Application_ptr;
typedef std::shared_ptr<Variable> Variable_ptr;
typedef std::shared_ptr<Lambda> Lambda_ptr;

class Expression {
    /**
     * abstract base class for all valid expressions
     */
  public:
    Expression() = delete;
    virtual std::string to_string() const noexcept = 0;
    std::string get_name() const {
        return name;
    }
    virtual Expression_ptr beta_reduce() = 0;
    virtual Expression_ptr bind(Variable_ptr, Expression_ptr) = 0;
    virtual Expression_ptr alpha_convert(const Variable&, const std::string&) = 0;
    virtual Variable_ptr get_head() = 0;

  protected:
    Expression(std::string name) : name(name) {}
    std::string name;
};

class Application final : public Expression {
    /**
     * two Expressions after each other
     * e.g. x y or (\ x . x) y
     */
  public:
    Application(std::string name) : Expression(name), fst(), snd() {}
    Application(std::string name, Expression_ptr fst, Expression_ptr snd) : Expression(name), fst(fst), snd(snd) {}

    Expression_ptr bind(Variable_ptr e_old, Expression_ptr e_new) override {
        /**
         * binds e_old to e_new in both expressions of this application
         * returns new Expression
         */
        return std::make_shared<Application>(name, fst->bind(e_old, e_new), snd->bind(e_old, e_new));
    }
    Expression_ptr beta_reduce() override {
        /**
         * invokes a beta reduction:
         * if fst is a lambda, binds second to the bound variable in fst
         * otherwise, passes beta-reduction on to fst and snd
         * TODO: is invoking fst AND snd correct? Does this still keep normal order?
         */
        // if it has a head, it is a lambda
        if(auto h = fst->get_head(); h != nullptr) return fst->bind(h, snd);
        else return std::make_shared<Application>(name, fst->beta_reduce(), snd->beta_reduce());
    }
    Expression_ptr alpha_convert(const Variable& old, const std::string& new_name) override {
        /**
         * renames bound variable old_name to new_name
         */
        Expression_ptr res = std::make_shared<Application>(name, fst->alpha_convert(old, new_name),
                snd->alpha_convert(old, new_name));
        return res;
    }
    std::string to_string() const noexcept override {
        /**
         * string representation
         */
        std::stringstream ss;
        //ss << "( " << fst->to_string() << " " << snd->to_string() << ") ";
        if(fst->get_head() != nullptr) ss << "(" << fst->to_string() << ")" << " ";
        else ss << fst->to_string();
        if(snd->get_head() != nullptr) ss << "(" << snd->to_string() << ")" << " ";
        else ss << snd->to_string();
        return ss.str();
    }
    Variable_ptr get_head() override {
        /**
         * returns nullptr, as only lambdas have a head
         */
        return std::shared_ptr<Variable>(nullptr);
    }
  private:
    Expression_ptr fst;
    Expression_ptr snd;

};

class Variable final : public Expression {
    /**
     * Variables have a name and can be bound or not
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
        /**
         * id uniquely identifies a variable
         */
        return id;
    }
    std::string to_string() const noexcept override {
        /**
         * string representation of variable is its name
         */
        return name + " ";
    }
    Expression_ptr beta_reduce() override {
        /**
         * beta reduction of variable is identity
         */
        return std::make_shared<Variable>(name, bound);
    }
    Expression_ptr alpha_convert(const Variable& old, const std::string& new_name) override {
        /**
         * renames if name matches and variable is bound
         */
        std::string name;
        if(bound && id == old.id) name = new_name;
        else name = this->name;
        return std::make_shared<Variable>(name, bound, id);

    }
    Expression_ptr bind(Variable_ptr e1, Expression_ptr e2) override {
        /**
         * returns e2 if e1 matches itself, else returns copy of itself
         */
        if(id == e1->id) return e2;
        else return std::make_shared<Variable>(name, bound, id);
    }
    Variable_ptr get_head() override {
        /**
         * Variable does not have head, returns nullptr
         */
        return std::shared_ptr<Variable>(nullptr);
    }

  private:
    bool bound;
    ID id;
};

class Lambda final : public Expression {
    /**
     * lambda expressions have a head and a body, e.g.
     * \ x . (y x)
     * \ head . body
     */
  public:
    Lambda() = delete;
    Lambda(std::string name, Variable_ptr head, Expression_ptr body) : head(head), body(body), Expression(name) {}
    std::string to_string() const noexcept override {
        /**
         * string representation with brackets and "\\ x ." as lambda abstraction
         */
        std::stringstream ss;
        ss << "\\ " << head->to_string() << ". " << body->to_string();
        return ss.str();
    }
    Expression_ptr beta_reduce() override {
        /**
         * beta reduction needs an application, so it gets passed down to body
         */
        return std::make_shared<Lambda>(name, head, body->beta_reduce());
    }
    Expression_ptr bind(Variable_ptr e1, Expression_ptr e2) override {
        /**
         * if e1 matches head, binds head and returns body
         * else passes binding on to body
         */
        if(e1->get_id() == head->get_id()) return body->bind(e1, e2);
        return std::make_shared<Lambda>(name, head, body->bind(e1, e2));
    }
    Expression_ptr alpha_convert(const Variable& old, const std::string& new_name) override {
        /**
         * passes conversion on to head and body
         */
        return std::make_shared<Lambda>(name,
                std::static_pointer_cast<Variable>(head->alpha_convert(old, new_name)),
                body->alpha_convert(old, new_name));
    }
    Variable_ptr get_head() override {
        /**
         * returns head, the variable in the abstraction
         * e.g. \ x . x y -> x is head
         */
        return head;
    }
    Expression_ptr get_body() {
        /**
         * returns body, the expression after the abstraction
         * e.g. \ x . x y -> x y is body
         */
        return body;
    }
  private:
    Variable_ptr head;
    Expression_ptr body;
};


