#include <vector>
#include "gtest/gtest.h"
#include "../src/lib/lambda-syntax.hpp"

class SyntaxTest : public ::testing::Test {
  public:
    SyntaxTest() : is(), os(), p(is) {}
    std::stringstream is;
    std::stringstream os;
    Parser<> p;
};

// positive tests

TEST_F(SyntaxTest, VariableParses) {
    is << "x;";
    auto result = std::dynamic_pointer_cast<const Variable>(
            p.statement().last_command().execute()
            );
    ASSERT_EQ(result->get_name(), "x");
}

TEST_F(SyntaxTest, LambdaParsesSimple) {
    is << "\\ x . x;";
    auto result = std::dynamic_pointer_cast<const Lambda>(
            p.statement().last_command().execute()
            );
    os << *result;
    ASSERT_EQ(os.str(), "\\x . x");
}

TEST_F(SyntaxTest, ApplicationParsesSimple) {
    is << "(f) x;";
    auto result = std::dynamic_pointer_cast<const Application>(
            p.statement().last_command().execute()
            );
    os << *result;
    ASSERT_EQ(os.str(), "(f) x");
}

TEST_F(SyntaxTest, AssignmentParsesSimple) {
    is << "'A' = a;";
    auto prog = p.statement();
    ASSERT_TRUE(prog.contains("A"));
    auto expr = std::dynamic_pointer_cast<const Variable>(
            prog["A"].execute()
            );
    ASSERT_EQ(expr->get_name(), "a");
}

TEST_F(SyntaxTest, AlphaParsesSimple) {
    is << "\\ x . x x>a;";
    auto lambda = p.statement().last_command().execute();
    os << *lambda;
    ASSERT_EQ(os.str(), "\\a . a");
}

TEST_F(SyntaxTest, BetaParsesSimple) {
    is << "(\\ x . x) a >;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

TEST_F(SyntaxTest, BetaParses) {
    is << "(\\ y . (\\ x . x) y ) a 9>;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

TEST_F(SyntaxTest, Retrieval1) {
    is << "'ID' = \\ x . x;";
    p.statement();
    is << "ID;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "\\x . x");
}

TEST_F(SyntaxTest, Retrieval2) {
    is << "'ID' = (\\ x . x) a >;";
    p.statement();
    is << "ID;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

TEST_F(SyntaxTest, Retrieval3) {
    is << "'ID' = (\\ x . x) a;";
    p.statement();
    is << "ID >;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

TEST_F(SyntaxTest, Literal) {
    is << "(1) 1 >;";
    auto var = p.statement().last_command().execute()->alpha_convert("x", "y");
    os << *var;
    ASSERT_EQ(os.str(), "\\y . \\x . (y) x");
}

TEST_F(SyntaxTest, Retrieval4) {
    is << "'ID' = \\ x . x;";
    p.statement();
    is << "(ID) a >;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "a");
}

TEST_F(SyntaxTest, test_true) {
    is << "true;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "\\a . \\b . a");
}
TEST_F(SyntaxTest, test_false) {
    is << "false;";
    auto var = p.statement().last_command().execute();
    os << *var;
    ASSERT_EQ(os.str(), "\\a . \\b . b");
}

// negative tests

TEST_F(SyntaxTest, NoSemicolon) {
    is << "\\ x . x";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, IllegalAssignment) {
    is << "\\ x. x = (f) a;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, IllegalReduction) {
    is << "(f) a 6>t;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, WrongVariableCase) {
    is << "(f) A;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, WrongVariableCase2) {
    is << "a = (f) b;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, WrongBeta) {
    is << "(\\ x . x) a true>;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, WrongAssign) {
    is << "'?' = x;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, UnclosedAssign) {
    is << "'A = x;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, Unassign) {
    is << "'A';";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, NoBody) {
    is << "\\ x y;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, Brackets) {
    is << "(f x;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, ExpressionWrongToken) {
    is << "'A' = =;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, BetaUnclosed) {
    is << "(\\x . x) a 1;";
    ASSERT_THROW(p.statement(), SyntaxException);
}

TEST_F(SyntaxTest, MaxIter) {
    Parser<> p1 = Parser(is, 10);
    is << "(\\x . (x) x) \\y . (y) y >;";
    ASSERT_THROW(p1.statement().last_command().execute(),
                 MaxIterationsExceeded);
}

TEST_F(SyntaxTest, MaxIterException) {
    Parser<> p1 = Parser(is, 10);
    is << "(\\x . (x) x) \\y . (y) y >;";
    try{
        p1.statement().last_command().execute();
        ASSERT_TRUE(false);
    } catch(MaxIterationsExceeded& e) {
        ASSERT_EQ(std::string(e.what()), "Maximum iterations exceeded");
    }
}

TEST_F(SyntaxTest, lowercaseAssign) {
    is << "'a' = 5;";
    ASSERT_THROW(p.statement(),SyntaxException);
}
