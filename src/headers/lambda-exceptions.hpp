#pragma once
#include <exception>
#include <string>
#include <iostream>

static const std::string REDECL_HEAD = "Error: variable re-declared:";
static const std::string SYNTAX_HEAD = "Error: unexpected syntax!";
static const std::string EMPTY_HEAD = "Error: empty body is not allowed!";
static const std::string START_HEAD = "Error: illegal start of expression!";
static const std::string END_HEAD = "Error: unexpected end of expression!";

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

class ReDeclarationException : public SyntaxParseException {
  public:
    ReDeclarationException(std::string& line) : SyntaxParseException(REDECL_HEAD + line) {}
};

class SyntaxException : public SyntaxParseException {
  public:
    SyntaxException(std::string& line) : SyntaxParseException(SYNTAX_HEAD + line) {}
    SyntaxException() : SyntaxParseException(SYNTAX_HEAD) {}
};

class EmptyException : public SyntaxParseException {
  public:
    EmptyException(std::string& line) : SyntaxParseException(EMPTY_HEAD + line) {}
};

class StartException : public SyntaxParseException {
  public:
    StartException(std::string& line) : SyntaxParseException(START_HEAD + line) {}
};

class EndException : public SyntaxParseException {
  public:
    EndException(std::string& line) : SyntaxParseException(END_HEAD + line) {}
};

class MaxIterationsExceeded : public std::exception {
  public:
    const char* what() const noexcept override {
        return "Maximum iterations exceeded!";
    }
};
