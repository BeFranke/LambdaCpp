#include "lambda-struct.hpp"

std::ostream& operator<<(std::ostream& os, const Expression& ex) {
    return ex.print(os);
}
