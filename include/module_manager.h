#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include "ast_node.h"

// Function type for module functions
using ModuleFunction = std::function<std::string(const std::vector<std::unique_ptr<ASTNode>> &)>;

class ModuleManager
{
public:
    static ModuleManager &getInstance();

    void registerModule(const std::string &moduleName);
    bool isModuleImported(const std::string &moduleName) const;

    // New methods for function management
    void registerFunction(const std::string &moduleName, const std::string &functionName, ModuleFunction func);
    bool hasFunction(const std::string &qualifiedName) const;
    std::string callFunction(const std::string &qualifiedName, const std::vector<std::unique_ptr<ASTNode>> &args);

    // Add error handling method
    std::string getLastError() const { return lastError; }

    // Add module function registration methods
    void registerUserDefinedFunction(const std::string &moduleName, const std::string &functionName,
                                     const std::vector<FunctionNode::Parameter> &params, const std::string &returnType,
                                     std::unique_ptr<ASTNode> body);

    std::unique_ptr<ASTNode> getUserDefinedFunction(const std::string &qualifiedName) const;

private:
    ModuleManager() = default;
    std::unordered_set<std::string> importedModules;
    std::unordered_map<std::string, std::unordered_map<std::string, ModuleFunction>> moduleFunctions;
    mutable std::string lastError;

    // Add storage for user-defined functions
    struct UserFunction
    {
        std::vector<FunctionNode::Parameter> parameters; // Updated type
        std::string returnType;
        std::unique_ptr<ASTNode> body;
    };

    std::unordered_map<std::string, std::unordered_map<std::string, UserFunction>> userDefinedFunctions;
};
