#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Optimizer.h"
#include "Debugger.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: hsx <file.hsx> [--debug] [--optimize]\n";
        return 1;
    }

    std::string filename;
    bool debugMode = false;
    bool optimizeMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debugMode = true;
        else if (arg == "--optimize") optimizeMode = true;
        else filename = arg;
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            lines.push_back(line);
    }

    try {
        hsx::Runtime::init();

        std::vector<std::unique_ptr<hsx::ASTNode>> program;
        for (const auto& l : lines) {
            program.push_back(hsx::Parser::parseLine(l));
        }

        if (optimizeMode) {
            hsx::Optimizer::optimize(program);
        }

        if (debugMode) {
            std::unordered_map<std::string, std::vector<std::unique_ptr<hsx::ASTNode>>> funcs;
            std::vector<std::unique_ptr<hsx::ASTNode>> mainProg;
            bool inFunc = false;
            std::string currFunc;
            std::vector<std::unique_ptr<hsx::ASTNode>> currBody;
            for (auto& node : program) {
                if (auto* fd = dynamic_cast<hsx::FunctionDefNode*>(node.get())) {
                    if (inFunc) funcs[currFunc] = std::move(currBody);
                    inFunc = true;
                    currFunc = fd->name;
                    currBody.clear();
                } else if (inFunc) {
                    if (dynamic_cast<hsx::ReturnNode*>(node.get())) {
                        currBody.push_back(std::move(node));
                        funcs[currFunc] = std::move(currBody);
                        inFunc = false;
                    } else {
                        currBody.push_back(std::move(node));
                    }
                } else {
                    mainProg.push_back(std::move(node));
                }
            }
            if (inFunc) funcs[currFunc] = std::move(currBody);
            hsx::Debugger debugger(std::move(mainProg), std::move(funcs));
            debugger.run();
        } else {
            hsx::Interpreter interpreter;
            interpreter.execute(std::move(program));
        }

        hsx::Runtime::cleanup();
    } catch (const std::exception& e) {
        std::cerr << "HSX Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}