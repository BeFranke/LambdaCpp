#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include "lambda-exceptions.hpp"
#include "utils.hpp"

typedef unsigned long ID;

// syntactic constants
// using '\' as replacement for "lambda" is stolen from Haskell
constexpr char LAMBDA = '\\';
constexpr char BODY_START = '.';
constexpr char BRCK_OPN = '(';
constexpr char BRCK_CLS =')';

// counter for unique ids for variables (currently IDs are not used)
static ID _next_id = 0;

// enum to indicate type of Expression for save casting
enum TYPE {LBD, VAR, APP};

class Expression {
    /**
     * abstract base class for all valid expressions
     */
  public:
    Expression() = delete;
    Expression(std::string name, TYPE t) : name(name), t(t) {}
    virtual ~Expression() {};
    virtual std::string to_string() const = 0;
    TYPE get_type() const {
        return t;
    }
    std::string get_name() const {
        return name;
    }
    virtual bool reduce_first() = 0;
  protected:
    std::string name;
    TYPE t;
  private:
    friend void swap(Expression& e1, Expression& e2) {
        using std::swap;
        swap(e1.name, e2.name);
        swap(e1.t, e2.t);
    }
};

typedef std::shared_ptr<Expression> Expression_ptr;

class Application : public Expression {
    /**
     * multiple Expressions after each other
     * e.g. x y z
     */
  public:
    Application(std::string name) : Expression(name, APP), parts() {}
    // this constructor needs information about the classes Lambda and Variable and will
    // therefore be declared further down
    Application(const Application& other);
    Application(Application&& other) : Application() {
        swap(*this, other);
    }
    /*~Application() {
        for (auto x: parts) delete x;
    }*/
    Application& operator=(Application other) {
        swap(*this, other);
        return *this;
    }
    std::string to_string() const override {
        std::stringstream ss;
        for(Expression_ptr e: parts) {
            if(e->get_type() == VAR) ss << e->get_name() << ' ';
            else ss << '(' << e->to_string() << ") ";
        }
        return ss.str();
    }
    void append(std::shared_ptr<Expression> e) {
        parts.push_back(e);
    }
    unsigned long n() const {
        return parts.size();
    }
    Expression_ptr get_body(unsigned int i) {
        return parts[i];
    }
    void _set_parts(std::vector<Expression_ptr> parts_new) {
        parts = parts_new;
    }
    bool reduce_first() override;

  private:
    Application() : Expression("invalid", APP), parts() {}
    std::vector<Expression_ptr> parts;
    friend void swap(Application& a1, Application& a2) {
        a1.parts.swap(a2.parts);
        swap(static_cast<Expression&>(a1), static_cast<Expression&>(a2));
    }
};

typedef std::shared_ptr<Application> Application_ptr;

