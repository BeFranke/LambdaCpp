#include <iostream>
#include "gtest/gtest.h"
#include "../src/headers/tokenizer.hpp"

using namespace std;

void std_test(std::string input, TOKEN_TYPE expected[]) {
    stringstream ss;
    ss << input;
    //Tokenizer tz(ss);
    auto tz = Tokenizer{ss};
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expected[i]);
    }
    ASSERT_GE(i, 1);
}

TEST(TOKENIZER, T1) {
    TOKEN_TYPE expected[] = {LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER};
    std_test("\\ x. x", expected);
}
TEST(TOKENIZER, T2) {
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER};
    std_test("(\\ x. x) bt", expcted);
}
TEST(TOKENIZER, T3) {
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER};
    std_test("(\\x.x)bt", expcted);
}
TEST(TOKENIZER, T4) {
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            CONV_END};
    std_test("(\\x.x)bt>", expcted);
}
TEST(TOKENIZER, T5) {
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            IDENTIFIER, CONV_END, IDENTIFIER};
    std_test("(\\x.x)bt bt>z", expcted);
}
TEST(TOKENIZER, T6) {
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            LITERAL, CONV_END};
    std_test("(\\x.x)bt 77>", expcted);
}
TEST(TOKENIZER, T7) {
    TOKEN_TYPE expcted[] = {NAME_DEFINE, NAME, NAME_DEFINE, ASSIGNMENT, LAMBDA,
                            IDENTIFIER, BODY_START, IDENTIFIER, SEPARATOR,
                            BRACKET_OPEN, NAME, BRACKET_CLOSE, IDENTIFIER,
                            SEPARATOR};
    std_test("# this is a comment\n 'A' = \\ x . x; (A) y;", expcted);
}
TEST(TOKENIZER, T8) {
    TOKEN_TYPE expcted[] = {NAME_DEFINE, NAME, NAME_DEFINE, ASSIGNMENT, LAMBDA,
                            IDENTIFIER, BODY_START, IDENTIFIER, SEPARATOR};
    std_test("'ID' = \\ x. x;", expcted);
}
TEST(TOKENIZER, T9) {
    stringstream ss;
    ss << "\\ ? . ?";
    Tokenizer tz(ss);
    tz.get();
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T10) {
    stringstream ss;
    ss << "/ x . x y";
    Tokenizer tz(ss);
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T11) {
    stringstream ss;
    ss << "xy?";
    Tokenizer tz(ss);
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T12) {
    stringstream ss;
    ss << "?xyz";
    Tokenizer tz(ss, {"?"});
    ASSERT_THROW(tz.get(), ReservedSymbol);
}
TEST(TOKENIZER, T13) {
    stringstream ss;
    ss << "exit;";
    Tokenizer tz(ss, {"exit"});
    ASSERT_THROW(tz.get(), ReservedSymbol);
}

TEST(TOKENIZER, T14) {
    TOKEN_TYPE expcted[] = {LITERAL, LITERAL};
    std_test("true false", expcted);
}
