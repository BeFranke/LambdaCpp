#pragma once

#include "lambda-struct.hpp"

/**
 * ABSTRACT:
 * This file is used for encoding natural numbers into lambda calculus.
 * This also represents a possible extension to the framework: simple
 * mathematical operations could be supported
 */

/**
 * @param n natural number to encode
 * @return Expression pointer that represents this natural in church encoding
 */
Lambda_ptr church_encode(unsigned int n);

Lambda_ptr church_true();

Lambda_ptr church_false();