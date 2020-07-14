#include "gtest/gtest.h"
#include "../headers/lambda-syntax.hpp"

TEST(REDUCTION, TEST1) {
    Expression_ptr e = from_string("(\\ x . h x l l o ) a");
    ASSERT_TRUE(e->reduce_first());
    Expression_ptr e1 = std::static_pointer_cast<Application>(e)->get_body(0);
    Application_ptr ap = std::static_pointer_cast<Lambda>(e1)->empty_head_to_application();
    ASSERT_EQ(ap->to_string(), "h a l l o ");
}