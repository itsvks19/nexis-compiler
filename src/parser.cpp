#include "parser.h"
#include "token.h"
#include <iostream>
#include "module_manager.h"

Parser::Parser(Lexer &lexer, const std::string& source) 
        : lexer_(lexer), current_token_(lexer.getNextToken()), source_(source) {}

std::unique_ptr<ASTNode> Parser::parse()
{
    return parseProgram();
}

std::unique_ptr<ASTNode> Parser::parseProgram()
{
    auto programNode = std::make_unique<ModuleNode>();
    programNode->name = "Program";  // Root node containing all modules

    // Parse modules until end of file
    while (current_token_.type != END_OF_FILE) {
        if (current_token_.type == MODULE) {
            auto moduleNode = parseModule();
            if (moduleNode) {
                programNode->body.push_back(std::move(moduleNode));
            }
        } else {
            // Skip unexpected tokens between modules
            consume(current_token_.type);
        }
    }

    return programNode;
}

std::unique_ptr<ASTNode> Parser::parseModule()
{
    if (current_token_.type != MODULE)
    {
        std::cerr << "Syntax error: Expected 'module' keyword" << std::endl;
        return nullptr;
    }
    consume(MODULE);

    if (current_token_.type != IDENTIFIER)
    {
        std::cerr << "Syntax error: Expected module name" << std::endl;
        return nullptr;
    }
    std::string moduleName = current_token_.value;
    consume(IDENTIFIER);

    if (current_token_.type != LBRACE)
    {
        std::cerr << "Syntax error: Expected '{'" << std::endl;
        return nullptr;
    }
    consume(LBRACE);

    auto moduleNode = std::make_unique<ModuleNode>();
    moduleNode->name = moduleName;

    // Register the module itself
    ModuleManager::getInstance().registerModule(moduleName);

    while (current_token_.type != RBRACE)
    {
        if (current_token_.type == COMMENT)
        {
            consume(COMMENT);
            continue;
        }
        if (current_token_.type == IMPORT)
        {
            parseImportStatement();
            continue;
        }
        if (current_token_.type == FUNC)
        {
            auto funcNode = parseFunctionDeclaration();
            if (funcNode) {
                // Register the function with the module manager
                if (auto fn = dynamic_cast<FunctionNode*>(funcNode.get())) {
                    std::string qualifiedName = moduleName + "." + fn->name;
                    ModuleManager::getInstance().registerUserDefinedFunction(
                        moduleName,
                        fn->name,
                        fn->parameters,
                        fn->returnType,
                        funcNode->clone()
                    );
                }
                moduleNode->body.push_back(std::move(funcNode));
            }
            continue;
        }
        auto statement = parseStatement();
        if (statement)
        {
            moduleNode->body.push_back(std::move(statement));
        }
        else
        {
            consume(current_token_.type); // Skip unexpected tokens
        }
    }

    consume(RBRACE);
    return moduleNode;
}

void Parser::parseImportStatement()
{
    consume(IMPORT);

    std::string modulePath;
    
    // Parse module path (e.g., std.io)
    while (current_token_.type == IDENTIFIER) {
        modulePath += current_token_.value;
        consume(IDENTIFIER);
        
        if (current_token_.type == DOT) {
            modulePath += ".";
            consume(DOT);
        } else {
            break;
        }
    }

    if (modulePath.empty()) {
        std::cerr << "Syntax error: Expected module name after 'import'" << std::endl;
        return;
    }

    // Register the imported module
    ModuleManager::getInstance().registerModule(modulePath);

    if (current_token_.type != SEMICOLON)
    {
        std::cerr << "Syntax error: Expected ';' after import statement" << std::endl;
        return;
    }
    consume(SEMICOLON);
}

