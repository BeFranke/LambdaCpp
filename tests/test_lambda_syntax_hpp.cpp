#include <cassert>
#include <cstring>
#include "../lambda-syntax.hpp"

int main() {
    std::string tst = "\\x .x ";
    Lambda* x = (Lambda*)from_string(tst);
    assert(x->to_string() == tst);
}
