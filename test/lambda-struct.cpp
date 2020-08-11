#include "gtest/gtest.h"
#include "../src/lib/lambda-struct.hpp"

using namespace std;

inline vector<Variable_ptr> make_vars(vector<string> names, bool bound) {
    vector<Variable_ptr> res(names.size());
    for(unsigned int i = 0; i < names.size(); ++i) {
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
    // lets rename the x in \ x . (\ z . z) x
    auto bounds = make_vars({"z", "x"}, true);
    auto l1 = make_shared<Lambda>(bounds[0], bounds[0]);
    auto a1 = make_shared<Application>(l1, bounds[1]);
    auto l2 = make_shared<Lambda>(bounds[1], a1);
    auto res = l2->alpha_convert("x", "y");
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "\\y . (\\z . z) y");
}
TEST(BETA, conflicting_names) {
    auto bound = make_vars({"x", "y", "x", "x"}, true);
    auto unbound = make_vars({"b"}, false);

    auto xy = make_shared<Application>(bound[2], bound[1]);
    auto lx_xy = make_shared<Lambda>(bound[2], xy);
    auto lx_ly = make_shared<Lambda>(bound[1], lx_xy);
    auto a1 = make_shared<Application>(lx_ly, bound[0]);
    auto id = make_shared<Lambda>(bound[3], bound[3]);
    auto a2 = make_shared<Application>(a1, id);
    auto lx_outer = make_shared<Lambda>(bound[0], a2);
    auto outer = make_shared<Application>(lx_outer, unbound[0]);

    cout << *outer << endl;
    auto res1 = outer->beta_reduce();
    cout << *res1 << endl;
    auto res2 = res1->beta_reduce();
    cout << *res2 << endl;
    auto res3 = res2->beta_reduce();
    cout << *res3 << endl;
    auto res4 = res3->beta_reduce();

    stringstream ss;
    ss << *res4;
    ASSERT_EQ(ss.str(), "b");
}

TEST(ALPHA, conflicting_names) {
    auto bound = make_vars({"x", "x", "y"}, true);
    auto xy = make_shared<Application>(bound[2], bound[1]);
    auto ly = make_shared<Lambda>(bound[2], xy);
    auto lx = make_shared<Lambda>(bound[1], ly);
    auto a1 = make_shared<Application>(lx, bound[0]);
    auto a2 = make_shared<Application>(a1, bound[0]);
    auto outer = make_shared<Lambda>(bound[0], a2);
    cout << *outer << endl;
    auto res1 = outer->alpha_convert("x", "u");
    cout << *res1 << endl;
    auto res2 = res1->alpha_convert("x", "v");
    cout << *res2 << endl;
    auto res3 = res2->alpha_convert("y", "w");
    stringstream ss;
    ss << *res3;
    ASSERT_EQ(ss.str(), "\\u . ((\\v . \\w . (w) v) u) u");
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

TEST(BETA, no_normal_form) {
    // (\ x . (x) x) \ y . (y) y
    auto bound = make_vars({"x", "y"}, true);
    auto xx = make_shared<const Application>(bound[0], bound[0]);
    auto yy = make_shared<const Application>(bound[1], bound[1]);
    auto lx = make_shared<const Lambda>(bound[0], xx);
    auto ly = make_shared<const Lambda>(bound[1], yy);
    auto ap = make_shared<const Application>(lx, ly);
    for(char i = 0; i < 99; ++i) {
        auto ap_new = ap->beta_reduce();
        ASSERT_NE(ap_new, ap);
        ap = static_pointer_cast<const Application>(ap_new);
        if(i == 10) cout << *ap << endl;
    }
}

TEST(BETA, lambda) {
    // \ x . (\ y . x) a
    auto bound = make_vars({"x", "y"}, true);
    auto unbound = make_vars({"a"}, false);
    auto inner = make_shared<Lambda>(bound[1], bound[0]);
    auto app = make_shared<Application>(inner, unbound[0]);
    auto outer = make_shared<Lambda>(bound[0], app);
    auto res = outer->beta_reduce();
    stringstream ss;
    ss << *res;
    ASSERT_EQ(ss.str(), "\\x . x");
}

TEST(ALPHA, name_clash) {
    // trying to rename x to y in \ x. (x) y
    auto bound = make_vars({"x"}, true);
    auto unbound = make_vars({"y"}, false);
    auto xy = make_shared<Application>(bound[0], unbound[0]);
    auto lx = make_shared<Lambda>(bound[0], xy);
    ASSERT_THROW(lx->alpha_convert("x", "y"), NameClash);
}