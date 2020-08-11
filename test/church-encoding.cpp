#include <gtest/gtest.h>
#include "../src/lib/church-encoding.hpp"

TEST(CHURCH, zero) {
    Expression_ptr res = church_encode(0);
    std::stringstream ss;
    ss << *res;
    std::string expected = "\\f . \\x . x";
    ASSERT_EQ(ss.str(), expected);
}

TEST(CHURCH, five) {
    Expression_ptr res = church_encode(5);
    std::stringstream ss;
    ss << *res;
    std::string expected = "\\f . \\x . (f) (f) (f) (f) (f) x";
    ASSERT_EQ(ss.str(), expected);
}