class Variable : public Expression {
    /**
     * Variables are named as a single lower case char and can be bound or not
     * e.g. x
     */
  public:
    Variable() = delete;
    Variable(std::string name, bool bound) : Expression(name, VAR), bound(bound), id(_next_id++) {
        assert(name.size() == 1);
        assert(islower(name[0]));
    }
    Variable(char cname, bool bound) : Expression(std::string(1, cname), VAR), bound(bound), id(_next_id++) {
        assert(name.size() == 1);
    }
    Variable(const Variable &other) : bound(other.bound), id(other.id), Expression(other.get_name(), other.get_type()){}
    bool is_bound() const {
        return bound;
    }
    ID get_id() const {
        return id;
    }
    std::string to_string() const override {
        return name;
    }
    bool reduce_first() override {
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
    Lambda() : head(), tail(), Expression("anonymous", LBD) {}
    Lambda(std::string name) : head(), tail(), Expression(name, LBD) {}
    Lambda(const Lambda& other) : head(std::vector<Variable_ptr>(other.head.size())),
        tail(std::vector<Expression_ptr>(other.tail.size())),
        Expression(other.name, LBD) {
        for(auto v: other.head) {
            head.push_back(std::make_shared<Variable>(*v));
        }
        for(auto e: other.tail) {
            switch(e->get_type()) {
                case VAR: tail.push_back(std::static_pointer_cast<Variable>(e)); break;
                case LBD: tail.push_back(std::make_shared<Lambda>(*std::static_pointer_cast<Lambda>(e))); break;
                case APP: tail.push_back(std::make_shared<Application>(*std::static_pointer_cast<Application>(e))); break;
            }
        }
    }
    Lambda(Lambda&& other) : Lambda() {
        swap(*this, other);
    }
    /*~Lambda() {
        for(auto x: head) delete x;
        for(auto x: tail) delete x;
    }*/
    Lambda& operator=(Lambda other) {
        swap(*this, other);
        return *this;
    }
    void append_head(Variable_ptr x) {
        /**
         * appends a Variable (pointer) to the head of the lambda
         */
        assert(x->is_bound());
        head.push_back(x);
    }
    void append_tail(Expression_ptr x) {
        /**
         * appends a Application (pointer) to the tail of the lambda
         */
        tail.push_back(x);
    }
    unsigned long n_head() const {
        /**
         * gives the size of the head, i.e. how many bound variables are in the top-level(!) of this lambda
         * this does not include bound variables in nested lambdas
         * i.e. \ x y . (\ z . z ) x y -> n_head = 2 ( x and y)
         */
        return head.size();
    }
    unsigned long n_tail() const {
        /**
         * gives the number of terms in the tail of this lambda
         * i.e. \ x y . (\ z . z ) x y -> n_tail = 3
         */
         return tail.size();
    }
    Variable_ptr get_head(unsigned long index) {
        /**
         * returns pointer to index-th variable in the lambda-head
         */
        return head[index];
    }
    Expression_ptr get_body(unsigned long index) {
        /**
         * returns pointer to index-th expression in the lambda-tail
         */
        return tail[index];
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
         Variable_ptr to_bind = head[0];
         // replace in tail
         for(int i = 0; i < n_tail(); ++i) {
             if(tail[i] == to_bind) {
                 switch(tail[i]->get_type()) {
                     case LBD:
                         tail[i] = std::make_shared<Lambda>(*std::static_pointer_cast<Lambda>(to_insert));
                         break;
                     case APP:
                         tail[i] = std::make_shared<Application>(*std::static_pointer_cast<Application>(to_insert));
                         break;
                     case VAR:
                         tail[i] = to_insert;
                         break;
                 }
             }
         }
        head.erase(head.begin());
    }
    bool reduce_first() override;
    Application_ptr empty_head_to_application() {
        assert(n_head() == 0);
        assert(n_tail() > 1);
        Application_ptr res = std::make_shared<Application>(name);
        res->_set_parts(tail);
        return res;
    }
    Variable_ptr singleton_to_variable() {
        assert(n_head() == 0);
        assert(n_tail() == 1);
        assert(tail[0]->get_type() == VAR);
        Variable_ptr res = std::static_pointer_cast<Variable>(tail[0]);
        return res;
    }
  private:
    std::vector<Variable_ptr> head;
    std::vector<Expression_ptr> tail;
    friend void swap(Lambda& l1, Lambda& l2) {
        l1.head.swap(l2.head);
        l1.tail.swap(l2.tail);
        swap(static_cast<Expression&>(l1), static_cast<Expression&>(l2));
    }
};

typedef std::shared_ptr<Lambda> Lambda_ptr;

// late definition of the copy constructor, because we need information about the other classes for it
Application::Application(const Application& other) : parts(other.parts.size()), Expression(other.name, APP) {
    for(auto e: other.parts) {
        switch(e->get_type()) {
            case VAR: parts.push_back(std::static_pointer_cast<Variable>(e)); break;
            case LBD: parts.push_back(std::make_shared<Lambda>(*std::static_pointer_cast<Lambda>(e))); break;
            case APP: parts.push_back(std::make_shared<Application>(*std::static_pointer_cast<Application>(e))); break;
        }
    }
}

bool Application::reduce_first() {
    if(parts.size() <= 1) return false;
    int i;
    for(i = 0; i < parts.size(); ++i) if(parts[i]->get_type() == LBD) break;
    if(parts.size() == i+1) return false;
    Lambda_ptr lbd = std::static_pointer_cast<Lambda>(parts[i]);
    Expression_ptr to_insert = parts[i+1];
    lbd->bind(to_insert);
    parts.erase(parts.begin() + i+1);
    return true;
}

bool Lambda::reduce_first() {
    if(tail.size() <= 1) return false;
    int i;
    for(i = 0; i < tail.size(); ++i) if(tail[i]->get_type() == LBD) break;
    if(tail.size() == i+1) return false;
    Lambda_ptr lbd = std::static_pointer_cast<Lambda>(tail[i]);
    Expression_ptr to_insert = tail[i+1];
    lbd->bind(to_insert);
    tail.erase(tail.begin() + i+1);
    return true;
};

static inline std::shared_ptr<Variable> has_been_bound(char name, std::vector<std::shared_ptr<Variable>> bound) {
    // make sure the last occurance is found, as the last occurance corresponds to the innermost binding
    auto res = std::find_if(bound.rbegin(), bound.rend(), [name](Variable_ptr t) {return t->get_name()[0] == name;});
    if(res == bound.rend()) return nullptr;
    else return *res;
}

static Expression_ptr from_string_rec(std::string str, std::vector<std::shared_ptr<Variable>> &bound) {
    /**
     * builds a single expression from a string, so the input should previously be split on SEP
     * THIS MAY NOT INCLUDE BINDINGS, THESE HAVE TO BE SPLITTED FROM THE 'X =' part beforehand!
     * str: string that defines the lambda expression
     * bound: vector to be filled with the variables that are bound, should be empty on top-level call
     */
    str = _trim_string(str);
    if(str.size() == 1) {
        // single variable detected
        if(auto x = has_been_bound(str[0], bound)) return x;
        else return std::make_shared<Variable>(str[0], false);
    }
    else if(str[0] == LAMBDA) {
        // lambda expression detected
        Lambda_ptr res = std::make_shared<Lambda>();
        // index for O(1) checks if vars are bound
        long var_index[26];
        std::fill_n(var_index, 26, -1);
        // iterate head
        int i = 1;
        for(char c = str[i]; c != BODY_START; c = str[++i]) {
            if(isspace(c)) continue;
            else if(!islower(c)) throw SyntaxException(str);
            else if(var_index[index(c)] > -1) throw ReDeclarationException(str);
            else {
                res->append_head(std::make_shared<Variable>(c, true));
                var_index[index(c)] = res->n_head() - 1;
            }
        }
        if(res->n_head() == 0) throw EmptyException(str);
        size_t old_n_bound = bound.size();
        bound.insert(bound.end(), res->get_head_all().begin(), res->get_head_all().end());
        //iterate tail and build lambda recursively
        for(char c = str[++i]; i < str.size(); c = str[++i]) {
            if(isspace(c)) continue;
            else if(islower(c)) {
                // if there is an entry != -1 for c in var_index, c is a variable thats been declared in teh head -> bound
                if(var_index[index(c)] > -1)
                    res->append_tail(res->get_head(static_cast<unsigned long>(var_index[index(c)])));
                else if(Variable_ptr x = has_been_bound(c, bound)) res->append_tail(x);
                else res->append_tail(std::make_shared<Variable>(c, false));
            }
            else if(c == BRCK_OPN) {
                // brackets in lambda expression must contain a valid expression themselfs
                unsigned int begin = static_cast<unsigned int>(++i);
                c = str[i];
                for(char brckt_debt = 0; (c != BRCK_CLS || brckt_debt > 0); c = str[++i]){
                    if(i == str.size()) throw EndException(str);
                    else if(c == BRCK_OPN) ++brckt_debt;
                    else if(c == BRCK_CLS) --brckt_debt;
                }
                // for the expression in brackets we can just use a recursive call
                res->append_tail(from_string_rec(str.substr(begin, i - begin), bound));
                ++i;
            }
            else throw SyntaxException(str);
        }
        // take away the bound variables that were added for this branch
        bound.resize(old_n_bound);
        if(res->n_tail() == 0) throw EmptyException(str);
        return res;
    }
    else if(isalpha(str[0]) || str[0] == BRCK_OPN) {
        // expression of multiple variables, no lambda
        Application_ptr res = std::make_shared<Application>("anonymous");
        for(int i = 0; i < str.size(); ++i) {
            char c = str[i];
            if(isspace(c)) continue;
            if(islower(c)) res->append(std::make_shared<Variable>(c, false));
            else if(c == LAMBDA) {
                res->append(from_string_rec(str.substr(static_cast<unsigned long>(i)), bound));
                break;
            }
            else if(c == BRCK_OPN) {
                unsigned int begin = static_cast<unsigned int>(++i);
                c = str[i];
                for(char brckt_debt = 0; (c != BRCK_CLS || brckt_debt > 0); c = str[++i]){
                    if(i == str.size()) throw EndException(str);
                    else if(c == BRCK_OPN) ++brckt_debt;
                    else if(c == BRCK_CLS) --brckt_debt;
                }
                res->append(from_string_rec(str.substr(begin, i - begin), bound));
            }
            else {
                throw SyntaxException(str);
            }
        }
        return res;
    }
    else {
        // ERROR
        throw StartException(str);
    }
}

inline Expression_ptr from_string(std::string str) {
    /**
     * initial call for from_string_rec, so not every call needs to init the bound-vector
     */
    std::vector<Variable_ptr> bound;
    return from_string_rec(str, bound);
}