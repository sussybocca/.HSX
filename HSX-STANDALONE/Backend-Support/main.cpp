#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <filesystem>

#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Optimizer.h"
#include "HSXtoJS.h"
#include "Debugger.h"
#include "Core.h"
#include "Tokenizer.h"
#include "Compiler.h"
#include "VM.h"
#include "HSXBuild.h"

#pragma comment(lib, "ws2_32.lib")

namespace fs = std::filesystem;

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string content(size, '\0');
    file.read(&content[0], size);
    return content;
}

void serveBrowserApp(const std::string& appJs, int port) {
    std::string html = 
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
        "<title>HSX App</title><style>body{font-family:sans-serif;margin:20px;}"
        "button{margin:5px;padding:8px 16px;cursor:pointer;}"
        "input{margin:5px;padding:8px;}</style></head><body>"
        "<script>" + appJs + "</script>"
        "</body></html>";

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) return;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind to port " << port << "\n";
        closesocket(server);
        return;
    }
    listen(server, 1);
    
    std::cout << "HSX Browser App: http://localhost:" << port << "\n";
    std::cout << "Press Ctrl+C to stop the server.\n";
    
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " + std::to_string(html.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + html;

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;
        
        char buf[4096];
        recv(client, buf, sizeof(buf), 0);
        send(client, response.c_str(), (int)response.size(), 0);
        closesocket(client);
    }
    closesocket(server);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: hsx <file.hsx|projectDir> [--debug] [--optimize] [--fast] [--browser [port]] [--build] [--backend]\n";
        return 1;
    }

    std::string filename;
    bool debugMode = false;
    bool optimizeMode = false;
    bool fastMode = false;
    bool browserMode = false;
    bool buildMode = false;
    bool backendMode = false;
    int browserPort = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debugMode = true;
        else if (arg == "--optimize") optimizeMode = true;
        else if (arg == "--fast") fastMode = true;
        else if (arg == "--browser") {
            browserMode = true;
            if (i + 1 < argc && argv[i+1][0] != '-') {
                browserPort = std::stoi(argv[++i]);
            }
        }
        else if (arg == "--build") buildMode = true;
        else if (arg == "--backend") backendMode = true;
        else filename = arg;
    }

    if (backendMode) {
        std::string dir = fs::path(filename).parent_path().string();
        if (dir.empty()) dir = ".";
        std::string importsPath = dir + "/IMPORTS.hsx";
        std::vector<std::string> allLines;

        if (fs::exists(importsPath)) {
            std::ifstream importsFile(importsPath);
            if (importsFile) {
                std::string line;
                while (std::getline(importsFile, line)) {
                    if (line.find("IMPORT:") == 0) {
                        size_t start = line.find("((");
                        size_t end = line.find("))");
                        if (start != std::string::npos && end != std::string::npos) {
                            std::string module = line.substr(start + 2, end - start - 2);
                            std::string modulePath = dir + "/H.Source/_.HSX/H_APP/LIB/" + module + ".hsx";
                            if (fs::exists(modulePath)) {
                                std::ifstream modFile(modulePath);
                                if (modFile) {
                                    std::string modLine;
                                    while (std::getline(modFile, modLine)) {
                                        if (!modLine.empty()) allLines.push_back(modLine);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Cannot open: " << filename << "\n";
            return 1;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) allLines.push_back(line);
        }
        file.close();

        auto backendProgram = hsx::BackendParser::parse(allLines);
        hsx::Runtime::init();
        hsx::Interpreter interpreter;
        interpreter.executeServer(backendProgram);
        hsx::Runtime::cleanup();
        return 0;
    }

    if (buildMode) {
        std::string projectDir = filename;
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".hsx") {
            projectDir = fs::path(filename).parent_path().string();
        }
        if (projectDir.empty()) projectDir = ".";
        hsx::HSXBuild::build(projectDir, "dist");
        return 0;
    }

    if (fastMode) {
        std::string src = readFile(filename);
        if (src.empty()) {
            std::cerr << "Error: Cannot open file " << filename << "\n";
            return 1;
        }

        auto t1 = std::chrono::high_resolution_clock::now();

        hsx::Arena arena(1024 * 1024);
        hsx::CompiledUnit unit;
        hsx::Compiler compiler(src.c_str(), (uint32_t)src.size(), arena, unit);

        if (!compiler.compile()) {
            return 1;
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        auto compileUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        hsx::VM vm(unit);
        auto t3 = std::chrono::high_resolution_clock::now();
        vm.execute();
        auto t4 = std::chrono::high_resolution_clock::now();
        auto execUs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

        std::cerr << "[HSX Fast] Compiled " << unit.code.size() << " instrs in " 
                  << compileUs << "us | Executed in " << execUs << "us | Total: " 
                  << (compileUs + execUs) << "us\n";
        return 0;
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    file.close();

    try {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        std::vector<std::unique_ptr<hsx::ASTNode>> program;
        for (const auto& l : lines) {
            program.push_back(hsx::Parser::parseLine(l));
        }

        if (optimizeMode) {
            hsx::Optimizer::optimize(program);
        }

        if (browserMode) {
            std::string appJs = hsx::HSXtoJS::transpile(program);
            std::cout << "Opening HSX app in browser...\n";
            ShellExecuteA(NULL, "open", 
                ("http://localhost:" + std::to_string(browserPort)).c_str(),
                NULL, NULL, SW_SHOW);
            serveBrowserApp(appJs, browserPort);
        } else if (debugMode) {
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
            hsx::Runtime::init();
            hsx::Interpreter interpreter;
            interpreter.execute(std::move(program));
            hsx::Runtime::cleanup();
        }

        WSACleanup();
    } catch (const std::exception& e) {
        std::cerr << "HSX Error: " << e.what() << "\n";
        WSACleanup();
        return 1;
    }

    return 0;
}