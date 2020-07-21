#include <vector>
#include "gtest/gtest.h"
#include "../src/headers/lambda-syntax.hpp"

TEST(BUILD_TREE, test1) {
    // \x . x
    std::vector<token> in = {{OPERATOR, "\\"}, {IDENTIFIER, "x"}, {OPERATOR, "."}, {IDENTIFIER, "x"}};
    auto result = build_syntax_tree(in.begin(), in.end());
    ASSERT_EQ(result->to_string(), "\\x . x ");
}

TEST(BUILD_TREE, test2) {
    // (\x . (x) y) h
    std::vector<token> in = {{SEPARATOR, "("}, {OPERATOR, "\\"}, {IDENTIFIER, "x"}, {OPERATOR, "."},
                             {SEPARATOR, "("}, {IDENTIFIER, "x"}, {SEPARATOR, ")"}, {IDENTIFIER, "y"},
                             {SEPARATOR, ")"}, {IDENTIFIER, "z"}};
    auto result = build_syntax_tree(in.begin(), in.end());
    ASSERT_EQ(result->to_string(), "(\\x . (x) y) z ");
}

TEST(BUILD_TREE, test3) {
    // from http://www.mathematik.uni-ulm.de/numerik/cpp/ss18/cpp-2018-06-19.pdf
    // (\x . a) (\x . (x) x) \y.(y) y
    std::vector<token> in = {{SEPARATOR, "("}, {OPERATOR, "\\"}, {IDENTIFIER, "x"}, {OPERATOR, "."},
                             {IDENTIFIER, "a"}, {SEPARATOR, ")"}, {SEPARATOR, "("}, {OPERATOR, "\\"},
                             {IDENTIFIER, "x"}, {OPERATOR, "."}, {SEPARATOR, "("}, {IDENTIFIER, "x"},
                             {SEPARATOR, ")"}, {IDENTIFIER, "x"}, {SEPARATOR, ")"}, {OPERATOR, "\\"},
                             {IDENTIFIER, "y"}, {OPERATOR, "."}, {SEPARATOR, "("}, {IDENTIFIER, "y"},
                             {SEPARATOR, ")"}, {IDENTIFIER, "y"}};
    auto result = build_syntax_tree(in.begin(), in.end());
    ASSERT_EQ(result->to_string(), "(\\x . a) (\\x . (x) x) \\y . (y) y ");
}