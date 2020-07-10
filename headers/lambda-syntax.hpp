#include <vector>
#include <cassert>
#include <algorithm>

typedef unsigned long ID;

constexpr char LAMBDA = '\\';
constexpr char BODY_START = '.';
constexpr char SEP = ';';
constexpr char BRCK_OPN = '(';
constexpr char BRCK_CLS =')';
constexpr unsigned int MAX_VARS = 264;
const std::runtime_error ER_REDECL = std::runtime_error("Error: variable re-declared!");
const std::runtime_error ER_SYNTAX = std::runtime_error("Error: unexpected syntax!");
const std::runtime_error ER_EMPTY = std::runtime_error("Error: empty tail is not allowed!");
const std::runtime_error ER_START = std::runtime_error("Error: illegal start of expression!");
const std::runtime_error ER_END = std::runtime_error("Error: unexpected end of expression!");

static ID _next_id = 0;

// enum to indicate type of Application, so we do not have to check-cast things using dynamic_cast
enum TYPE {LBD, VAR, APP};



static inline char index(char c) {
    /**
     * returns the index of the char c in the alphabet, only lower case allowed
     */
    assert(islower(c));
    return c - 'a';
}

static std::string _trim_string(const std::string &str) {
    /**
     * trims str of leading and ending whitespaces
     * WARNING: this removes newline, too.
     * (IDEA: ignore newlines entirely, terminate expressions with ';')
     */
    size_t begin = 0;
    size_t end = str.size();
    for(char c = str[0]; isspace(c); c = str[++begin]);
    for(char c = str[end - 1]; isspace(c); c = str[--end - 1]);
    return str.substr(begin, end - begin);
}

class Expression {
    /**
     * abstract base class for all valid lambda expressions
     */
  public:
    Expression() = delete;
    Expression(std::string name, TYPE t) : name(name), t(t) {}
    virtual std::string to_string() const = 0;
    TYPE get_type() const {
        return t;
    }
    std::string get_name() const {
        return name;
    }
    friend void swap(Expression& e1, Expression& e2) {
        using std::swap;
        swap(e1.name, e2.name);
        swap(e1.t, e2.t);
    }
  protected:
    std::string name;
    TYPE t;
};

class Application : public Expression {
    /**
     * multiple Expressions after each other
     */
  public:
    Application(std::string name) : Expression(name, APP), parts() {}
    Application(std::string name, TYPE t): Expression(name, t), parts() {}
    // this constructor needs information about the classes Lambda and Variable and will
    // therefore be declared further down
    Application(const Application& other);
    Application(Application&& other) : Application() {
        swap(*this, other);
    }
    ~Application() {
        for (auto x: parts) delete x;
    }
    Application& operator=(Application other) {
        swap(*this, other);
        return *this;
    }
    std::string to_string() const {
        std::stringstream ss;
        for(Expression* e: parts) {
            if(e->get_type() == VAR) ss << e->get_name() << ' ';
            else ss << '(' << e->to_string() << ") ";
        }
        return ss.str();
    }
    void append(Expression* e) {
        parts.push_back(e);
    }
    unsigned long n() const {
        return parts.size();
    }
    friend void swap(Application& a1, Application& a2) {
        a1.parts.swap(a2.parts);
        swap(static_cast<Expression&>(a1), static_cast<Expression&>(a2));
    }

  private:
    Application() : Expression("invalid", APP), parts() {}
    std::vector<Expression*> parts;
};

