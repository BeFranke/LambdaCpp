#pragma once
#include <exception>
#include <string>
#include <iostream>

/**
 * ABSTRACT: This header contains all kinds of exceptions that could be thrown
 * in the other headers
 */

static const std::string SYNTAX_HEAD = "Error: unexpected syntax! ";


class SyntaxParseException : public std::exception {
  public:
    SyntaxParseException() = delete;
    SyntaxParseException(std::string text) : std::exception(), text(text) {}
    const char* what() const noexcept override {
        return text.c_str();
    }
  private:
    std::string text;
};

class SyntaxException : public SyntaxParseException {
  public:
    SyntaxException(std::string line) : SyntaxParseException(SYNTAX_HEAD + line)
    {}
    SyntaxException() : SyntaxParseException(SYNTAX_HEAD) {}
};


class MaxIterationsExceeded : public std::exception {
  public:
    const char* what() const noexcept override {
        return "Maximum iterations exceeded!";
    }
};

class NameClash : public std::exception {
  public:
    const char* what() const noexcept override {
        return "Requested name already exists!";
    }
};

class ReservedSymbol : public std::exception {
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