#include <vector>
#include "gtest/gtest.h"
#include "../src/headers/lambda-syntax.hpp"

class SyntaxTest : public ::testing::Test {
  public:
    SyntaxTest() : is(), os(), p(is) {}
    std::stringstream is;
    std::stringstream os;
    Parser p;
};

// simple positive tests

TEST_F(SyntaxTest, VariableParses) {
    is << "x;";
    auto result = std::dynamic_pointer_cast<Variable>(p.program().last_command().execute());
    ASSERT_EQ(result->get_name(), "x");
}

TEST_F(SyntaxTest, LambdaParsesSimple) {
    is << "\\ x . x;";
    auto result = std::dynamic_pointer_cast<Lambda>(p.program().last_command().execute());
    os << *result;
    ASSERT_EQ(os.str(), "\\x . x");
}

TEST_F(SyntaxTest, ApplicationParsesSimple) {
    is << "(f) x;";
    auto result = std::dynamic_pointer_cast<Application>(p.program().last_command().execute());
    os << *result;
    ASSERT_EQ(os.str(), "(f) x");
}

TEST_F(SyntaxTest, AssignmentParsesSimple) {
    is << "A = a;";
    auto prog = p.program();
    ASSERT_NE(prog.symbols().find("A"), prog.symbols().end());
    auto expr = std::dynamic_pointer_cast<Variable>(prog.symbols()["A"].execute());
    ASSERT_EQ(expr->get_name(), "a");
}

TEST_F(SyntaxTest, AlphaParsesSimple) {
    is << "\\ x . x -x>a;";
    auto lambda = p.program().last_command().execute();
    os << *lambda;
    ASSERT_EQ(os.str(), "\\a . a");
}

TEST_F(SyntaxTest, BetaParsesSimple) {
    is << "(\\ x . x) a >;";
    auto var = p.program().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

// exception tests

TEST_F(SyntaxTest, NoSemicolon) {
    is << "\\ x . x";
    ASSERT_THROW(p.program(), SyntaxException);
}

TEST_F(SyntaxTest, IllegalAssignment) {
    is << "\\ x. x = (f) a;";
    ASSERT_THROW(p.program(), SyntaxException);
}

TEST_F(SyntaxTest, IllegalReduction) {
    is << "(f) a -6>t;";
    ASSERT_THROW(p.program(), SyntaxException);
}

TEST_F(SyntaxTest, WrongVariableCase) {
    is << "(f) A;";
    ASSERT_THROW(p.program(), SyntaxException);
}

TEST_F(SyntaxTest, WrongVariableCase2) {
    is << "a = (f) b;";
    ASSERT_THROW(p.program(), SyntaxException);
}

// complex positive tests

TEST_F(SyntaxTest, MultiLine) {
    is << "A = \\ x . \n x";
    auto var = p.program().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "(\\x . x) y");
}