class Variable : public Expression {
    /**
     * Variables are named as a single lower case char and can be bound or not
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
    std::string to_string() const {
        return name;
    }
  private:
    bool bound;
    ID id;
};

class Lambda : public Expression {
    /**
     * lambda expressions have a head and a tail, e.g.
     * \ x y . y x
     * \ head . tail
     */
  public:
    Lambda() : head(), tail(), Expression("anonymous", LBD) {}
    Lambda(std::string name) : head(), tail(), Expression(name, LBD) {}
    Lambda(const Lambda& other) : head(std::vector<Variable*>(other.head.size())),
        tail(std::vector<Expression*>(other.tail.size())),
        Expression(other.name, LBD) {
        for(auto v: other.head) {
            head.push_back(new Variable(*v));
        }
        for(auto e: other.tail) {
            switch(e->get_type()) {
                case VAR: tail.push_back(new Variable(*static_cast<Variable*>(e))); break;
                case LBD: tail.push_back(new Lambda(*static_cast<Lambda*>(e))); break;
                case APP: tail.push_back(new Application(*static_cast<Application*>(e))); break;
            }
        }
    }
    Lambda(Lambda&& other) : Lambda() {
        swap(*this, other);
    }
    ~Lambda() {
        for(auto x: head) delete x;
        for(auto x: tail) delete x;
    }
    Lambda& operator=(Lambda other) {
        swap(*this, other);
        return *this;
    }
    void append_head(Variable* x) {
        /**
         * appends a Variable (pointer) to the head of the lambda
         */
        assert(x->is_bound());
        head.push_back(x);
    }
    void append_tail(Expression* x) {
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
    Variable* get_head(unsigned long index) {
        /**
         * returns pointer to index-th variable in the lambda-head
         */
        return head[index];
    }
    Expression* get_tail(unsigned long index) {
        /**
         * returns pointer to index-th expression in the lambda-tail
         */
        return tail[index];
    }
    std::string to_string() const {
        std::stringstream ss;
        if(head.size() > 0) ss << "\\ ";
        for(Variable* v: head) ss << v->get_name() << ' ';
        ss << ". ";
        for(Expression* e: tail) {
            if(e->get_type() != VAR) ss << '(';
            ss << e->to_string() << " ";
            if(e->get_type() != VAR) ss << ") ";
        }
        std::string res = ss.str();
        return res;
    }
    const std::vector<Variable*> &get_head_all() const {
        return head;
    }
    friend void swap(Lambda& l1, Lambda& l2) {
        l1.head.swap(l2.head);
        l1.tail.swap(l2.tail);
        swap(static_cast<Expression&>(l1), static_cast<Expression&>(l2));
    }
  private:
    std::vector<Variable*> head;
    std::vector<Expression*> tail;
};

// late definition of the copy constructor, because we need information about the other classes for it
Application::Application(const Application& other) : parts(other.parts.size()), Expression(other.name, APP) {
    for(auto e: other.parts) {
        switch(e->get_type()) {
            case VAR: parts.push_back(new Variable(*static_cast<Variable*>(e))); break;
            case LBD: parts.push_back(new Lambda(*static_cast<Lambda*>(e))); break;
            case APP: parts.push_back(new Application(*static_cast<Application*>(e))); break;
        }
    }
}

static inline Variable* has_been_bound(char name, std::vector<Variable*> bound) {
    // make sure the last occurance is found, as the last occurance corresponds to the innermost binding
    auto res = std::find_if(bound.rbegin(), bound.rend(), [name](Variable* t) {return t->get_name()[0] == name;});
    if(res == bound.rend()) return nullptr;
    else return *res;
}

static Expression* from_string_rec(std::string str, std::vector<Variable*> &bound) {
    /**
     * builds a single expression from a string, so the input should previously be split on SEP
     * THIS MAY NOT INCLUDE BINDINGS, THESE HAVE TO BE SPLITTED FROM THE 'X =' part beforehand!
     */
    str = _trim_string(str);
    if(str.size() == 1) {
        // single variable detected
        if(Variable* x = has_been_bound(str[0], bound)) return x;
        else return new Variable(str[0], false);
    }
    else if(str[0] == LAMBDA) {
        // lambda expression detected
        Lambda* res = new Lambda();
        // index for O(1) checks if vars are bound
        long var_index[26];
        std::fill_n(var_index, 26, -1);
        // iterate head
        int i = 1;
        for(char c = str[i]; c != BODY_START; c = str[++i]) {
            if(isspace(c)) continue;
            else if(!islower(c)) throw ER_SYNTAX;
            else if(var_index[index(c)] > -1) throw ER_REDECL;
            else {
                res->append_head(new Variable(c, true));
                var_index[index(c)] = res->n_head() - 1;
            }
        }
        size_t old_n_bound = bound.size();
        bound.insert(bound.end(), res->get_head_all().begin(), res->get_head_all().end());
        //iterate tail and build lambda recursively
        for(char c = str[++i]; i < str.size(); c = str[++i]) {
            if(isspace(c)) continue;
            else if(islower(c)) {
                // if there is an entry != -1 for c in var_index, c is a variable thats been declared in teh head -> bound
                if(var_index[index(c)] > -1)
                    res->append_tail(res->get_head(static_cast<unsigned long>(var_index[index(c)])));
                else if(Variable* x = has_been_bound(c, bound)) res->append_tail(x);
                else res->append_tail(new Variable(c, false));
            }
            else if(c == BRCK_OPN) {
                // brackets in lambda expression must contain a valid expression themselfs
                unsigned int begin = static_cast<unsigned int>(++i);
                for(c = str[i]; c != BRCK_CLS && i < str.size(); c = str[++i]);
                if(i == str.size() - 1 && str[i] != BRCK_CLS) throw ER_END;
                // for the expression in bracket we can just use a recursive call
                // WARNING: this leads to bound not being correct, e.g. \x y . z (x y)
                res->append_tail(from_string_rec(str.substr(begin, i - begin), bound));
                ++i;
            }
            else throw ER_SYNTAX;
        }
        // take away the bound variables that were added for this branch
        bound.resize(old_n_bound);
        if(res->n_tail() == 0) throw ER_EMPTY;
        return res;
    }
    else if(isalpha(str[0])) {
        // expression of multiple variables, no lambda
        Application* res = new Application("anonymous");
        for(int i = 0; i < str.size(); ++i) {
            char c = str[i];
            if(isspace(c)) continue;
            if(islower(c)) res->append(new Variable(c, false));
            else if(c == LAMBDA) {
                res->append(from_string_rec(str.substr(static_cast<unsigned long>(i)), bound));
                break;
            }
            else if(c == BRCK_OPN) {
                unsigned int begin = static_cast<unsigned int>(++i);
                for(c = str[i]; c != BRCK_CLS; c = str[++i])
                    if(i == str.size()) throw ER_END;
                res->append(from_string_rec(str.substr(begin, i - begin), bound));
            }
            else {
                throw ER_SYNTAX;
            }
        }
        return res;
    }
    else {
        // ERROR
        throw ER_START;
    }
}

inline Expression* from_string(std::string str) {
    std::vector<Variable*> bound;
    return from_string_rec(str, bound);
}