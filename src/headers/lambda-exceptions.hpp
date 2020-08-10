#pragma once
#include <exception>
#include <string>
#include <iostream>

/**
 * ABSTRACT: This header contains all kinds of exceptions that could be thrown
 * in the other headers
 */

static const std::string SYNTAX_HEAD = "SyntaxError: ";

class LambdaException : public std::exception {};

class SyntaxException : public LambdaException {
  public:
    SyntaxException(std::string line) : line(SYNTAX_HEAD + line) {}
    SyntaxException() : line(SYNTAX_HEAD) {}
    std::string line;
    const char* what() const noexcept override {
        return line.c_str();
    }
};


class MaxIterationsExceeded : public LambdaException {
  public:
    const char* what() const noexcept override {
        return "Maximum iterations exceeded";
    }
};

class NameClash : public LambdaException {
  public:
    const char* what() const noexcept override {
        return "Requested name already exists";
    }
};

class ReservedSymbol : public LambdaException {
  public:
    ReservedSymbol(std::string symbol) : symbol(std::move(symbol)) {}
    std::string symbol;
};

  class InvalidReservedSymbol : public std::exception {
    public:
      InvalidReservedSymbol(std::string txt) : txt(txt) {}
      const char* what() const noexcept override {
          return txt.c_str();
      }
    private:
      std::string txt;
  };