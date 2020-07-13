#pragma once
#include <exception>
#include <string>
#include <iostream>

static const std::string REDECL_HEAD = "Error: variable re-declared:";
static const std::string SYNTAX_HEAD = "Error: unexpected syntax!";
static const std::string EMPTY_HEAD = "Error: empty tail is not allowed!";
static const std::string START_HEAD = "Error: illegal start of expression!";
static const std::string END_HEAD = "Error: unexpected end of expression!";

class AbstractLambdaException : public std::exception {
  public:
    AbstractLambdaException() = delete;
    AbstractLambdaException(std::string text) : std::exception(), text(text) {}
    const char* what() const noexcept override {
        return text.c_str();
    }
  private:
    std::string text;
};

class ReDeclarationException : public AbstractLambdaException {
  public:
    ReDeclarationException(std::string& line) : AbstractLambdaException(REDECL_HEAD + line) {}
};

class SyntaxException : public AbstractLambdaException {
  public:
    SyntaxException(std::string& line) : AbstractLambdaException(SYNTAX_HEAD + line) {}
};

class EmptyException : public AbstractLambdaException {
  public:
    EmptyException(std::string& line) : AbstractLambdaException(EMPTY_HEAD + line) {}
};

class StartException : public AbstractLambdaException {
  public:
    StartException(std::string& line) : AbstractLambdaException(START_HEAD + line) {}
};

class EndException : public AbstractLambdaException {
  public:
    EndException(std::string& line) : AbstractLambdaException(END_HEAD + line) {}
};