#pragma once

#include "token.h"

#include <string>
#include <vector>

class Lexer
{
private:
    std::string source_;
    size_t current_;
    int line_;
    int column_;
    size_t length_;

public:
    Lexer(const std::string &source);
    
    Token getNextToken();
    std::pair<int, int> getCurrentPosition() const { return {line_, column_}; }

private:
    void skipWhitespace();
    Token identifier();
    Token number();
    Token stringLiteral();
    Token singleLineComment();
    Token multiLineComment();
    void advanceColumn(int count = 1) { column_ += count; }
    void advanceLine() { line_++; column_ = 1; }
    char peek() const { return current_ < length_ ? source_[current_] : '\0'; }
    char advance() { column_++; return current_ < length_ ? source_[current_++] : '\0'; }
};
