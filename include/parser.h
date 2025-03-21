#pragma once

#include "lexer.h"
#include "ast_node.h"
#include <memory>

class Parser
{
public:
    Parser(Lexer &lexer, const std::string& source);

    std::unique_ptr<ASTNode> parse();
    std::unique_ptr<ASTNode> parseProgram();  // Add new method

private:
    std::unique_ptr<ASTNode> parseModule();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseVariableDeclaration();
    std::unique_ptr<ASTNode> parseFunctionDeclaration();
    std::unique_ptr<ASTNode> parseReturnStatement();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parsePrimaryExpression();
    std::unique_ptr<ASTNode> parseFunctionCall(const std::string &functionName);
    std::unique_ptr<ASTNode> parseIfStatement();
    void parseImportStatement();

    void consume(TokenType type);
    void reportError(const std::string& message);

    // tokenToString helper function
    std::string tokenToString(TokenType type);

    std::string getSourceLine(int lineNumber) const;

private:
    Lexer &lexer_;
    Token current_token_;
    const std::string& source_;
};
