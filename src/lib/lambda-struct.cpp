#include "lambda-struct.hpp"

/**
 * This method was moved to the .cpp file to avoid linking errors
 * that occur when e.g. linking all test files together
 */
std::ostream& operator<<(std::ostream& os, const Expression& ex) {
    return ex.print(os);
}
