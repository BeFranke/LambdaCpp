#include "gtest/gtest.h"
#include "../src/headers/lambda-struct.hpp"

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
    Lambda_ptr l = make_shared<Lambda>(v2, v2);
    Application_ptr a = make_shared<Application>(l, v1);
    auto res = a->beta_reduce();
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "hallo");
}

TEST(BETA, simple_2) {
    // tests reduction of (\ x . (\ y . y a) b) c, which requires 2 steps
    auto bounds = make_vars({"x", "y"}, true);
    auto unbounds = make_vars({"a", "b", "c"}, false);
    Application_ptr a1 = make_shared<Application>(bounds[1], unbounds[0]);
    Lambda_ptr l1 = make_shared<Lambda>(bounds[1], a1);
    Application_ptr a2 = make_shared<Application>(l1, unbounds[1]);
    Lambda_ptr l2 = make_shared<Lambda>(bounds[0], a2);
    Application_ptr out = make_shared<Application>(l2, unbounds[2]);
    // first reduction
    auto res1 = out->beta_reduce();
    stringstream ss;
    ss << *res1;
    ASSERT_EQ(ss.str(), "(\\y . (y) a) b");
    ss.str("");
    // second reduction
    auto res2 = res1->beta_reduce();
    ss << *res2;
    ASSERT_EQ(ss.str(), "(b) a");
}

TEST(ALPHA, simple_1) {
    // lets rename the outer x in \ x . (\ x . x) x
    auto bounds = make_vars({"x", "x"}, true);
    auto l1 = make_shared<Lambda>(bounds[0], bounds[0]);
    auto a1 = make_shared<Application>(l1, bounds[1]);
    auto l2 = make_shared<Lambda>(bounds[1], a1);
    auto res = l2->alpha_convert("x", "y");
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "\\y . (\\x . x) y");
}

TEST(BETA, conflicting_names) {
    // (\ x . (\ x . \ y . s u x x y ) c e x x ) s
    auto bound = make_vars({"x", "x", "y"}, true);
    auto unbound = make_vars({"s", "u", "c", "e", "s"}, false);
    auto a1 = make_shared<Application>(unbound[0], unbound[1]);
    auto a2 = make_shared<Application>(a1, bound[1]);
    auto a3 = make_shared<Application>(a2, bound[1]);
    auto a4 = make_shared<Application>(a3, bound[2]);
    auto l1 = make_shared<Lambda>(bound[2], a4);
    auto l2 =  make_shared<Lambda>(bound[1], l1);
    auto a5 = make_shared<Application>(l2, unbound[2]);
    auto a6 = make_shared<Application>(a5, unbound[3]);
    auto a7 = make_shared<Application>(a6, bound[0]);
    auto a8 = make_shared<Application>(a7, bound[0]);
    auto l3 = make_shared<Lambda>(bound[0], a8);
    auto out = make_shared<Application>(l3, unbound[4]);


    auto res1 = out->beta_reduce();
    auto res2 = res1->beta_reduce();
    auto res3 = res2->beta_reduce();
    stringstream ss;
    ss << *res3;
    ASSERT_EQ(ss.str(), "((((((s) u) c) c) e) s) s");
}
TEST(ALPHA, conflicting_names) {
    // (\ x . (\ x . \ y . s u x x y ) c e x x ) s
    // this time with alpha
    auto bound = make_vars({"x", "x", "y"}, true);
    auto unbound = make_vars({"s", "u", "c", "e", "s"}, false);
    auto a1 = make_shared<Application>(unbound[0], unbound[1]);
    auto a2 = make_shared<Application>(a1, bound[1]);
    auto a3 = make_shared<Application>(a2, bound[1]);
    auto a4 = make_shared<Application>(a3, bound[2]);
    auto l1 = make_shared<Lambda>(bound[2], a4);
    auto l2 =  make_shared<Lambda>(bound[1], l1);
    auto a5 = make_shared<Application>(l2, unbound[2]);
    auto a6 = make_shared<Application>(a5, unbound[3]);
    auto a7 = make_shared<Application>(a6, bound[0]);
    auto a8 = make_shared<Application>(a7, bound[0]);
    auto l3 = make_shared<Lambda>(bound[0], a8);
    auto out = make_shared<Application>(l3, unbound[4]);

    auto res = out->alpha_convert(bound[1]->get_name(), "y")->alpha_convert(bound[2]->get_name(), "z");
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "(\\x . ((((\\y . \\z . ((((s) u) y) y) z) c) e) x) x) s");
}
TEST(BETA, first_part_no_reduction) {
    // (g) ((\ x . (x) x) o) d
    auto bound = make_vars({"x"}, true);
    auto unbound = make_vars({"g", "o", "d"}, false);
    auto a1 = make_shared<Application>(bound[0], bound[0]);
    auto l1 = make_shared<Lambda>(bound[0], a1);
    auto a2 = make_shared<Application>(l1, unbound[1]);
    auto a3 = make_shared<Application>(a2, unbound[2]);
    auto a4 = make_shared<Application>(unbound[0], a3);

    auto res = a4->beta_reduce();
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "(g) ((o) o) d");
}
TEST(BETA, NORMAL_ORDER) {
    // from http://www.mathematik.uni-ulm.de/numerik/cpp/ss18/cpp-2018-06-19.pdf
    // (\x . a) (\x . (x) x) \y.(y) y
    auto unbound = make_vars({"a"}, false);
    auto bound = make_vars({"x", "x", "y"}, true);
    auto a1 = make_shared<Application>(bound[2], bound[2]);
    auto l1 = make_shared<Lambda>(bound[2], a1);
    auto a2 = make_shared<Application>(bound[1], bound[1]);
    auto l2 = make_shared<Lambda>(bound[1], a2);
    auto a3 = make_shared<Application>(l2, l1);
    auto l3 = make_shared<Lambda>(bound[0], unbound[0]);
    auto a4 = make_shared<Application>(l3, a3);

    auto res = a4->beta_reduce();
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "a");
}
TEST(STRING_REPR, test1) {
    // (\x . a) (\x . (x) x) \y.(y) y
    auto unbound = make_vars({"a"}, false);
    auto bound = make_vars({"x", "x", "y"}, true);
    auto a1 = make_shared<Application>(bound[2], bound[2]);
    auto l1 = make_shared<Lambda>(bound[2], a1);
    auto a2 = make_shared<Application>(bound[1], bound[1]);
    auto l2 = make_shared<Lambda>(bound[1], a2);
    auto a3 = make_shared<Application>(l2, l1);
    auto l3 = make_shared<Lambda>(bound[0], unbound[0]);
    auto a4 = make_shared<Application>(l3, a3);

    stringstream ss;
    ss << *a4;
    ASSERT_EQ(ss.str(), "(\\x . a) (\\x . (x) x) \\y . (y) y");
}