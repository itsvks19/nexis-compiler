#include "evaluator.h"
#include "symbol_table.h"
#include "module_manager.h"
#include <iostream>

std::string evaluateNode(ASTNode *node)
{
    if (!node)
        return "";

    if (auto literalNode = dynamic_cast<LiteralNode *>(node))
    {
        if (literalNode->type == "identifier")
        {
            std::string value = SymbolTable::getInstance().getValue(literalNode->value);
            return value.empty() ? literalNode->value : value;
        }
        if (literalNode->type == "string") {
            std::string value = literalNode->value;
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                return value.substr(1, value.length() - 2);
            }
        }
        return literalNode->value;
    }
    else if (auto binaryOpNode = dynamic_cast<BinaryOperationNode *>(node))
    {
        std::string left = evaluateNode(binaryOpNode->left.get());
        std::string right = evaluateNode(binaryOpNode->right.get());

        if (binaryOpNode->op == "+")
        {
            try {
                return std::to_string(std::stoi(left) + std::stoi(right));
            } catch (...) {
                return left + right;
            }
        }
        else if (binaryOpNode->op == "*")
        {
            try {
                return std::to_string(std::stoi(left) * std::stoi(right));
            } catch (...) {
                return "0";
            }
        }
    }
    else if (auto functionCallNode = dynamic_cast<FunctionCallNode *>(node))
    {
        auto& mm = ModuleManager::getInstance();
        return mm.callFunction(functionCallNode->name, functionCallNode->arguments);
    }
    else if (auto ifNode = dynamic_cast<IfStatementNode*>(node))
    {
        std::string condValue = evaluateNode(ifNode->condition.get());
        
        // Convert condition to boolean
        bool condBool = false;
        if (condValue == "true") {
            condBool = true;
        }
        else if (condValue == "false") {
            condBool = false;
        }
        else {
            try {
                condBool = std::stoi(condValue) != 0;
            } catch (...) {
                condBool = !condValue.empty();
            }
        }
        
        std::string result;
        if (condBool) {
            for (const auto& stmt : ifNode->thenBranch) {
                result = evaluateNode(stmt.get());
            }
        } else if (!ifNode->elseBranch.empty()) {
            for (const auto& stmt : ifNode->elseBranch) {
                result = evaluateNode(stmt.get());
            }
        }
        return result;
    }
    return "";
}
