#pragma once
#include <string>
#include <cassert>

static inline char index(char c) {
    /**
     * returns the index of the char c in the alphabet, only lower case allowed
     */
    assert(islower(c));
    return c - 'a';
}

static std::string _trim_string(const std::string &str) {
    /**
     * trims str of leading and ending whitespaces
     * WARNING: this removes newline, too.
     * (IDEA: ignore newlines entirely, terminate expressions with ';')
     */
    size_t begin = 0;
    size_t end = str.size();
    for(char c = str[0]; isspace(c); c = str[++begin]);
    for(char c = str[end - 1]; isspace(c); c = str[--end - 1]);
    return str.substr(begin, end - begin);
}