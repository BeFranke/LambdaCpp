#include <vector>
#include <cassert>

/*
 * TODO: should we move non-lambda expressions like "x y z" into the base class, with a private vector of expressions?
 * TODO: (That therefore does not get inherited)
 */

typedef unsigned long ID;

constexpr char LAMBDA = '\\';
constexpr char BODY_START = '.';
constexpr char SEP = ';';
constexpr char BRCK_OPN[2] = {'(', '['};
constexpr char BRCK_CLS[2] = {')', ']'};
const std::runtime_error ER_REDECL = std::runtime_error("Error: variable re-declared!");
const std::runtime_error ER_SYNTAX = std::runtime_error("Error: unexpected syntax!");
const std::runtime_error ER_EMPTY = std::runtime_error("Error: empty tail is not allowed!");

static ID _next_id = 0;

template<typename T>
static inline int index(T e, const T set[], size_t n) {
    for(int i = 0; i < n; ++i) if(set[i] == e) return i;
    return -1;
}

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
     * base class for Variable and Lambda, and not much more
     */
  public:
    Expression(std::string name) : name(name) {}

  protected:
    std::string name;
  private:
    virtual std::string to_string() = 0;
};

class Variable : public Expression {
    /**
     * Variables are named as a single lower case char and can be bound or not
     */
  public:
    Variable() = delete;
    Variable(std::string name, bool bound) : Expression(name), bound(bound), id(_next_id++) {
        assert(name.size() == 1);
        assert(islower(name[0]));
    }
    Variable(char cname, bool bound) : Expression(std::string(1, cname)), bound(bound), id(_next_id++) {
        assert(name.size() == 1);
    }
    bool is_bound() const {
        return bound;
    }
    std::string get_name() const {
        return name;
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
    Lambda() : head(), tail(), Expression("anonymous") {}
    Lambda(std::string name) : head(), tail(), Expression(name) {}
    // TODO: copy and move constructor, or we will have memory problems!
    ~Lambda() {
        for(auto x: head) delete x;
        for(auto x: tail) delete x;
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
         * appends a Expression (pointer) to the tail of the lambda
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
        for(Variable* v: head) ss << v->get_name() << " ";
        // TODO, after considering moving non-lambda expressions into base class
        //if(head.size() > 0) ss << "\\ ";
    }
  private:
    std::vector<Variable*> head;
    std::vector<Expression*> tail;
};

Expression* from_string(std::string str) {
    /**
     * builds a single expression from a string, so the input should previously be split on SEP
     */
    str = _trim_string(str);
    if(str.size() == 1) {
        // single variable detected
        return new Variable(str[0], false);
    }
    else if(str[0] == LAMBDA) {
        // lambda expression detected
        Lambda* res = new Lambda();
        // index for O(1) checks if vars are bound
        long var_index[26] = {-1};
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

        //iterate tail and build lambda recursively
        for(char c = str[++i]; i < str.size(); c = str[++i]) {
            if(isspace(c)) continue;
            else if(islower(c)) {
                // if there is an entry != -1 for c in var_index, c is a variable thats been declared in teh head -> bound
                if(var_index[index(c)] > -1)
                    res->append_tail(res->get_head(static_cast<unsigned long>(var_index[index(c)])));
                else res->append_tail(new Variable(c, false));
            }
            else if(int j = index(c, BRCK_OPN, 2); j != -1) {
                // brackets in lambda expression must contain a valid expression themselfs
                unsigned int begin = static_cast<unsigned int>(i);
                for(; index(c, BRCK_CLS, 2) == -1; ++i);
                // for the expression in bracket we can just use a recursive call
                // WARNING: this leads to bound not being correct, e.g. \x y . z (x y)
                res->append_tail(from_string(str.substr(begin, i - begin)));
            }
        }
        if(res->n_tail() == 0) throw ER_EMPTY;
        return res;
    }
    else if(isalpha(str[0])) {
        // binding, check for existence of = sign
        // TODO
    }
    else {
        // ERROR
        // TODO
    }
    // last step: top-down iteration to update bound?
    // Problem: name clashes
    // solution?: give every variable an id, but keep original name
}