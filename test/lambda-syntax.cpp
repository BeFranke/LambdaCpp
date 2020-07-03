#include "gtest/gtest.h"
#include "../headers/lambda-syntax.hpp"

TEST(TRIM_TEST, Main) {
    ASSERT_EQ("test", _trim_string("     test"));
    ASSERT_EQ("te  st", _trim_string("   te  st"));
    ASSERT_EQ("tes  t", _trim_string("   tes  t  "));
}