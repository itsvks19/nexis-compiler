#include "lexer.h"

#include <unordered_map>
#include <iostream>

Lexer::Lexer(const std::string &source) : source_(source), current_(0), line_(1), column_(1), length_(source.length()) {}

Token Lexer::getNextToken()
{
    skipWhitespace();

    Token token;
    token.line = line_;
    token.column = column_;

    if (current_ >= source_.length())
    {
        token.type = END_OF_FILE;
        return token;
    }

    // Save the starting position before consuming any characters
    int startColumn = column_;

    char c = source_[current_];

    if (isalpha(c) || c == '_')
    {
        return identifier();
    }
    else if (isdigit(c))
    {
        return number();
    }
    else if (c == '"')
    {
        return stringLiteral();
    }
    else if (c == '/' && current_ + 1 < source_.length() && source_[current_ + 1] == '/')
    {
        return singleLineComment();
    }
    else if (c == '/' && current_ + 1 < source_.length() && source_[current_ + 1] == '*')
    {
        return multiLineComment();
    }
    else
    {
        switch (c)
        {
        case '+':
        case '*':
        case '/':
        case '=':
            if (current_ + 1 < source_.length() && source_[current_ + 1] == '=') {
                current_ += 2;
                column_ += 2;
                return {OPERATOR, "==", line_};
            }
            current_++;
            column_++;
            return {OPERATOR, std::string(1, c), line_};
        case '-':
            if (current_ + 1 < source_.length() && source_[current_ + 1] == '>')
            {
                current_ += 2;
                column_ += 2;
                return {ARROW, "->", line_};
            }
            else
            {
                current_++;
                column_++;
                return {OPERATOR, "-", line_};
            }
        case '(':
            current_++;
            column_++;
            return {LPAREN, "(", line_};
        case ')':
            current_++;
            column_++;
            return {RPAREN, ")", line_};
        case '{':
            current_++;
            column_++;
            return {LBRACE, "{", line_};
        case '}':
            current_++;
            column_++;
            return {RBRACE, "}", line_};
        case ';':
            current_++;
            column_++;
            return {SEMICOLON, ";", line_};
        case '.':
            current_++;
            column_++;
            return {DOT, ".", line_};
        case ',':
            current_++;
            column_++;
            return {COMMA, ",", line_};
        case ':':
            current_++;
            column_++;
            return {COLON, ":", line_};
        case '<':
            if (current_ + 1 < source_.length() && source_[current_ + 1] == '=') {
                current_ += 2;
                column_ += 2;
                return {OPERATOR, "<=", line_};
            } else {
                current_++;
                column_++;
                return {OPERATOR, "<", line_};
            }
        case '>':
            if (current_ + 1 < source_.length() && source_[current_ + 1] == '=') {
                current_ += 2;
                column_ += 2;
                return {OPERATOR, ">=", line_};
            } else {
                current_++;
                column_++;
                return {OPERATOR, ">", line_};
            }
        default:
            std::cerr << "Lexical error at line " << line_ << ": Unexpected character '" << c << "'" << std::endl;
            return {END_OF_FILE, "", line_};
        }
    }

    // Update the token's column to where it started, not where it ended
    token.column = startColumn;
    return token;
}

void Lexer::skipWhitespace()
{
    while (current_ < source_.length())
    {
        char c = source_[current_];
        if (c == ' ' || c == '\t')
        {
            column_++;
            current_++;
        }
        else if (c == '\n')
        {
            line_++;
            column_ = 1;
            current_++;
        }
        else
        {
            break;
        }
    }

    // Handle comments after code
    if (current_ < source_.length() && source_[current_] == '/')
    {
        if (current_ + 1 < source_.length() && source_[current_ + 1] == '/')
        {
            singleLineComment();
        }
        else if (current_ + 1 < source_.length() && source_[current_ + 1] == '*')
        {
            multiLineComment();
        }
    }
}

Token Lexer::identifier()
{
    std::string id;
    while (current_ < source_.length() && (isalnum(source_[current_]) || source_[current_] == '_'))
    {
        id += source_[current_];
        current_++;
        column_++;
    }

    // Check for keywords
    if (id == "module")
        return {MODULE, id, line_};
    if (id == "import")
        return {IMPORT, id, line_};
    if (id == "func")
        return {FUNC, id, line_};
    if (id == "let")
        return {LET, id, line_};
    if (id == "var")
        return {VAR, id, line_};
    if (id == "if")
        return {IF, id, line_};
    if (id == "else")
        return {ELSE, id, line_};
    if (id == "for")
        return {FOR, id, line_};
    if (id == "while")
        return {WHILE, id, line_};
    if (id == "await")
        return {AWAIT, id, line_};
    if (id == "return")
        return {RETURN, id, line_};
    if (id == "true")
        return {BOOLEAN, "true", line_};
    if (id == "false")
        return {BOOLEAN, "false", line_};

    return {IDENTIFIER, id, line_};
}

Token Lexer::number()
{
    std::string num;
    while (current_ < source_.length() && isdigit(source_[current_]))
    {
        num += source_[current_];
        current_++;
        column_++;
    }

    return {NUMBER, num, line_};
}

Token Lexer::stringLiteral()
{
    current_++; // Skip the opening quote
    column_++;
    std::string str;
    while (current_ < source_.length() && source_[current_] != '"')
    {
        str += source_[current_];
        current_++;
        column_++;
    }

    if (current_ < source_.length() && source_[current_] == '"')
    {
        current_++; // Skip the closing quote
        column_++;
        return {STRING, str, line_};
    }
    else
    {
        std::cerr << "Lexical error at line " << line_ << ": Unterminated string literal" << std::endl;
        return {END_OF_FILE, "", line_};
    }
}

Token Lexer::singleLineComment()
{
    current_ += 2; // Skip the "//"
    column_ += 2;
    while (current_ < source_.length() && source_[current_] != '\n')
    {
        current_++;
        column_++;
    }
    skipWhitespace();
    return {COMMENT, "", line_};
}

Token Lexer::multiLineComment()
{
    current_ += 2; // Skip the "/*"
    column_ += 2;
    while (current_ + 1 < source_.length() && !(source_[current_] == '*' && source_[current_ + 1] == '/'))
    {
        if (source_[current_] == '\n')
        {
            line_++;
            column_ = 1;
        }
        else
        {
            column_++;
        }
        current_++;
    }
    current_ += 2; // Skip the "*/"
    column_ += 2;
    skipWhitespace();
    return {COMMENT, "", line_};
}
