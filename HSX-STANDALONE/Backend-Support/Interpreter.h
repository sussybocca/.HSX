#ifndef HSX_INTERPRETER_H
#define HSX_INTERPRETER_H

#include "Parser.h"
#include "Runtime.h"
#include "StandardLibrary.h"
#include "BackendParser.h"
#include <iostream>
#include <unordered_map>
#include <functional>
#include <deque>

namespace hsx {

class Interpreter {
public:
    void execute(std::vector<ASTPtr> program) {
        std::unordered_map<std::string, std::vector<ASTPtr>> functions;
        std::vector<ASTPtr> mainProgram;
        bool insideFunction = false;
        std::string currentFuncName;
        std::vector<ASTPtr> currentFuncBody;

        for (auto& node : program) {
            if (auto* fdef = dynamic_cast<FunctionDefNode*>(node.get())) {
                if (insideFunction) {
                    functions[currentFuncName] = std::move(currentFuncBody);
                }
                insideFunction = true;
                currentFuncName = fdef->name;
                currentFuncBody.clear();
            } else if (insideFunction) {
                if (dynamic_cast<ReturnNode*>(node.get())) {
                    currentFuncBody.push_back(std::move(node));
                    functions[currentFuncName] = std::move(currentFuncBody);
                    insideFunction = false;
                } else {
                    currentFuncBody.push_back(std::move(node));
                }
            } else {
                mainProgram.push_back(std::move(node));
            }
        }
        if (insideFunction) {
            functions[currentFuncName] = std::move(currentFuncBody);
        }

        executeBlock(mainProgram, functions);
    }

    void executeServer(const BackendProgram& program) {
        int port = 8080;
        std::unordered_map<std::string, std::vector<BackendStmt>> routes;
        std::unordered_map<std::string, std::string> cache;
        std::unordered_map<std::string, std::vector<BackendStmt>> errorHandlers = program.errorHandlers;
        double vars[256] = {};

        for (const auto& stmt : program.statements) {
            if (stmt.cmd == BackendCmd::SRV && !stmt.numberArgs.empty()) {
                port = (int)stmt.numberArgs[0];
            }
        }

        Runtime::startBackendServer(port, program.statements, cache, errorHandlers, vars);
    }

private:
    std::unordered_map<std::string, std::variant<double, std::string, std::vector<double>>> variables;
    std::unordered_map<std::string, std::vector<ASTPtr>>* funcs = nullptr;

    void printVariant(const std::variant<double, std::string, std::vector<double>>& v) {
        if (std::holds_alternative<double>(v))
            std::cout << std::get<double>(v) << std::endl;
        else if (std::holds_alternative<std::string>(v))
            std::cout << std::get<std::string>(v) << std::endl;
        else if (std::holds_alternative<std::vector<double>>(v)) {
            auto& vec = std::get<std::vector<double>>(v);
            std::cout << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << vec[i];
            }
            std::cout << "]" << std::endl;
        }
    }

    void executeBlock(std::vector<ASTPtr>& block,
                      std::unordered_map<std::string, std::vector<ASTPtr>>& functions) {
        funcs = &functions;
        for (size_t pc = 0; pc < block.size(); ++pc) {
            auto& node = block[pc];
            if (auto* p = dynamic_cast<PrintNode*>(node.get())) {
                if (p->value == "__INPUT__") {
                    std::string input;
                    std::getline(std::cin, input);
                    std::cout << input << std::endl;
                } else {
                    std::cout << p->value << std::endl;
                }
            }
            else if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
                variables[a->name] = a->value;
            }
            else if (auto* l = dynamic_cast<LoopNode*>(node.get())) {
                if (pc + 1 >= block.size())
                    throw std::runtime_error("Loop without body");
                auto body = std::vector<ASTPtr>();
                body.push_back(std::move(block[pc+1]));
                for (int i = 0; i < l->count; ++i) {
                    executeBlock(body, functions);
                }
                block[pc+1] = std::move(body[0]);
                pc++;
            }
            else if (auto* w = dynamic_cast<WhileLoopNode*>(node.get())) {
                if (pc + 1 >= block.size())
                    throw std::runtime_error("While loop without body");
                auto body = std::vector<ASTPtr>();
                body.push_back(std::move(block[pc+1]));
                while (evaluateCondition(w->varName, w->compareValue)) {
                    executeBlock(body, functions);
                }
                block[pc+1] = std::move(body[0]);
                pc++;
            }
            else if (auto* ie = dynamic_cast<IfElsePrintNode*>(node.get())) {
                bool cond = evaluateCondition(ie->condVar, ie->condValue);
                std::cout << (cond ? ie->ifBranch : ie->elseBranch) << std::endl;
            }
            else if (auto* fe = dynamic_cast<FrontendNode*>(node.get())) {
                Runtime::renderFrontend(fe->tag, fe->content);
            }
            else if (auto* be = dynamic_cast<BackendNode*>(node.get())) {
                Runtime::handleBackend(*be);
            }
            else if (auto* fc = dynamic_cast<FunctionCallNode*>(node.get())) {
                if (functions.find(fc->name) == functions.end())
                    throw std::runtime_error("Function not defined: " + fc->name);
                auto& fbody = functions[fc->name];
                executeBlock(fbody, functions);
            }
            else if (auto* ret = dynamic_cast<ReturnNode*>(node.get())) {
                if (variables.find(ret->varName) != variables.end()) {
                    printVariant(variables[ret->varName]);
                } else {
                    std::cout << "undefined" << std::endl;
                }
            }
        }
    }

    bool evaluateCondition(const std::string& var, double val) {
        if (variables.find(var) == variables.end()) return false;
        auto& stored = variables[var];
        if (std::holds_alternative<double>(stored)) {
            return std::get<double>(stored) == val;
        }
        return false;
    }
};

} // namespace hsx
#endif