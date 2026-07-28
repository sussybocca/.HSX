#include "Lexer.h"
#include "Parser.h"
#include "HSXtoJS.h"
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>

extern "C" {

char* compileHSX(const char* source) {
    std::vector<std::unique_ptr<hsx::ASTNode>> program;
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            try {
                program.push_back(hsx::Parser::parseLine(line));
            } catch (...) {}
        }
    }
    
    std::string result = hsx::HSXtoJS::transpile(program);
    
    char* out = (char*)malloc(result.size() + 1);
    std::memcpy(out, result.c_str(), result.size() + 1);
    return out;
}

void freeHSX(char* ptr) {
    free(ptr);
}

}