#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable {
public:
    static SymbolTable& getInstance() {
        static SymbolTable instance;
        return instance;
    }

    void pushScope() {
        scopes.push_back({});
    }

    void popScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
        }
    }

    void setValue(const std::string& name, const std::string& value) {
        if (!scopes.empty()) {
            scopes.back()[name] = value;
        } else {
            variables[name] = value;
        }
    }

    std::string getValue(const std::string& name) const {
        // Search from innermost scope to outermost
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        auto it = variables.find(name);
        return it != variables.end() ? it->second : "";
    }

private:
    SymbolTable() = default;
    std::unordered_map<std::string, std::string> variables;
    std::vector<std::unordered_map<std::string, std::string>> scopes;
};
