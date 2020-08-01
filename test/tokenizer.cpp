#include <iostream>
#include "gtest/gtest.h"
#include "../src/headers/tokenizer.hpp"


// TODO fix this mess


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
TEST(TOKENIZER, T9) {
    stringstream ss;
    ss << "ID = \\ x. x";
    TOKEN_TYPE expcted[] = {IDENTIFIER, COMMAND, OPERATOR, IDENTIFIER, OPERATOR, IDENTIFIER};
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
TEST(VALID_CC, T1) {
    Token t = Token();
    t.str = "x", t.tok = LITERAL;
    ASSERT_FALSE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T2) {
    Token t = Token();
    t.str = ">", t.tok = COMMAND;
    ASSERT_TRUE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T3) {
    Token t = Token();
    t.str = "-5>", t.tok = COMMAND;
    ASSERT_TRUE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T4) {
    Token t = Token();
    t.str = "->", t.tok = COMMAND;
    ASSERT_TRUE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T5) {
    Token t = Token();
    t.str = "-x>y", t.tok = COMMAND;
    ASSERT_TRUE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T6) {
    Token t = Token();
    t.str = "x->", t.tok = COMMAND;
    ASSERT_FALSE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T7) {
    Token t = Token();
    t.str = "-x>", t.tok = COMMAND;
    ASSERT_FALSE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T8) {
    Token t = Token();
    t.str = "-5>x", t.tok = COMMAND;
    ASSERT_FALSE(valid_conversion_cmd(t));
}
TEST(VALID_CC, T9) {
    Token t = Token();
    t.str = "-77>", t.tok = COMMAND;
    ASSERT_TRUE(valid_conversion_cmd(t));
}