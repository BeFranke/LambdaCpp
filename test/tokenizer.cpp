#include <iostream>
#include "gtest/gtest.h"
#include "../src/headers/tokenizer.hpp"


using namespace std;

TEST(TOKENIZER, T1) {
    stringstream ss;
    ss << "\\ x. x";
    TOKEN_TYPE expcted[] = {LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T2) {
    stringstream ss;
    ss << "(\\ x. x) bt";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T3) {
    stringstream ss;
    ss << "(\\x.x)bt";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T4) {
    stringstream ss;
    ss << "(\\x.x)bt>";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            CONV_END};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T5) {
    stringstream ss;
    ss << "(\\x.x)bt -bt>z";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            CONV_START, IDENTIFIER, CONV_END, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T6) {
    stringstream ss;
    ss << "(\\x.x)bt ->";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            CONV_START, CONV_END};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T7) {
    stringstream ss;
    ss << "(\\x.x)bt -77>";
    TOKEN_TYPE expcted[] = {BRACKET_OPEN, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, BRACKET_CLOSE, IDENTIFIER,
                            CONV_START, LITERAL, CONV_END};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T8) {
    stringstream ss;
    ss << "# this is a comment\n A = \\ x . x; (A) y;";
    TOKEN_TYPE expcted[] = {IDENTIFIER, ASSIGNMENT, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, SEPARATOR, BRACKET_OPEN,
                            IDENTIFIER, BRACKET_CLOSE, IDENTIFIER, SEPARATOR};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t; (t = tz.get()); ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T9) {
    stringstream ss;
    ss << "ID = \\ x. x;";
    TOKEN_TYPE expcted[] = {IDENTIFIER, ASSIGNMENT, LAMBDA, IDENTIFIER, BODY_START, IDENTIFIER, SEPARATOR};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T10) {
    stringstream ss;
    ss << "\\ ? . ?";
    Tokenizer tz(ss);
    tz.get();
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T11) {
    stringstream ss;
    ss << "/ x . x y";
    Tokenizer tz(ss);
    ASSERT_THROW(tz.get(), SyntaxException);
}