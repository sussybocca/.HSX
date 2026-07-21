#ifndef HSX_DEBUGGER_H
#define HSX_DEBUGGER_H

#include "Parser.h"
#include "Runtime.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <variant>

namespace hsx {

class Debugger {
public:
    Debugger(std::vector<ASTPtr> prog,
             std::unordered_map<std::string, std::vector<ASTPtr>> funcs)
        : program(std::move(prog)), functions(std::move(funcs)), pc(0), stepMode(true) {}

    void run() {
        std::string command;
        while (pc < program.size()) {
            if (stepMode || breakpoints.count(pc)) {
                std::cout << "[DBG] Line " << pc << " -> ";
                printNodeBrief(program[pc]);
                std::cout << "> ";
                std::getline(std::cin, command);
                if (command == "s" || command == "step") {
                    stepMode = true;
                } else if (command == "c" || command == "continue") {
                    stepMode = false;
                } else if (command.rfind("b ", 0) == 0) {
                    int line = std::stoi(command.substr(2));
                    breakpoints.insert(line);
                    std::cout << "Breakpoint set at line " << line << std::endl;
                    continue;
                } else if (command == "vars") {
                    printVariables();
                    continue;
                } else if (command == "q") {
                    break;
                }
            }
            executeCurrentLine();
            if (!stepMode && !breakpoints.count(pc)) {
                while (pc < program.size() && !breakpoints.count(pc)) {
                    executeCurrentLine();
                }
                if (pc >= program.size()) break;
                stepMode = true;
            } else {
                pc++;
            }
        }
        std::cout << "[DBG] Program finished." << std::endl;
    }

private:
    std::vector<ASTPtr> program;
    std::unordered_map<std::string, std::vector<ASTPtr>> functions;
    size_t pc;
    bool stepMode;
    std::unordered_set<int> breakpoints;
    std::unordered_map<std::string, std::variant<double, std::string, std::vector<double>>> variables;

    void printVariant(const std::variant<double, std::string, std::vector<double>>& v) {
        if (std::holds_alternative<double>(v))
            std::cout << std::get<double>(v);
        else if (std::holds_alternative<std::string>(v))
            std::cout << std::get<std::string>(v);
        else if (std::holds_alternative<std::vector<double>>(v)) {
            auto& vec = std::get<std::vector<double>>(v);
            std::cout << "[";
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << vec[i];
            }
            std::cout << "]";
        }
    }

    void printVariantLine(const std::variant<double, std::string, std::vector<double>>& v) {
        printVariant(v);
        std::cout << std::endl;
    }

    void executeCurrentLine() {
        if (pc >= program.size()) return;
        auto& node = program[pc];
        try {
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
                if (pc + 1 >= program.size()) throw std::runtime_error("Loop without body");
                int count = l->count;
                for (int i = 0; i < count; ++i) {
                    executeSingleNode(program[pc+1]);
                }
                pc++;
            }
            else if (auto* w = dynamic_cast<WhileLoopNode*>(node.get())) {
                if (w->varName == "__DEAD__") {
                    pc++;
                    return;
                }
                if (pc + 1 >= program.size()) throw std::runtime_error("While without body");
                while (evaluateCondition(w->varName, w->compareValue)) {
                    executeSingleNode(program[pc+1]);
                }
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
            else if (auto* ret = dynamic_cast<ReturnNode*>(node.get())) {
                if (variables.find(ret->varName) != variables.end()) {
                    printVariantLine(variables[ret->varName]);
                }
            }
        } catch (std::exception& e) {
            std::cerr << "[DBG] Runtime error: " << e.what() << std::endl;
        }
    }

    void executeSingleNode(const ASTPtr& node) {
        if (auto* p = dynamic_cast<PrintNode*>(node.get())) {
            if (p->value == "__INPUT__") {
                std::string input;
                std::getline(std::cin, input);
                std::cout << input << std::endl;
            } else std::cout << p->value << std::endl;
        }
        else if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
            variables[a->name] = a->value;
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
    }

    bool evaluateCondition(const std::string& var, double val) {
        if (variables.find(var) == variables.end()) return false;
        auto& stored = variables[var];
        if (std::holds_alternative<double>(stored))
            return std::get<double>(stored) == val;
        return false;
    }

    void printVariables() {
        for (auto& [name, val] : variables) {
            std::cout << name << " = ";
            printVariant(val);
            std::cout << std::endl;
        }
    }

    void printNodeBrief(const ASTPtr& node) {
        if (dynamic_cast<PrintNode*>(node.get())) std::cout << "Print";
        else if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) std::cout << "Assign " << a->name;
        else if (dynamic_cast<LoopNode*>(node.get())) std::cout << "Loop";
        else if (auto* w = dynamic_cast<WhileLoopNode*>(node.get())) std::cout << "While " << w->varName;
        else if (dynamic_cast<IfElsePrintNode*>(node.get())) std::cout << "IfElse";
        else if (auto* fe = dynamic_cast<FrontendNode*>(node.get())) std::cout << "Frontend " << fe->tag;
        else if (auto* be = dynamic_cast<BackendNode*>(node.get())) std::cout << "Backend " << be->command;
        else if (dynamic_cast<FunctionDefNode*>(node.get())) std::cout << "FuncDef";
        else if (dynamic_cast<ReturnNode*>(node.get())) std::cout << "Return";
        else if (dynamic_cast<CommentNode*>(node.get())) std::cout << "Comment";
        else std::cout << "Other";
        std::cout << std::endl;
    }
};

} // namespace hsx
#endif