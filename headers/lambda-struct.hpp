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
    virtual bool beta_reduce() = 0;
    virtual bool alpha_convert(const std::string& old_name, const std::string& new_name) = 0;

  protected:
    std::string name;
};

typedef std::shared_ptr<Expression> Expression_ptr;

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
    bool beta_reduce() override {
        // TODO
    }
    bool alpha_convert(const std::string& old_name, const std::string& new_name) override {
        return fst->alpha_convert(old_name, new_name) || snd->alpha_convert(old_name, new_name);
    }
    std::string to_string() const override {
        std::stringstream ss;
        ss << "( " << fst->to_string() << " " << snd->to_string() << " ) ";
        return ss.str();
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
    bool beta_reduce() override {
        return false;
    }
    bool alpha_convert(const std::string& old_name, const std::string& new_name) override {
        if(bound && this->name.compare(old_name) == 0) {
            this->name = new_name;
            return true;
        }
        return false;
    }
  private:
    bool bound;
    ID id;
};

typedef std::shared_ptr<Variable> Variable_ptr;

class Lambda : public Expression {
    /**
     * lambda expressions have a head and a body, e.g.
     * \ x y . y x
     * \ head . body
     */
  public:
    Lambda() = delete;
    Lambda(std::string name, Variable_ptr head, Expression_ptr body) : head(head), body(body), Expression(name) {}
    std::string to_string() const override {
        std::stringstream ss;
        ss << "( \\ " << head->to_string() << " . " << body->to_string() << " )";
        return ss.str();
    }
    bool beta_reduce() override {
        // TODO
    }
    bool alpha_convert(const std::string& old_name, const std::string& new_name) override {
        bool res;
        if(head->get_name().compare(old_name) == 0) {
            head->alpha_convert(old_name, new_name);
            res = true;
        }
        else res = false;
        return res || body->alpha_convert(old_name, new_name);

    }
    Variable_ptr get_head() {
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