std::unique_ptr<ASTNode> Parser::parseStatement()
{
    switch (current_token_.type)
    {
    case IF:
        return parseIfStatement();
    case LET:
    case VAR:
        return parseVariableDeclaration();
    case FUNC:
        return parseFunctionDeclaration();
    case RETURN:
        return parseReturnStatement();
    case IDENTIFIER:
        {
            auto expr = parseExpression();
            if (!expr) {
                return nullptr;
            }
            if (current_token_.type != SEMICOLON) {
                reportError("Missing semicolon at end of statement");
                // Skip until semicolon or end of statement
                while (current_token_.type != SEMICOLON && 
                       current_token_.type != RBRACE && 
                       current_token_.type != END_OF_FILE) {
                    current_token_ = lexer_.getNextToken();
                }
                if (current_token_.type == SEMICOLON) {
                    consume(SEMICOLON);
                }
                return nullptr;
            }
            consume(SEMICOLON);
            return expr;
        }
    default:
        if (current_token_.type == COMMENT)
        {
            consume(COMMENT);
            return nullptr;
        }
        std::cerr << "Syntax error: Unexpected token" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<ASTNode> Parser::parseIfStatement()
{
    consume(IF);
    consume(LPAREN);
    
    auto condition = parseExpression();
    if (!condition) {
        return nullptr;
    }
    
    consume(RPAREN);
    consume(LBRACE);
    
    auto ifNode = std::make_unique<IfStatementNode>();
    ifNode->condition = std::move(condition);
    
    while (current_token_.type != RBRACE && current_token_.type != END_OF_FILE) {
        auto statement = parseStatement();
        if (statement) {
            ifNode->thenBranch.push_back(std::move(statement));
        }
    }
    
    consume(RBRACE);
    
    if (current_token_.type == ELSE) {
        consume(ELSE);
        consume(LBRACE);
        
        while (current_token_.type != RBRACE && current_token_.type != END_OF_FILE) {
            auto statement = parseStatement();
            if (statement) {
                ifNode->elseBranch.push_back(std::move(statement));
            }
        }
        
        consume(RBRACE);
    }
    
    return ifNode;
}

std::unique_ptr<ASTNode> Parser::parseVariableDeclaration()
{
    bool isMutable = current_token_.type == VAR;
    consume(isMutable ? VAR : LET);

    if (current_token_.type != IDENTIFIER)
    {
        std::cerr << "Syntax error: Expected variable name" << std::endl;
        return nullptr;
    }
    std::string variableName = current_token_.value;
    consume(IDENTIFIER);

    std::string type;
    if (current_token_.type == COLON)
    {
        consume(COLON);
        if (current_token_.type != IDENTIFIER)
        {
            std::cerr << "Syntax error: Expected type" << std::endl;
            return nullptr;
        }
        type = current_token_.value;
        consume(IDENTIFIER);
    }

    auto variableDeclarationNode = std::make_unique<VariableDeclarationNode>();
    variableDeclarationNode->name = variableName;
    variableDeclarationNode->type = type;
    variableDeclarationNode->isMutable = isMutable;

    if (current_token_.type == OPERATOR || current_token_.value == "=")
    {
        consume(OPERATOR);
        variableDeclarationNode->initializer = parseExpression();
    }

    consume(SEMICOLON);
    return variableDeclarationNode;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration()
{
    consume(FUNC);

    if (current_token_.type != IDENTIFIER)
    {
        std::cerr << "Syntax error: Expected function name" << std::endl;
        return nullptr;
    }
    std::string functionName = current_token_.value;
    consume(IDENTIFIER);

    consume(LPAREN);
    std::vector<FunctionNode::Parameter> parameters;
    
    // Parse parameters with types
    while (current_token_.type == IDENTIFIER)
    {
        FunctionNode::Parameter param;
        param.name = current_token_.value;
        consume(IDENTIFIER);
        
        if (current_token_.type == COLON) {
            consume(COLON);
            if (current_token_.type != IDENTIFIER) {
                std::cerr << "Syntax error: Expected parameter type" << std::endl;
                return nullptr;
            }
            param.type = current_token_.value;
            consume(IDENTIFIER);
        }
        
        parameters.push_back(param);
        
        if (current_token_.type == COMMA)
        {
            consume(COMMA);
            if (current_token_.type != IDENTIFIER) {
                std::cerr << "Syntax error: Expected parameter after comma" << std::endl;
                return nullptr;
            }
        }
    }
    consume(RPAREN);

    if (current_token_.type != ARROW) {
        std::cerr << "Syntax error: Expected '->' after parameters" << std::endl;
        return nullptr;
    }
    consume(ARROW);

    if (current_token_.type != IDENTIFIER)
    {
        std::cerr << "Syntax error: Expected return type" << std::endl;
        return nullptr;
    }
    std::string returnType = current_token_.value;
    consume(IDENTIFIER);

    if (current_token_.type != LBRACE) {
        std::cerr << "Syntax error: Expected '{' after return type" << std::endl;
        return nullptr;
    }
    consume(LBRACE);

    auto funcNode = std::make_unique<FunctionNode>();
    funcNode->name = functionName;
    funcNode->parameters = parameters;
    funcNode->returnType = returnType;

    while (current_token_.type != RBRACE && current_token_.type != END_OF_FILE)
    {
        if (current_token_.type == COMMENT)
        {
            consume(COMMENT);
            continue;
        }
        
        auto statement = parseStatement();
        if (statement)
        {
            funcNode->body.push_back(std::move(statement));
        }
    }

    if (current_token_.type != RBRACE) {
        std::cerr << "Syntax error: Expected '}' at end of function" << std::endl;
        return nullptr;
    }
    consume(RBRACE);
    
    return funcNode;
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement()
{
    consume(RETURN);
    auto expression = parseExpression();
    consume(SEMICOLON);
    return expression;
}

std::unique_ptr<ASTNode> Parser::parseExpression()
{
    auto left = parsePrimaryExpression();

    while (current_token_.type == OPERATOR)
    {
        std::string op = current_token_.value;
        consume(OPERATOR);
        auto right = parsePrimaryExpression();
        auto binaryOpNode = std::make_unique<BinaryOperationNode>();
        binaryOpNode->op = op;
        binaryOpNode->left = std::move(left);
        binaryOpNode->right = std::move(right);
        left = std::move(binaryOpNode);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parsePrimaryExpression()
{
    if (current_token_.type == NUMBER || current_token_.type == STRING || current_token_.type == BOOLEAN)
    {
        auto literalNode = std::make_unique<LiteralNode>();
        literalNode->value = current_token_.value;
        literalNode->type = current_token_.type == NUMBER ? "int" : (current_token_.type == STRING ? "string" : "boolean");
        consume(current_token_.type);
        return literalNode;
    }
    else if (current_token_.type == IDENTIFIER)
    {
        std::string identifier = current_token_.value;
        consume(IDENTIFIER);

        // Handle method calls (e.g., Math.square)
        if (current_token_.type == DOT)
        {
            consume(DOT);
            if (current_token_.type != IDENTIFIER)
            {
                std::cerr << "Syntax error: Expected method name after '.'" << std::endl;
                return nullptr;
            }
            std::string methodName = identifier + "." + current_token_.value;
            consume(IDENTIFIER);

            if (current_token_.type == LPAREN)
            {
                // Don't consume semicolon here - let parseStatement handle it
                return parseFunctionCall(methodName);
            }
        }
        // Handle direct function calls
        else if (current_token_.type == LPAREN)
        {
            // Don't consume semicolon here - let parseStatement handle it
            return parseFunctionCall(identifier);
        }

        // If it's just an identifier
        auto literalNode = std::make_unique<LiteralNode>();
        literalNode->value = identifier;
        literalNode->type = "identifier";
        return literalNode;
    }

    std::cerr << "Syntax error: Unexpected token '" << current_token_.value << "'" << std::endl;
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall(const std::string& functionName) {
    auto& mm = ModuleManager::getInstance();
    
    // For module-qualified calls, verify module is imported
    size_t dotPos = functionName.find('.');
    if (dotPos != std::string::npos) {
        std::string moduleName = functionName.substr(0, dotPos);
        if (!mm.isModuleImported(moduleName) && !mm.isModuleImported("std." + moduleName)) {
            std::cerr << "Error: Module '" << moduleName << "' not imported" << std::endl;
            return nullptr;
        }
    }

    // Create function call node even if function is not found yet
    // This allows for forward declarations and runtime function registration
    auto functionCallNode = std::make_unique<FunctionCallNode>();
    functionCallNode->name = functionName;

    consume(LPAREN);

    // Parse arguments
    while (current_token_.type != RPAREN) {
        auto argument = parseExpression();
        if (argument) {
            functionCallNode->arguments.push_back(std::move(argument));
        }

        if (current_token_.type == COMMA) {
            consume(COMMA);
        } else if (current_token_.type != RPAREN) {
            std::cerr << "Syntax error: Expected ',' or ')'" << std::endl;
            return nullptr;
        }
    }

    consume(RPAREN);
    return functionCallNode;
}

void Parser::consume(TokenType type)
{
    // debug print
    // std::cout << "Consume: " << current_token_.value << " (Type " << tokenToString(current_token_.type) << ")" << std::endl;

    if (current_token_.type == type)
    {
        current_token_ = lexer_.getNextToken();
    }
    else
    {
        reportError("Expected '" + tokenToString(type) + "' but found '" + current_token_.value + "'");
    }
}

std::string Parser::getSourceLine(int lineNumber) const {
    if (lineNumber <= 0) return "";
    
    std::string line;
    int currentLine = 1;
    size_t pos = 0;
    
    while (pos < source_.length()) {
        if (currentLine == lineNumber) {
            while (pos < source_.length() && source_[pos] != '\n') {
                line += source_[pos++];
            }
            return line;
        }
        if (source_[pos] == '\n') {
            currentLine++;
        }
        pos++;
    }
    return line;
}

void Parser::reportError(const std::string& message) {
    std::cerr << "\033[1;31mError\033[0m at line " << current_token_.line 
              << ", column " << current_token_.column << ": " << message << std::endl;
    
    // Get and print the erroneous line
    std::string sourceLine = getSourceLine(current_token_.line);
    std::string prevLine = getSourceLine(current_token_.line - 1);
    
    // If line is empty and we have a previous line, show the previous line
    if (sourceLine.empty() || current_token_.column == 0) {
        sourceLine = prevLine;
        std::cerr << prevLine << std::endl;
        // Point to the end of the previous line where semicolon should be
        for (size_t i = 0; i < prevLine.length(); i++) {
            std::cerr << " ";
        }
    } else {
        std::cerr << sourceLine << std::endl;
        // Print the error pointer at the correct column
        for (int i = 0; i < current_token_.column - 1; i++) {
            std::cerr << " ";
        }
    }
    std::cerr << "^\n";
}

std::string Parser::tokenToString(TokenType type)
{
    switch (type)
    {
    case MODULE:
        return "MODULE";
    case IDENTIFIER:
        return "IDENTIFIER";
    case LBRACE:
        return "LBRACE";
    case RBRACE:
        return "RBRACE";
    case IMPORT:
        return "IMPORT";
    case COMMENT:
        return "COMMENT";
    case LET:
        return "LET";
    case VAR:
        return "VAR";
    case FUNC:
        return "FUNC";
    case RETURN:
        return "RETURN";
    case COLON:
        return "COLON";
    case SEMICOLON:
        return "SEMICOLON";
    case LPAREN:
        return "LPAREN";
    case RPAREN:
        return "RPAREN";
    case COMMA:
        return "COMMA";
    case ARROW:
        return "ARROW";
    case NUMBER:
        return "NUMBER";
    case STRING:
        return "STRING";
    case BOOLEAN:
        return "BOOLEAN";
    case OPERATOR:
        return "OPERATOR";
    case DOT:
        return "DOT";
    default:
        return "UNKNOWN";
    }
}
