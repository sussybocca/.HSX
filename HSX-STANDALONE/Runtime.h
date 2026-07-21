#ifndef HSX_RUNTIME_H
#define HSX_RUNTIME_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

namespace hsx {

struct BackendNode;

class Runtime {
public:
    static void init() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    static void cleanup() {
        WSACleanup();
    }

    static std::string readFile(const std::string& path) {
        std::ifstream f(path);
        if (!f) throw std::runtime_error("Cannot open file: " + path);
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static void writeFile(const std::string& path, const std::string& content) {
        std::ofstream f(path);
        if (!f) throw std::runtime_error("Cannot write file: " + path);
        f << content;
    }

    static void appendFile(const std::string& path, const std::string& content) {
        std::ofstream f(path, std::ios::app);
        if (!f) throw std::runtime_error("Cannot append to file: " + path);
        f << content;
    }

    static std::string httpGet(const std::string& url) {
        std::string host, path = "/";
        size_t proto_end = url.find("://");
        size_t host_start = (proto_end != std::string::npos) ? proto_end + 3 : 0;
        size_t path_start = url.find('/', host_start);
        if (path_start != std::string::npos) {
            host = url.substr(host_start, path_start - host_start);
            path = url.substr(path_start);
        } else {
            host = url.substr(host_start);
        }

        int port = 80;
        size_t colon = host.find(':');
        if (colon != std::string::npos) {
            port = std::stoi(host.substr(colon+1));
            host = host.substr(0, colon);
        }

        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
            throw std::runtime_error("Host resolution failed: " + host);
        }

        SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == INVALID_SOCKET) {
            freeaddrinfo(res);
            throw std::runtime_error("Socket creation failed");
        }

        if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock);
            freeaddrinfo(res);
            throw std::runtime_error("Connection failed");
        }
        freeaddrinfo(res);

        std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
        send(sock, request.c_str(), (int)request.size(), 0);

        std::string response;
        char buffer[4096];
        int bytes;
        while ((bytes = recv(sock, buffer, sizeof(buffer)-1, 0)) > 0) {
            buffer[bytes] = '\0';
            response += buffer;
        }
        closesocket(sock);

        size_t header_end = response.find("\r\n\r\n");
        if (header_end != std::string::npos)
            return response.substr(header_end + 4);
        return response;
    }

    static void dbSet(const std::string& dbName, const std::string& key, const std::string& value) {
        std::unordered_map<std::string, std::string> store = loadDB(dbName);
        store[key] = value;
        saveDB(dbName, store);
    }

    static std::string dbGet(const std::string& dbName, const std::string& key) {
        auto store = loadDB(dbName);
        if (store.count(key)) return store[key];
        throw std::runtime_error("Key not found: " + key);
    }

    static void startServer(int port) {
        std::thread([port]() {
            SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == INVALID_SOCKET) return;

            int opt = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);
            if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
                closesocket(server_fd);
                return;
            }
            listen(server_fd, 5);
            std::cout << "[Server] Listening on port " << port << std::endl;

            while (true) {
                sockaddr_in client;
                int len = sizeof(client);
                SOCKET client_fd = accept(server_fd, (sockaddr*)&client, &len);
                if (client_fd == INVALID_SOCKET) continue;
                const char* msg = "HSX Server Active\n";
                send(client_fd, msg, (int)strlen(msg), 0);
                closesocket(client_fd);
            }
        }).detach();
    }

    static void renderFrontend(const std::string& tag, const std::string& content) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (tag == "+T_X_T+") {
            SetConsoleTextAttribute(hConsole, 11);
            std::cout << content << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+B_A_I_T+") {
            SetConsoleTextAttribute(hConsole, 14);
            std::cout << "[ BUTTON: " << content << " ]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
            std::cout << "Press ENTER to click...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "Button clicked!" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+I_M_G+") {
            std::cout << "[Image: " << content << "]" << std::endl;
        }
        else if (tag == "+L_N_K+") {
            SetConsoleTextAttribute(hConsole, 11);
            std::cout << content << " (link)" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+L_S_T+") {
            std::cout << "List: " << content << std::endl;
        }
        else if (tag == "+D_I_V+") {
            SetConsoleTextAttribute(hConsole, 15);
            std::cout << "[DIV Container]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+/D_I_V+") {
            SetConsoleTextAttribute(hConsole, 15);
            std::cout << "[/DIV Container]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+F_R_M+") {
            SetConsoleTextAttribute(hConsole, 13);
            std::cout << "[Form Start]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+/F_R_M+") {
            SetConsoleTextAttribute(hConsole, 13);
            std::cout << "[Form End]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+T_B_L+") {
            SetConsoleTextAttribute(hConsole, 11);
            std::cout << "[Table Start]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else if (tag == "+/T_B_L+") {
            SetConsoleTextAttribute(hConsole, 11);
            std::cout << "[Table End]" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        else {
            std::cout << "[Frontend] " << tag << " : " << content << std::endl;
        }
    }

    static void handleBackend(const BackendNode& node) {
        std::string cmd = node.command;
        if (cmd == "{I_N_U_T}") {
            std::cout << node.params;
            std::string input;
            std::getline(std::cin, input);
            std::cout << "[Input received: " << input << "]" << std::endl;
        }
        else if (cmd == "{F_I_L}") {
            if (!node.params.empty()) {
                std::cout << readFile(node.params) << std::endl;
            }
        }
        else if (cmd == "{A_P_I}") {
            if (!node.params.empty()) {
                std::string result = httpGet(node.params);
                std::cout << result << std::endl;
            }
        }
        else if (cmd == "{D_B_C}") {
            std::cout << "[Database connected: " << node.params << "]" << std::endl;
        }
        else if (cmd == "{S_R_V}") {
            int port = node.hasNumber ? static_cast<int>(node.numberVal) : 8080;
            startServer(port);
        }
        else if (cmd == "{J_S_N}") {
            std::cout << "[JSON parsed]" << std::endl;
        }
        else if (cmd == "{Q_R_Y}") {
            std::cout << "[Query executed: " << node.params << "]" << std::endl;
        }
        else if (cmd == "{S_C_K}") {
            std::cout << "[Socket opened]" << std::endl;
        }
        else {
            std::cout << "[Backend] " << cmd;
            if (!node.params.empty()) std::cout << " " << node.params;
            if (node.hasNumber) std::cout << " " << node.numberVal;
            std::cout << std::endl;
        }
    }

private:
    static std::unordered_map<std::string, std::string> loadDB(const std::string& name) {
        std::unordered_map<std::string, std::string> map;
        std::ifstream f(name + ".hsxdb");
        if (!f) return map;
        std::string line;
        while (std::getline(f, line)) {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string k = line.substr(0, eq);
                std::string v = line.substr(eq+1);
                map[k] = v;
            }
        }
        return map;
    }

    static void saveDB(const std::string& name, const std::unordered_map<std::string, std::string>& map) {
        std::ofstream f(name + ".hsxdb");
        for (auto& p : map)
            f << p.first << "=" << p.second << "\n";
    }
};

} // namespace hsx
#endif