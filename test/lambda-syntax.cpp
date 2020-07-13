#include "gtest/gtest.h"
#include "../headers/lambda-syntax.hpp"

#define FROM_TO_STRING(s) ASSERT_EQ(s, from_string(s)->to_string());

TEST(TRIM_TEST, Main) {
    ASSERT_EQ("test", _trim_string("     test"));
    ASSERT_EQ("te  st", _trim_string("   te  st"));
    ASSERT_EQ("tes  t", _trim_string("   tes  t  "));
}

TEST(FROM_TO_STRING_TEST, LAMBDAS) {
    FROM_TO_STRING("\\ x . x ");
    FROM_TO_STRING("\\ x y . x ");
    FROM_TO_STRING("\\ x y . y z x ");
}

TEST(FROM_TO_STRING_TEST, EXPRESSIONS) {
    FROM_TO_STRING("x z ");
    FROM_TO_STRING("x (z r t ) ");
    FROM_TO_STRING("x (\\ x y . x y z ) ")
    FROM_TO_STRING("\\ x . (\\ x . x  ) x ")
}

TEST(BOUND_TEST, t1) {
    std::string tst = "\\ x . (\\ x . x) x";
    Lambda* lb = static_cast<Lambda*>(from_string(tst));
    // test that the last x is bound to the lambda in the head (test by checking if the pointers are equal)
    ASSERT_EQ(lb->get_head(0), lb->get_body(1));
    ASSERT_TRUE(static_cast<Variable*>(lb->get_body(1))->is_bound());
    // test that the inner x is not the same as the outer x
    ASSERT_NE(lb->get_head(0), static_cast<Lambda*>(lb->get_body(0))->get_head(0));
}

TEST(BOUND_TEST, t2) {
    std::string tst = "\\ x . (\\ y . x) x";
    Lambda* lb = static_cast<Lambda*>(from_string(tst));
    // test that the inner x is the same as the outer x
    ASSERT_EQ(lb->get_head(0), static_cast<Lambda *>(lb->get_body(0))->get_body(0));
}

TEST(MEM_LEAK_TEST, p1) {
    for(int i = 0; i < 100; ++i) {
        try {
            Expression* l = from_string("\\ x . x (y u (z )");
        } catch(std::runtime_error) {

        }
    }
}