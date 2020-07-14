#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "../headers/tokenizer.hpp"

TEST(TOKENIZER, T1) {
    std::istringstream ss = std::istringstream("\\ x . \\ y . x y z");
    auto result = parse(ss);
    ASSERT_EQ(result->size(), 9);
    TOKEN_TYPE expected[] = {OPERATOR, IDENTIFIER, OPERATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, IDENTIFIER, IDENTIFIER};
    for(char i = 0; i < result->size(); ++i) {
        ASSERT_EQ(std::get<0>((*result)[i]), expected[i]);
    }
}

TEST(TOKENIZER, T2) {
    std::istringstream ss = std::istringstream("\\ x . (\\ y . x y) z");
    auto result = parse(ss);
    ASSERT_EQ(result->size(), 11);
    TOKEN_TYPE expected[] = {OPERATOR, IDENTIFIER, OPERATOR, SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, IDENTIFIER, SEPARATOR, IDENTIFIER};
    for(char i = 0; i < result->size(); ++i) {
        ASSERT_EQ(std::get<0>((*result)[i]), expected[i]);
    }
}

TEST(TOKENIZER, T3) {
    std::istringstream ss = std::istringstream("((  ((x   )) ))");
    auto result = parse(ss);
    ASSERT_EQ(result->size(), 9);
    TOKEN_TYPE expected[] = {SEPARATOR, SEPARATOR, SEPARATOR, SEPARATOR, IDENTIFIER, SEPARATOR, SEPARATOR, SEPARATOR, SEPARATOR};
    for(char i = 0; i < result->size(); ++i) {
        ASSERT_EQ(std::get<0>((*result)[i]), expected[i]);
    }
}

TEST(TOKENIZER, T4) {
    std::istringstream ss = std::istringstream("null = \\ x. n");
    auto result = parse(ss);
    ASSERT_EQ(result->size(), 6);
    TOKEN_TYPE expected[] = {IDENTIFIER, OPERATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER};
    for(char i = 0; i < result->size(); ++i) {
        ASSERT_EQ(std::get<0>((*result)[i]), expected[i]);
    }
}

TEST(TOKENIZER, T5) {
    std::istringstream ss = std::istringstream("null == \\ x. n");
    auto result = parse(ss);
    ASSERT_EQ(result->size(), 7);
    TOKEN_TYPE expected[] = {IDENTIFIER, OPERATOR, OPERATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER};
    for(char i = 0; i < result->size(); ++i) {
        ASSERT_EQ(std::get<0>((*result)[i]), expected[i]);
    }
}

TEST(TOKENIZER, T6) {
    std::istringstream ss = std::istringstream("1fehler");
    ASSERT_THROW(parse(ss), SyntaxException);
}

TEST(TOKENIZER, T7) {
    std::istringstream ss = std::istringstream("feh!ler");
    ASSERT_THROW(parse(ss), SyntaxException);
}
