#pragma once

#include <string>
#include <vector>
#include <memory>

class ASTNode
{
public:
    virtual ~ASTNode() = default;
    virtual std::unique_ptr<ASTNode> clone() const = 0;
};

class ModuleNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<ModuleNode>();
        node->name = name;
        for (const auto& child : body) {
            if (child) {
                node->body.push_back(child->clone());
            }
        }
        return node;
    }
};

class FunctionNode : public ASTNode {
public:
    std::string name;
    struct Parameter {
        std::string name;
        std::string type;
    };
    std::vector<Parameter> parameters;  // Changed from vector<string> to vector<Parameter>
    std::string returnType;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<FunctionNode>();
        node->name = name;
        node->parameters = parameters;
        node->returnType = returnType;
        for (const auto& child : body) {
            if (child) {
                node->body.push_back(child->clone());
            }
        }
        return node;
    }
};

class VariableDeclarationNode : public ASTNode {
public:
    std::string name;
    std::string type;
    std::unique_ptr<ASTNode> initializer;
    bool isMutable;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<VariableDeclarationNode>();
        node->name = name;
        node->type = type;
        node->isMutable = isMutable;
        if (initializer) {
            node->initializer = initializer->clone();
        }
        return node;
    }
};

class BinaryOperationNode : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<BinaryOperationNode>();
        node->op = op;
        if (left) node->left = left->clone();
        if (right) node->right = right->clone();
        return node;
    }
};

class LiteralNode : public ASTNode {
public:
    std::string value;
    std::string type; // "int", "string", "boolean"
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<LiteralNode>();
        node->value = value;
        node->type = type;
        return node;
    }
};

class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<FunctionCallNode>();
        node->name = name;
        for (const auto& arg : arguments) {
            if (arg) {
                node->arguments.push_back(arg->clone());
            }
        }
        return node;
    }
};

class ReturnStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<ReturnStatementNode>();
        if (expression) {
            node->expression = expression->clone();
        }
        return node;
    }
};

class IfStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBranch;
    std::vector<std::unique_ptr<ASTNode>> elseBranch;
    
    std::unique_ptr<ASTNode> clone() const override {
        auto node = std::make_unique<IfStatementNode>();
        if (condition) {
            node->condition = condition->clone();
        }
        for (const auto& stmt : thenBranch) {
            if (stmt) {
                node->thenBranch.push_back(stmt->clone());
            }
        }
        for (const auto& stmt : elseBranch) {
            if (stmt) {
                node->elseBranch.push_back(stmt->clone());
            }
        }
        return node;
    }
};
