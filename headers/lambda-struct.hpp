#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include "lambda-exceptions.hpp"

typedef unsigned long ID;

// counter for unique ids for variables (currently IDs are not used)
static ID _next_id = 0;

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
    virtual bool beta_reduce_first() = 0;
    virtual bool beta_reduce() = 0;
    virtual bool alpha_convert() = 0;
    virtual std::shared_ptr<Expression>& operator[](std::size_t index) = 0;

  protected:
    std::string name;
  private:
    friend void swap(Expression& e1, Expression& e2) {
        using std::swap;
        swap(e1.name, e2.name);
    }
};

typedef std::shared_ptr<Expression> Expression_ptr;

class Application final : public Expression {
    /**
     * multiple Expressions after each other
     * e.g. x y z
     */
  public:
    Application(std::string name) : Expression(name), fst(), snd() {}
    Application(std::string name, Expression_ptr fst, Expression_ptr snd) : Expression(name), fst(fst), snd(snd) {}

    Expression_ptr& operator[](std::size_t index) override {
        if(index == 0) return fst;
        else if(index == 1) return snd;
        else {
            throw std::runtime_error("Application only has two fields!");
        }
    }
  private:
    Expression_ptr fst;
    Expression_ptr snd;

};

typedef std::shared_ptr<Application> Application_ptr;

class Variable final : public Expression {
    /**
     * Variables are named as a single lower case char and can be bound or not
     * e.g. x
     */
  public:
    Variable() = delete;
    Variable(std::string name, bool bound) : Expression(name), bound(bound), id(_next_id++) {}
    bool is_bound() const noexcept {
        return bound;
    }
    ID get_id() const noexcept {
        return id;
    }
    std::string to_string() const override {
        return name;
    }
    bool beta_reduce_first() override {
        return false;
    }
  private:
    bool bound;
    ID id;
};

typedef std::shared_ptr<Variable> Variable_ptr;

class Lambda : public Expression {
    /**
     * lambda expressions have a head and a tail, e.g.
     * \ x y . y x
     * \ head . tail
     */
  public:
    Lambda() : head(), tail(), Expression("anonymous") {}
    Lambda(std::string name) : head(), tail(), Expression(name) {}
    Lambda& operator=(Lambda other) {
        swap(*this, other);
        return *this;
    }
    std::string to_string() const override {
        std::stringstream ss;
        if(head.size() > 0) ss << "\\ ";
        for(Variable_ptr v: head) ss << v->get_name() << ' ';
        ss << ". ";
        for(Expression_ptr e: tail) {
            if(e->get_type() != VAR) ss << '(';
            ss << e->to_string() << " ";
            if(e->get_type() != VAR) ss << ") ";
        }
        std::string res = ss.str();
        return res;
    }
    const std::vector<Variable_ptr> &get_head_all() const {
        return head;
    }
    void bind(Expression_ptr to_insert) {
        /**
         * mutates the Lambda by replacing every occurence of the first bound variable by to_insert
         */
    }
    bool beta_reduce_first() override;
    Application_ptr empty_head_to_application() {

    }
    Variable_ptr singleton_to_variable() {

    }
  private:
    Variable_ptr head;
    Expression_ptr tail;
};

typedef std::shared_ptr<Lambda> Lambda_ptr;


static inline std::shared_ptr<Variable> has_been_bound(char name, std::vector<std::shared_ptr<Variable>> bound) {
    // make sure the last occurance is found, as the last occurance corresponds to the innermost binding
    auto res = std::find_if(bound.rbegin(), bound.rend(), [name](Variable_ptr t) {return t->get_name()[0] == name;});
    if(res == bound.rend()) return nullptr;
    else return *res;
}
