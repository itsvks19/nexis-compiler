#include "module_manager.h"
#include "symbol_table.h"
#include "evaluator.h"

#include <algorithm>

ModuleManager& ModuleManager::getInstance() {
    static ModuleManager instance;
    return instance;
}

void ModuleManager::registerModule(const std::string& moduleName) {
    importedModules.insert(moduleName);
    
    // Also register the last part of the module path
    size_t lastDot = moduleName.find_last_of('.');
    if (lastDot != std::string::npos) {
        std::string shortName = moduleName.substr(lastDot + 1);
        importedModules.insert(shortName);
    }
}

bool ModuleManager::isModuleImported(const std::string& moduleName) const {
    return importedModules.find(moduleName) != importedModules.end();
}

void ModuleManager::registerFunction(const std::string& moduleName, const std::string& functionName, ModuleFunction func) {
    moduleFunctions[moduleName][functionName] = func;
}

void ModuleManager::registerUserDefinedFunction(const std::string& moduleName, 
                                              const std::string& functionName,
                                              const std::vector<FunctionNode::Parameter>& params,
                                              const std::string& returnType,
                                              std::unique_ptr<ASTNode> body) {
    UserFunction func;
    func.parameters = params;
    func.returnType = returnType;
    func.body = std::move(body);
    userDefinedFunctions[moduleName][functionName] = std::move(func);
}

std::unique_ptr<ASTNode> ModuleManager::getUserDefinedFunction(const std::string& qualifiedName) const {
    size_t dotPos = qualifiedName.find('.');
    if (dotPos == std::string::npos) return nullptr;

    std::string moduleName = qualifiedName.substr(0, dotPos);
    std::string functionName = qualifiedName.substr(dotPos + 1);

    auto moduleIt = userDefinedFunctions.find(moduleName);
    if (moduleIt != userDefinedFunctions.end()) {
        auto funcIt = moduleIt->second.find(functionName);
        if (funcIt != moduleIt->second.end()) {
            return funcIt->second.body->clone();
        }
    }

    return nullptr;
}

bool ModuleManager::hasFunction(const std::string& qualifiedName) const {
    size_t dotPos = qualifiedName.find_last_of('.');
    if (dotPos == std::string::npos) return false;

    std::string moduleName = qualifiedName.substr(0, dotPos);
    std::string functionName = qualifiedName.substr(dotPos + 1);

    // First check built-in functions
    auto moduleIt = moduleFunctions.find("std." + moduleName);
    if (moduleIt != moduleFunctions.end()) {
        if (moduleIt->second.find(functionName) != moduleIt->second.end()) {
            return true;
        }
    }
    
    moduleIt = moduleFunctions.find(moduleName);
    if (moduleIt != moduleFunctions.end()) {
        if (moduleIt->second.find(functionName) != moduleIt->second.end()) {
            return true;
        }
    }

    // Then check user-defined functions
    auto userModuleIt = userDefinedFunctions.find(moduleName);
    if (userModuleIt != userDefinedFunctions.end()) {
        return userModuleIt->second.find(functionName) != userModuleIt->second.end();
    }

    return false;
}

std::string ModuleManager::callFunction(const std::string& qualifiedName, const std::vector<std::unique_ptr<ASTNode>>& args) {
    size_t dotPos = qualifiedName.find_last_of('.');
    if (dotPos == std::string::npos) return "";

    std::string moduleName = qualifiedName.substr(0, dotPos);
    std::string functionName = qualifiedName.substr(dotPos + 1);

    // First try built-in functions
    auto moduleIt = moduleFunctions.find("std." + moduleName);
    if (moduleIt != moduleFunctions.end()) {
        auto funcIt = moduleIt->second.find(functionName);
        if (funcIt != moduleIt->second.end()) {
            return funcIt->second(args);
        }
    }

    moduleIt = moduleFunctions.find(moduleName);
    if (moduleIt != moduleFunctions.end()) {
        auto funcIt = moduleIt->second.find(functionName);
        if (funcIt != moduleIt->second.end()) {
            return funcIt->second(args);
        }
    }

    // Then try user-defined functions
    auto userModuleIt = userDefinedFunctions.find(moduleName);
    if (userModuleIt != userDefinedFunctions.end()) {
        auto funcIt = userModuleIt->second.find(functionName);
        if (funcIt != userModuleIt->second.end()) {
            const UserFunction& func = funcIt->second;
            
            // Create new symbol scope for function call
            SymbolTable::getInstance().pushScope();
            
            // Bind arguments to parameters
            for (size_t i = 0; i < func.parameters.size() && i < args.size(); i++) {
                std::string argValue = evaluateNode(args[i].get());
                SymbolTable::getInstance().setValue(func.parameters[i].name, argValue);
            }
            
            // Execute function body
            std::string result;
            for (const auto& stmt : dynamic_cast<FunctionNode*>(func.body.get())->body) {
                result = evaluateNode(stmt.get());
                // If this is a return statement, break out
                if (!SymbolTable::getInstance().getValue("_lastResult").empty()) {
                    result = SymbolTable::getInstance().getValue("_lastResult");
                    SymbolTable::getInstance().setValue("_lastResult", "");
                    break;
                }
            }
            
            // Restore previous scope
            SymbolTable::getInstance().popScope();
            return result;
        }
    }

    return "";
}
