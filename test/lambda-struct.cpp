#include "gtest/gtest.h"
#include "../headers/lambda-struct.hpp"

using namespace std;

inline vector<Variable_ptr> make_vars(vector<string> names, bool bound) {
    vector<Variable_ptr> res(names.size());
    for(int i = 0; i < names.size(); ++i) {
        res[i] = make_shared<Variable>(names[i], bound);
    }
    return res;
}

TEST(BETA, simple_1) {
    // tests reduction of ( \x . x) hallo
    Variable_ptr v1 = make_shared<Variable>("hallo", false);
    Variable_ptr v2 = make_shared<Variable>("x", true);
    Lambda_ptr l = make_shared<Lambda>("my lambda", v2, v2);
    Application_ptr a = make_shared<Application>("outer scope", l, v1);
    auto res = dynamic_pointer_cast<Variable>(a->beta_reduce());
    ASSERT_EQ(res->to_string(), "hallo");
}
TEST(BETA, simple_2) {
    // tests reduction of (\ x . (\ y . y a) b) c, which requires 2 steps
    auto bounds = make_vars({"x", "y"}, true);
    auto unbounds = make_vars({"a", "b", "c"}, false);
    Application_ptr a1 = make_shared<Application>("y a", bounds[1], unbounds[0]);
    Lambda_ptr l1 = make_shared<Lambda>("\\y.ya", bounds[1], a1);
    Application_ptr a2 = make_shared<Application>("(\\y.ya)b", l1, unbounds[1]);
    Lambda_ptr l2 = make_shared<Lambda>("\\x.(\\y.ya)b)", bounds[0], a2);
    Application_ptr out = make_shared<Application>("outer", l2, unbounds[2]);
    // first reduction
    auto res1 = out->beta_reduce();
    ASSERT_EQ(res1->to_string(), "( (\\ y . ( y a) ) b) ");
    // second reduction
    auto res2 = res1->beta_reduce();
    ASSERT_EQ(res2->to_string(), "( b a) ");
}