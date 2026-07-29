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
#include "BackendAST.h"

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

    static void startBackendServer(int port,
                                    const std::vector<BackendStmt>& statements,
                                    std::unordered_map<std::string, std::string>& cache,
                                    std::unordered_map<std::string, std::vector<BackendStmt>>& errorHandlers,
                                    double* vars) {
        SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
        if (server == INVALID_SOCKET) {
            std::cerr << "Socket creation failed\n";
            return;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed on port " << port << "\n";
            closesocket(server);
            return;
        }

        listen(server, 10);
        std::cout << "HSX Backend Server running on http://localhost:" << port << "\n";

        while (true) {
            SOCKET client = accept(server, nullptr, nullptr);
            if (client == INVALID_SOCKET) continue;

            char buf[8192] = {};
            int bytes = recv(client, buf, sizeof(buf) - 1, 0);
            if (bytes <= 0) {
                closesocket(client);
                continue;
            }
            buf[bytes] = '\0';
            std::string req(buf);

            std::string method = req.substr(0, req.find(' '));
            std::string path = extractPath(req);

            int currentStatus = 200;
            std::string currentBody;

            for (const auto& stmt : statements) {
                std::string routeMethod;
                switch (stmt.cmd) {
                    case BackendCmd::GET: routeMethod = "GET"; break;
                    case BackendCmd::POST: routeMethod = "POST"; break;
                    case BackendCmd::PUT: routeMethod = "PUT"; break;
                    case BackendCmd::DEL: routeMethod = "DELETE"; break;
                    default: continue;
                }
                if (!stmt.stringArgs.empty() && method == routeMethod && path == stmt.stringArgs[0]) {
                    executeBackendStmts(stmt.children, cache, errorHandlers, vars, currentBody, currentStatus);
                }
            }

            if (currentBody.empty() && currentStatus == 200) {
                currentStatus = 404;
                currentBody = "{\"error\":\"Not found\",\"path\":\"" + path + "\"}";
            }

            std::string response = "HTTP/1.1 " + std::to_string(currentStatus) + " OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Access-Control-Allow-Origin: *\r\n";
            response += "Content-Length: " + std::to_string(currentBody.size()) + "\r\n";
            response += "Connection: close\r\n\r\n";
            response += currentBody;

            send(client, response.c_str(), (int)response.size(), 0);
            closesocket(client);
        }
        closesocket(server);
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
    static std::string extractPath(const std::string& req) {
        size_t start = req.find(' ');
        if (start == std::string::npos) return "/";
        size_t end = req.find(' ', start + 1);
        if (end == std::string::npos) return "/";
        return req.substr(start + 1, end - start - 1);
    }

    static void executeBackendStmts(const std::vector<BackendStmt>& stmts,
                                     std::unordered_map<std::string, std::string>& cache,
                                     std::unordered_map<std::string, std::vector<BackendStmt>>& errorHandlers,
                                     double* vars,
                                     std::string& currentBody,
                                     int& currentStatus) {
        for (const auto& stmt : stmts) {
            if (stmt.hasCondition) {
                double varVal = 0;
                if (stmt.condVar.find("V_A_R") != std::string::npos) varVal = vars[0];
                else if (stmt.condVar.find("R_O_W_S") != std::string::npos) varVal = vars[0];
                else if (stmt.condVar.find("E_R_R_O_R_S") != std::string::npos) varVal = vars[0];

                if (varVal == stmt.condValue) {
                    executeBackendStmts(stmt.ifBranch, cache, errorHandlers, vars, currentBody, currentStatus);
                } else {
                    executeBackendStmts(stmt.elseBranch, cache, errorHandlers, vars, currentBody, currentStatus);
                }
                continue;
            }

            switch (stmt.cmd) {
                case BackendCmd::JSON:
                    if (!stmt.stringArgs.empty()) currentBody = unescapeJSON(stmt.stringArgs[0]);
                    break;
                case BackendCmd::STATUS:
                    if (!stmt.numberArgs.empty()) currentStatus = (int)stmt.numberArgs[0];
                    break;
                case BackendCmd::SET:
                    if (stmt.stringArgs.size() >= 2)
                        cache[stmt.stringArgs[0]] = stmt.stringArgs[1];
                    break;
                case BackendCmd::GET_KEY:
                    if (!stmt.stringArgs.empty() && cache.find(stmt.stringArgs[0]) != cache.end())
                        currentBody = "\"" + cache[stmt.stringArgs[0]] + "\"";
                    else if (!stmt.stringArgs.empty())
                        currentBody = "null";
                    break;
                case BackendCmd::FILE_WRITE:
                    if (stmt.stringArgs.size() >= 2)
                        writeFile(stmt.stringArgs[0], stmt.stringArgs[1]);
                    break;
                case BackendCmd::FILE_READ:
                    if (!stmt.stringArgs.empty())
                        currentBody = "\"" + readFile(stmt.stringArgs[0]) + "\"";
                    break;
                case BackendCmd::TRY:
                    executeBackendStmts(stmt.children, cache, errorHandlers, vars, currentBody, currentStatus);
                    break;
                case BackendCmd::THROW:
                    if (!stmt.stringArgs.empty() && errorHandlers.find(stmt.stringArgs[0]) != errorHandlers.end()) {
                        executeBackendStmts(errorHandlers[stmt.stringArgs[0]], cache, errorHandlers, vars, currentBody, currentStatus);
                    } else if (errorHandlers.find("*") != errorHandlers.end()) {
                        executeBackendStmts(errorHandlers["*"], cache, errorHandlers, vars, currentBody, currentStatus);
                    }
                    break;
                case BackendCmd::QRY:
                    if (!stmt.stringArgs.empty()) {
                        std::string sql = stmt.stringArgs[0];
                        if (sql.find("SELECT") != std::string::npos || sql.find("select") != std::string::npos) {
                            currentBody = querySelect(sql);
                            vars[0] = 0;
                        } else if (sql.find("INSERT") != std::string::npos || sql.find("insert") != std::string::npos) {
                            queryExec(sql);
                            vars[0] = 1;
                        } else if (sql.find("UPDATE") != std::string::npos || sql.find("update") != std::string::npos) {
                            queryExec(sql);
                            vars[0] = 1;
                        } else if (sql.find("DELETE") != std::string::npos || sql.find("delete") != std::string::npos) {
                            queryExec(sql);
                            vars[0] = 1;
                        }
                    }
                    break;
                case BackendCmd::INSERT:
                    if (stmt.stringArgs.size() >= 2) {
                        std::string table = stmt.stringArgs[0];
                        std::string values = stmt.stringArgs[1];
                        std::string sql = "INSERT INTO " + table + " VALUES (" + values + ")";
                        queryExec(sql);
                        vars[0] = 1;
                    }
                    break;
                case BackendCmd::FIND:
                    if (stmt.stringArgs.size() >= 1) {
                        std::string table = stmt.stringArgs[0];
                        std::string condition = stmt.stringArgs.size() >= 2 ? stmt.stringArgs[1] : "1=1";
                        std::string sql = "SELECT * FROM " + table + " WHERE " + condition;
                        currentBody = querySelect(sql);
                    }
                    break;
                case BackendCmd::UPDATE:
                    if (stmt.stringArgs.size() >= 3) {
                        std::string table = stmt.stringArgs[0];
                        std::string sets = stmt.stringArgs[1];
                        std::string condition = stmt.stringArgs[2];
                        std::string sql = "UPDATE " + table + " SET " + sets + " WHERE " + condition;
                        queryExec(sql);
                    }
                    break;
                case BackendCmd::DELETE_ROW:
                    if (stmt.stringArgs.size() >= 2) {
                        std::string table = stmt.stringArgs[0];
                        std::string condition = stmt.stringArgs[1];
                        std::string sql = "DELETE FROM " + table + " WHERE " + condition;
                        queryExec(sql);
                    }
                    break;
                case BackendCmd::DBC:
                    if (!stmt.stringArgs.empty()) {
                        dbConnect(stmt.stringArgs[0]);
                    }
                    break;
                case BackendCmd::MKDIR:
                    if (!stmt.stringArgs.empty())
                        CreateDirectoryA(stmt.stringArgs[0].c_str(), NULL);
                    break;
                case BackendCmd::DEL_FILE:
                    if (!stmt.stringArgs.empty())
                        DeleteFileA(stmt.stringArgs[0].c_str());
                    break;
                case BackendCmd::DEL_KEY:
                    if (!stmt.stringArgs.empty())
                        cache.erase(stmt.stringArgs[0]);
                    break;
                case BackendCmd::LIST_DIR:
                    if (!stmt.stringArgs.empty()) {
                        currentBody = listDirectory(stmt.stringArgs[0]);
                    }
                    break;
                case BackendCmd::EMAIL:
                    if (stmt.stringArgs.size() >= 1) {
                        std::string to = stmt.stringArgs[0];
                        std::string subject = stmt.stringArgs.size() >= 2 ? stmt.stringArgs[1] : "";
                        std::string body = stmt.stringArgs.size() >= 3 ? stmt.stringArgs[2] : "";
                        sendEmail(to, subject, body);
                    }
                    break;
                case BackendCmd::CACHE:
                    break;
                case BackendCmd::EXPIRE:
                    break;
                case BackendCmd::AUTH:
                    break;
                case BackendCmd::LOGIN:
                    break;
                case BackendCmd::SIGNUP:
                    break;
                case BackendCmd::LOGOUT:
                    break;
                case BackendCmd::SESSION:
                    break;
                case BackendCmd::MIDDLEWARE:
                    break;
                case BackendCmd::WS:
                    break;
                case BackendCmd::EMIT:
                    break;
                case BackendCmd::ON:
                    break;
                case BackendCmd::BCAST:
                    break;
                case BackendCmd::CRON:
                    break;
                case BackendCmd::VALID:
                    break;
                default: break;
            }
        }
    }

    static std::string dbPath;
    static std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> dbTables;

    static void dbConnect(const std::string& path) {
        dbPath = path;
        std::ifstream f(path);
        if (f) {
            std::string line;
            std::string currentTable;
            while (std::getline(f, line)) {
                if (line.empty()) continue;
                if (line[0] == '[') {
                    size_t end = line.find(']');
                    if (end != std::string::npos) {
                        currentTable = line.substr(1, end - 1);
                        dbTables[currentTable] = {};
                    }
                } else if (!currentTable.empty()) {
                    std::unordered_map<std::string, std::string> row;
                    size_t pos = 0;
                    while (pos < line.size()) {
                        size_t eq = line.find('=', pos);
                        size_t sep = line.find('|', pos);
                        if (eq == std::string::npos || (sep != std::string::npos && sep < eq)) {
                            if (sep == std::string::npos) break;
                            std::string val = line.substr(pos, sep - pos);
                            row[std::to_string(row.size())] = val;
                            pos = sep + 1;
                        } else {
                            std::string key = line.substr(pos, eq - pos);
                            size_t end = line.find('|', eq);
                            std::string val = (end != std::string::npos) ? line.substr(eq + 1, end - eq - 1) : line.substr(eq + 1);
                            row[key] = val;
                            pos = (end != std::string::npos) ? end + 1 : line.size();
                        }
                    }
                    if (!row.empty()) dbTables[currentTable].push_back(row);
                }
            }
        }
    }

    static std::string querySelect(const std::string& sql) {
        std::string table = extractTable(sql);
        std::ostringstream json;
        json << "[";
        if (dbTables.find(table) != dbTables.end()) {
            bool first = true;
            for (const auto& row : dbTables[table]) {
                if (!first) json << ",";
                first = false;
                json << "{";
                bool firstCol = true;
                for (const auto& [key, val] : row) {
                    if (!firstCol) json << ",";
                    firstCol = false;
                    json << "\"" << key << "\":\"" << val << "\"";
                }
                json << "}";
            }
        }
        json << "]";
        return json.str();
    }

    static void queryExec(const std::string& sql) {
        std::string table = extractTable(sql);
        if (sql.find("INSERT") != std::string::npos || sql.find("insert") != std::string::npos) {
            std::unordered_map<std::string, std::string> row;
            size_t paren = sql.find('(');
            if (paren != std::string::npos) {
                std::string values = sql.substr(paren + 1);
                if (!values.empty() && values.back() == ')') values.pop_back();
                int col = 0;
                size_t pos = 0;
                while (pos < values.size()) {
                    size_t comma = values.find(',', pos);
                    std::string val = (comma != std::string::npos) ? values.substr(pos, comma - pos) : values.substr(pos);
                    while (!val.empty() && val.front() == ' ') val.erase(0, 1);
                    while (!val.empty() && val.back() == ' ') val.pop_back();
                    if (!val.empty() && val.front() == '\'' && val.back() == '\'') val = val.substr(1, val.size() - 2);
                    row[std::to_string(col++)] = val;
                    if (comma == std::string::npos) break;
                    pos = comma + 1;
                }
            }
            if (!row.empty()) dbTables[table].push_back(row);
        }
        saveDBFile();
    }

    static std::string extractTable(const std::string& sql) {
        std::string upper = sql;
        for (char& c : upper) c = (char)std::toupper(c);
        size_t from = upper.find("FROM ");
        if (from == std::string::npos) {
            size_t into = upper.find("INTO ");
            if (into != std::string::npos) {
                size_t start = into + 5;
                size_t end = upper.find(' ', start);
                if (end == std::string::npos) end = upper.find('(', start);
                return upper.substr(start, end - start);
            }
            size_t update = upper.find("UPDATE ");
            if (update != std::string::npos) {
                size_t start = update + 7;
                size_t end = upper.find(' ', start);
                return upper.substr(start, end - start);
            }
            size_t del = upper.find("DELETE FROM ");
            if (del != std::string::npos) {
                size_t start = del + 12;
                size_t end = upper.find(' ', start);
                return upper.substr(start, end - start);
            }
            return "";
        }
        size_t start = from + 5;
        size_t end = upper.find(' ', start);
        if (end == std::string::npos) end = upper.size();
        return upper.substr(start, end - start);
    }

    static void saveDBFile() {
        if (dbPath.empty()) return;
        std::ofstream f(dbPath);
        for (const auto& [table, rows] : dbTables) {
            f << "[" << table << "]\n";
            for (const auto& row : rows) {
                bool first = true;
                for (const auto& [key, val] : row) {
                    if (!first) f << "|";
                    first = false;
                    f << key << "=" << val;
                }
                f << "\n";
            }
        }
    }

    static std::string listDirectory(const std::string& path) {
        std::ostringstream json;
        json << "[";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            bool first = true;
            do {
                if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
                if (!first) json << ",";
                first = false;
                json << "\"" << findData.cFileName << "\"";
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
        json << "]";
        return json.str();
    }

    static void sendEmail(const std::string& to, const std::string& subject, const std::string& body) {
        std::ofstream f("mailbox.log", std::ios::app);
        f << "TO: " << to << "\nSUBJECT: " << subject << "\nBODY: " << body << "\n---\n";
    }

    static std::string unescapeJSON(const std::string& s) {
        std::string out;
        for (size_t i = 0; i < s.size(); i++) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if (s[i+1] == '"') { out += '"'; i++; }
                else if (s[i+1] == '\\') { out += '\\'; i++; }
                else if (s[i+1] == 'n') { out += '\n'; i++; }
                else { out += s[i]; }
            } else {
                out += s[i];
            }
        }
        return out;
    }

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
// Static member definitions
std::string Runtime::dbPath;
std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> Runtime::dbTables;
} // namespace hsx
#endif