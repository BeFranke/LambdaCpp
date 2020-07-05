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
    FROM_TO_STRING("x (\\x y . x y z ) ")
}