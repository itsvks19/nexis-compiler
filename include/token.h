#pragma once

#include <string>

enum TokenType
{
    MODULE,
    IMPORT,
    FUNC,
    LET,
    VAR,
    IF,
    ELSE,
    FOR,
    WHILE,
    SPAWN,
    AWAIT,
    RETURN,
    IDENTIFIER,
    NUMBER,
    STRING,
    BOOLEAN,
    OPERATOR,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    DOT,
    COMMA,
    COLON,
    ARROW,
    COMMENT,
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;  // Add column tracking
};
