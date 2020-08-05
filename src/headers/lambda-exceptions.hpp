#pragma once
#include <exception>
#include <string>
#include <iostream>

/**
 * ABSTRACT: This header contains all kinds of exceptions that could be thrown in the other headers
 */

static const std::string SYNTAX_HEAD = "Error: unexpected syntax!";


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
    SyntaxException(std::string line) : SyntaxParseException(SYNTAX_HEAD + line) {}
    SyntaxException() : SyntaxParseException(SYNTAX_HEAD) {}
};


class MaxIterationsExceeded : public std::exception {
  public:
    const char* what() const noexcept override {
        return "Maximum iterations exceeded!";
    }
};

class EmptyException : public std::exception {
public:
  const char* what() const noexcept override {
      return "Stream provided for syntax tree is empty!";
  }
};

class NameClash : public std::exception {
  public:
    const char* what() const noexcept override {
        return "Requested name already exists!";
    }
};
