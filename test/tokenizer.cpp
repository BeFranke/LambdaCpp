#include <iostream>
#include "gtest/gtest.h"
#include "../src/headers/tokenizer.hpp"

using namespace std;



void std_test(std::string input, TokenType expected[]) {
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
    TokenType expected[] = {TokenType::lambda,
                            TokenType::identifier,
                            TokenType::body_start,
                            TokenType::identifier};
    std_test("\\ x. x", expected);
}
TEST(TOKENIZER, T2) {
    TokenType expcted[] = {TokenType::bracket_open, TokenType::lambda,
                           TokenType::identifier, TokenType::body_start,
                           TokenType::identifier, TokenType::bracket_close,
                           TokenType::identifier};
    std_test("(\\ x. x) bt", expcted);
}
TEST(TOKENIZER, T3) {
    TokenType expcted[] = {TokenType::bracket_open, TokenType::lambda,
                           TokenType::identifier, TokenType::body_start,
                           TokenType::identifier, TokenType::bracket_close,
                           TokenType::identifier};
    std_test("(\\x.x)bt", expcted);
}
TEST(TOKENIZER, T4) {
    TokenType expcted[] = {TokenType::bracket_open, TokenType::lambda,
                           TokenType::identifier, TokenType::body_start,
                           TokenType::identifier, TokenType::bracket_close,
                           TokenType::identifier, TokenType::conv_end};
    std_test("(\\x.x)bt>", expcted);
}
TEST(TOKENIZER, T5) {
    TokenType expcted[] = {TokenType::bracket_open, TokenType::lambda,
                           TokenType::identifier, TokenType::body_start,
                           TokenType::identifier, TokenType::bracket_close,
                           TokenType::identifier, TokenType::identifier,
                           TokenType::conv_end, TokenType::identifier};
    std_test("(\\x.x)bt bt>z", expcted);
}
TEST(TOKENIZER, T6) {
    TokenType expcted[] = {TokenType::bracket_open, TokenType::lambda,
                           TokenType::identifier, TokenType::body_start,
                           TokenType::identifier, TokenType::bracket_close,
                           TokenType::identifier, TokenType::literal,
                           TokenType::conv_end};
    std_test("(\\x.x)bt 77>", expcted);
}
TEST(TOKENIZER, T7) {
    TokenType expcted[] = {TokenType::name_define, TokenType::name,
                           TokenType::name_define, TokenType::assignment,
                           TokenType::lambda, TokenType::identifier,
                           TokenType::body_start, TokenType::identifier,
                           TokenType::separator, TokenType::bracket_open,
                           TokenType::name, TokenType::bracket_close,
                           TokenType::identifier, TokenType::separator};
    std_test("# this is a comment\n 'A' = \\ x . x; (A) y;", expcted);
}
TEST(TOKENIZER, T8) {
    TokenType expcted[] = {TokenType::name_define, TokenType::name,
                           TokenType::name_define, TokenType::assignment,
                           TokenType::lambda, TokenType::identifier,
                           TokenType::body_start, TokenType::identifier,
                           TokenType::separator};
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
    TokenType expcted[] = {TokenType::literal, TokenType::literal};
    std_test("true false", expcted);
}

TEST(TOKENIZER, EOF_exit) {
    stringstream ss;
    ss.str("");
    Tokenizer tz(ss);
    ASSERT_EQ(tz.get().tok, TokenType::undefined);
}

TEST(TOKENIZER, invalid_reserved1) {
    stringstream ss;
    ASSERT_THROW(Tokenizer tz(ss, {"?hallo"}), InvalidReservedSymbol);
}
