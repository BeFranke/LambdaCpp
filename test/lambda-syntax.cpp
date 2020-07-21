#include <vector>
#include "gtest/gtest.h"
#include "../headers/lambda-syntax.hpp"

TEST(BUILD_TREE, test1) {
    // \x . x
    std::vector<token> in = {{OPERATOR, "\\"}, {IDENTIFIER, "x"}, {OPERATOR, "."}, {IDENTIFIER, "x"}};
    auto result = build_syntax_tree(in.begin(), in.end());
    ASSERT_EQ(result->to_string(), "\\ x . x ");
}