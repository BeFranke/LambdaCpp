#include <iostream>
#include "gtest/gtest.h"
#include "../src/headers/tokenizer.hpp"

using namespace std;

TEST(TOKENIZER, T1) {
    stringstream ss;
    ss << "\\ x. x";
    TOKEN_TYPE expcted[] = {OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T2) {
    stringstream ss;
    ss << "(\\ x. x) bt";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T3) {
    stringstream ss;
    ss << "(\\x.x)bt";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T4) {
    stringstream ss;
    ss << "(\\x.x)bt>";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, COMMAND};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T5) {
    stringstream ss;
    ss << "(\\x.x)bt -bt>z";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, COMMAND};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T6) {
    stringstream ss;
    ss << "(\\x.x)bt ->";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, COMMAND};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T7) {
    stringstream ss;
    ss << "(\\x.x)bt -77>";
    TOKEN_TYPE expcted[] = {SEPARATOR, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, COMMAND};
    Tokenizer tz(ss);
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expcted[i]);
    }
}
TEST(TOKENIZER, T8) {
    stringstream ss;
    ss << "-a>";
    Tokenizer tz(ss);
    ASSERT_THROW(tz.get(), SyntaxException);
}