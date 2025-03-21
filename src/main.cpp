#include "lexer.h"
#include "parser.h"
#include "instance.h"
#include "module_manager.h"
#include "symbol_table.h"
#include "evaluator.h"
#include <iostream>
#include <vector>
#include <string>

void traverse(ASTNode *node, bool registerOnly = true)
{
    if (!node)
        return;

    if (auto moduleNode = dynamic_cast<ModuleNode *>(node))
    {
        // If this is the root program node, traverse all modules
        for (const auto &child : moduleNode->body)
        {
            if (auto childModule = dynamic_cast<ModuleNode*>(child.get())) {
                // Register the module before traversing its contents
                ModuleManager::getInstance().registerModule(childModule->name);
            }
            traverse(child.get(), registerOnly);
        }
    }
    else if (auto functionNode = dynamic_cast<FunctionNode *>(node))
    {
        // Only traverse function body if not in register-only mode
        if (registerOnly) {
            for (const auto &child : functionNode->body)
            {
                traverse(child.get(), registerOnly);
            }
        }
    }
    else if (auto varDeclNode = dynamic_cast<VariableDeclarationNode *>(node))
    {
        if (registerOnly && varDeclNode->initializer)
        {
            std::string value = evaluateNode(varDeclNode->initializer.get());
            SymbolTable::getInstance().setValue(varDeclNode->name, value);
        }
    }
    else if (auto functionCallNode = dynamic_cast<FunctionCallNode *>(node))
    {
        if (!registerOnly) {
            auto& mm = ModuleManager::getInstance();
            if (mm.hasFunction(functionCallNode->name)) {
                std::string result = mm.callFunction(functionCallNode->name, functionCallNode->arguments);
                if (!result.empty()) {
                    SymbolTable::getInstance().setValue("_lastResult", result);
                }
            } else {
                std::cerr << "Error: Undefined function '" << functionCallNode->name << "'" << std::endl;
            }
        }
    }
    else if (auto binaryOpNode = dynamic_cast<BinaryOperationNode *>(node))
    {
        if (!registerOnly) {
            traverse(binaryOpNode->left.get(), registerOnly);
            traverse(binaryOpNode->right.get(), registerOnly);
        }
    }
    else if (auto ifNode = dynamic_cast<IfStatementNode*>(node))
    {
        if (!registerOnly) {
            std::string result = evaluateNode(node);
            if (!result.empty()) {
                SymbolTable::getInstance().setValue("_lastResult", result);
            }
        }
    }
}

void registerStandardModules() {
    auto& mm = ModuleManager::getInstance();

    // Register IO functions using full module path
    mm.registerFunction("std.io", "print", [](const std::vector<std::unique_ptr<ASTNode>>& args) {
        for (const auto& arg : args) {
            std::cout << evaluateNode(arg.get());
        }
        std::cout << std::endl;
        return "";
    });

    mm.registerFunction("std.io", "println", [](const std::vector<std::unique_ptr<ASTNode>>& args) {
        std::string result;
        for (const auto& arg : args) {
            result += evaluateNode(arg.get());
        }
        std::cout << result << std::endl;
        return result;
    });

    mm.registerFunction("std.math", "add", [](const std::vector<std::unique_ptr<ASTNode>>& args) {
        if (args.size() != 2) return std::string("0");
        int a = std::stoi(evaluateNode(args[0].get()));
        int b = std::stoi(evaluateNode(args[1].get()));
        return std::to_string(a + b);
    });
    
    mm.registerFunction("std.math", "subtract", [](const std::vector<std::unique_ptr<ASTNode>>& args) {
        if (args.size() != 2) return std::string("0");
        int a = std::stoi(evaluateNode(args[0].get()));
        int b = std::stoi(evaluateNode(args[1].get()));
        return std::to_string(a - b);
    });
}

int main()
{
    registerStandardModules();

    std::string sourceCode = R"(
module Math {
    import std.io;

    func square(x: int) -> int {
        return x * x;
    }

    func hello() -> int {
        io.println("Hello from Math module!");
        return 0;
    }
}

module Main {
    import Math;
    import std.io;

    func main() -> int {
        let num = 8;
        let sq = Math.square(num);

        let hh: boolean = true;
        let hh2 = false;

        if (hh2) {
            io.println("True");
        } else {
            io.println("False");
        }

        Math.hello();
        io.print("Square is: " + sq);
        return 0;
    }
}
)";

    Lexer lexer(sourceCode);
    Parser parser(lexer, sourceCode);

    auto ast = parser.parse();
    if (!ast) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    } else {
        // std::cout << "Program parsed successfully" << std::endl;
    }

    traverse(ast.get());

    auto& mm = ModuleManager::getInstance();
    if (mm.hasFunction("Main.main")) {
        mm.callFunction("Main.main", {});
    } else {
        std::cerr << "Error: Main function not found" << std::endl;
        return 1;
    }

    return 0;
}
