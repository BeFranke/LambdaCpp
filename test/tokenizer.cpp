#include <iostream>
#include "gtest/gtest.h"
#include "../src/lib/tokenizer.hpp"

using namespace std;



void std_test(std::string input, TokenType expected[]) {
    stringstream ss;
    ss << input;
    //Tokenizer tz(ss);
    auto tz = Tokenizer<>{ss};
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
    bool called = false;
    stringstream ss;
    ss << "\\ ? . ?";
    Tokenizer<> tz{ss};
    auto func = [&called](){called = true;};
    tz.register_symbol("?", func);
    tz.get(), tz.get();
    ASSERT_TRUE(called);
}
TEST(TOKENIZER, T10) {
    stringstream ss;
    ss << "/ x . x y";
    Tokenizer<> tz{ss};
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T11) {
    stringstream ss;
    ss << "xy?";
    Tokenizer<> tz{ss};
    ASSERT_THROW(tz.get(), SyntaxException);
}
TEST(TOKENIZER, T12) {
    bool called = false;
    stringstream ss;
    ss << "?xyz";
    Tokenizer<> tz{ss};
    tz.register_symbol("?", [&called](){called = true;});
    tz.get();
    ASSERT_TRUE(called);
}
TEST(TOKENIZER, T13) {
    bool called = false;
    stringstream ss;
    ss << "exit";
    Tokenizer<> tz{ss};
    tz.register_symbol("exit", [&called](){called = true;});
    tz.get();
    ASSERT_TRUE(called);
}

TEST(TOKENIZER, T14) {
    TokenType expcted[] = {TokenType::literal, TokenType::literal};
    std_test("true false", expcted);
}

TEST(TOKENIZER, EOF_exit) {
    stringstream ss;
    ss.str("");
    Tokenizer<> tz{ss};
    ASSERT_EQ(tz.get().tok, TokenType::undefined);
}

TEST(TOKENIZER, invalid_reserved1) {
    stringstream ss;
    Tokenizer<> tz(ss);
    ASSERT_THROW(tz.register_symbol("?hallo", []() {return;}),
                 InvalidReservedSymbol);
}
TEST(TOKENIZER, invalid_reserved_text) {
    stringstream ss;
    Tokenizer<> tz(ss);
    try {
        tz.register_symbol("?hallo", []() {});
        FAIL();
    }
    catch (InvalidReservedSymbol& e) {
        ASSERT_EQ(
                "Only lowercase words or single characters may be reserved",
                std::string(e.what())
                );
    }
}
TEST(TOKENIZER, overwriteSymbol) {
    enum class MySymbol {
        lambda = '\\',
        body_start = '.',
        bracket_open = '(',
        bracket_close = ')',
        // this one has changed
        separator = '\n',
        comment = '#',
        assignment = '=',
        conversion_end = '>',
        name_definition = '\''
    };
    stringstream ss;
    Tokenizer<MySymbol> tz{ss};
    TokenType expected[] = {TokenType::lambda, TokenType::identifier,
                            TokenType::body_start, TokenType::identifier,
                            TokenType::separator};
    ss << "\\x.x\n";
    unsigned short i = 0;
    for(Token t = tz.get(); t; t = tz.get(), ++i) {
        ASSERT_EQ(t.tok, expected[i]);
    }
    ASSERT_GE(i, 1);
}
TEST(TOKENIZER, unregister) {
    bool called = false;
    stringstream ss;
    ss << "xyz";
    Tokenizer<> tz{ss};
    tz.register_symbol("xyz", [&called](){called = true;});
    tz.unregister_symbol("xyz");
    tz.get();
    ASSERT_FALSE(called);
}


/* This test does intentionally not compile, it is there to assert that
 * trying to specify a replacement for the Symbol class that does not define
 * every needed symbol leads to a compilation error.
TEST(TOKENIZER, overwriteSymbolError) {
    enum class MySymbol {
        //body_start = '.',
        bracket_open = '(',
        bracket_close = ')',
        // this one has changed
        separator = '\n',
        comment = '#',
        assignment = '=',
        conversion_end = '>',
        name_definition = '\''
    };
    stringstream ss;
    Tokenizer<MySymbol> tz(ss);
    tz.get();
}*/