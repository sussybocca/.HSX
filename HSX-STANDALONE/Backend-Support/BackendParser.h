#ifndef HSX_BACKEND_PARSER_H
#define HSX_BACKEND_PARSER_H

#include "Lexer.h"
#include "BackendAST.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace hsx {

class BackendParser {
public:
    static BackendProgram parse(const std::vector<std::string>& lines) {
        BackendProgram program;
        BackendStmt* currentRoute = nullptr;
        BackendStmt* currentTry = nullptr;

        for (size_t lineNum = 0; lineNum < lines.size(); lineNum++) {
            const auto& line = lines[lineNum];

            if (line.find("IMPORT:") == 0) {
                size_t start = line.find("((");
                size_t end = line.find("))");
                if (start != std::string::npos && end != std::string::npos) {
                    std::string module = line.substr(start + 2, end - start - 2);
                    program.imports.push_back(module);
                }
                continue;
            }

            if (line.find("HSX:") != 0) continue;

            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            if (tokens.empty() || tokens[0].type != TokenType::HSX) continue;

            std::vector<BackendStmt> lineStmts;
            for (size_t i = 1; i < tokens.size(); i++) {
                if (tokens[i].type == TokenType::CURLY_BE) {
                    BackendStmt stmt;
                    stmt.cmd = parseCommand(tokens[i].value);
                    for (size_t j = i + 1; j < tokens.size(); j++) {
                        if (tokens[j].type == TokenType::STRING) {
                            stmt.stringArgs.push_back(tokens[j].value);
                        } else if (tokens[j].type == TokenType::EQUALS) {
                            if (j + 1 < tokens.size() && tokens[j+1].type == TokenType::NUMBER) {
                                stmt.numberArgs.push_back(std::stod(tokens[j+1].value));
                                j++;
                            }
                        } else if (tokens[j].type == TokenType::NUMBER) {
                            stmt.numberArgs.push_back(std::stod(tokens[j].value));
                        } else if (tokens[j].type == TokenType::CURLY_BE) {
                            break;
                        }
                    }
                    lineStmts.push_back(stmt);
                }
            }

            if (lineStmts.empty()) continue;

            bool hasCondition = false;
            for (size_t i = 1; i < tokens.size(); i++) {
                if (tokens[i].type == TokenType::C_OPEN) {
                    hasCondition = true;
                    break;
                }
            }

            if (hasCondition && !lineStmts.empty()) {
                BackendStmt condStmt;
                condStmt.hasCondition = true;

                for (size_t i = 1; i < tokens.size(); i++) {
                    if (tokens[i].type == TokenType::C_OPEN) {
                        i++;
                        if (i < tokens.size() && tokens[i].type == TokenType::STAR) {
                            i++;
                            if (i < tokens.size() && tokens[i].type == TokenType::IDENTIFIER) {
                                condStmt.condVar = tokens[i].value;
                                i++;
                                if (i < tokens.size() && tokens[i].type == TokenType::EQUALS) {
                                    i++;
                                    if (i < tokens.size() && tokens[i].type == TokenType::NUMBER) {
                                        condStmt.condValue = std::stod(tokens[i].value);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }

                for (size_t i = 1; i < tokens.size(); i++) {
                    if (tokens[i].type == TokenType::F_OPEN) {
                        i++;
                        while (i < tokens.size() && tokens[i].type != TokenType::F_CLOSE) {
                            if (tokens[i].type == TokenType::CURLY_BE) {
                                BackendStmt child;
                                child.cmd = parseCommand(tokens[i].value);
                                for (size_t j = i + 1; j < tokens.size() && tokens[j].type != TokenType::F_CLOSE && tokens[j].type != TokenType::CURLY_BE; j++) {
                                    if (tokens[j].type == TokenType::STRING) {
                                        child.stringArgs.push_back(tokens[j].value);
                                    } else if (tokens[j].type == TokenType::EQUALS) {
                                        if (j + 1 < tokens.size() && tokens[j+1].type == TokenType::NUMBER) {
                                            child.numberArgs.push_back(std::stod(tokens[j+1].value));
                                            j++;
                                        }
                                    } else if (tokens[j].type == TokenType::NUMBER) {
                                        child.numberArgs.push_back(std::stod(tokens[j].value));
                                    }
                                }
                                condStmt.ifBranch.push_back(child);
                            }
                            i++;
                        }
                        break;
                    }
                }

                for (size_t i = 1; i < tokens.size(); i++) {
                    if (tokens[i].type == TokenType::E_OPEN) {
                        i++;
                        while (i < tokens.size() && tokens[i].type != TokenType::E_CLOSE) {
                            if (tokens[i].type == TokenType::CURLY_BE) {
                                BackendStmt child;
                                child.cmd = parseCommand(tokens[i].value);
                                for (size_t j = i + 1; j < tokens.size() && tokens[j].type != TokenType::E_CLOSE && tokens[j].type != TokenType::CURLY_BE; j++) {
                                    if (tokens[j].type == TokenType::STRING) {
                                        child.stringArgs.push_back(tokens[j].value);
                                    } else if (tokens[j].type == TokenType::EQUALS) {
                                        if (j + 1 < tokens.size() && tokens[j+1].type == TokenType::NUMBER) {
                                            child.numberArgs.push_back(std::stod(tokens[j+1].value));
                                            j++;
                                        }
                                    } else if (tokens[j].type == TokenType::NUMBER) {
                                        child.numberArgs.push_back(std::stod(tokens[j].value));
                                    }
                                }
                                condStmt.elseBranch.push_back(child);
                            }
                            i++;
                        }
                        break;
                    }
                }

                if (currentTry) {
                    currentTry->children.push_back(condStmt);
                } else if (currentRoute) {
                    currentRoute->children.push_back(condStmt);
                } else {
                    std::cerr << "[BackendParser Warning] Line " << (lineNum + 1) 
                              << ": Condition block outside of route or try block, ignored.\n";
                }
                continue;
            }

            for (auto& stmt : lineStmts) {
                if (stmt.cmd == BackendCmd::ON_ERROR) {
                    std::string errorType = stmt.stringArgs.empty() ? "*" : stmt.stringArgs[0];
                    program.errorHandlers[errorType] = std::vector<BackendStmt>();
                }
                else if (isRouteHandler(stmt.cmd)) {
                    program.statements.push_back(stmt);
                    currentRoute = &program.statements.back();
                    currentTry = nullptr;
                }
                else if (currentTry) {
                    currentTry->children.push_back(stmt);
                }
                else if (currentRoute) {
                    currentRoute->children.push_back(stmt);
                    if (stmt.cmd == BackendCmd::TRY) {
                        currentTry = &currentRoute->children.back();
                    }
                }
                else {
                    program.statements.push_back(stmt);
                    if (stmt.cmd == BackendCmd::TRY) {
                        currentTry = &program.statements.back();
                    }
                }
            }
        }

        std::cerr << "[BackendParser] Parsed " << program.statements.size() << " top-level statements, "
                  << program.imports.size() << " imports, "
                  << program.errorHandlers.size() << " error handlers.\n";
        for (size_t i = 0; i < program.statements.size(); i++) {
            const auto& s = program.statements[i];
            std::cerr << "  Statement " << i << ": cmd=" << (int)s.cmd 
                      << " route=" << (s.stringArgs.empty() ? "(none)" : s.stringArgs[0])
                      << " children=" << s.children.size() << "\n";
        }
        return program;
    }

private:
    static BackendCmd parseCommand(const std::string& token) {
        static std::unordered_map<std::string, BackendCmd> map = {
            {"{S_R_V}", BackendCmd::SRV}, {"{D_B_C}", BackendCmd::DBC}, {"{Q_R_Y}", BackendCmd::QRY},
            {"{T_B_L_E}", BackendCmd::TBL}, {"{I_N_S_R_T}", BackendCmd::INSERT}, {"{F_I_N_D}", BackendCmd::FIND},
            {"{U_P_D_A_T_E}", BackendCmd::UPDATE}, {"{D_E_L_E_T_E}", BackendCmd::DELETE_ROW},
            {"{A_U_T_H}", BackendCmd::AUTH}, {"{L_O_G_I_N}", BackendCmd::LOGIN}, {"{S_I_G_N_U_P}", BackendCmd::SIGNUP},
            {"{L_O_G_O_U_T}", BackendCmd::LOGOUT}, {"{S_E_S_S_I_O_N}", BackendCmd::SESSION}, {"{M_I_D_D_L_E}", BackendCmd::MIDDLEWARE},
            {"{G_E_T}", BackendCmd::GET}, {"{P_O_S_T}", BackendCmd::POST}, {"{P_U_T}", BackendCmd::PUT}, {"{D_E_L}", BackendCmd::DEL},
            {"{R_E_S}", BackendCmd::RES}, {"{J_S_O_N}", BackendCmd::JSON}, {"{S_T_A_T_U_S}", BackendCmd::STATUS},
            {"{H_E_A_D_E_R}", BackendCmd::HEADER}, {"{F_I_L_E}", BackendCmd::FILE_READ}, {"{W_R_I_T_E}", BackendCmd::FILE_WRITE},
            {"{D_E_L_F}", BackendCmd::DEL_FILE}, {"{M_K_D_I_R}", BackendCmd::MKDIR}, {"{L_I_S_T}", BackendCmd::LIST_DIR},
            {"{E_M_A_I_L}", BackendCmd::EMAIL}, {"{S_U_B_J_E_C_T}", BackendCmd::SUBJECT}, {"{B_O_D_Y}", BackendCmd::BODY},
            {"{C_A_C_H_E}", BackendCmd::CACHE}, {"{S_E_T}", BackendCmd::SET}, {"{G_E_T_K}", BackendCmd::GET_KEY},
            {"{E_X_P_I_R_E}", BackendCmd::EXPIRE}, {"{D_E_L_K}", BackendCmd::DEL_KEY},
            {"{W_S}", BackendCmd::WS}, {"{E_M_I_T}", BackendCmd::EMIT}, {"{O_N}", BackendCmd::ON}, {"{B_C_A_S_T}", BackendCmd::BCAST},
            {"{T_R_Y}", BackendCmd::TRY}, {"{T_H_R_O_W}", BackendCmd::THROW}, {"{O_N_E_R_R}", BackendCmd::ON_ERROR},
            {"{V_A_L_I_D}", BackendCmd::VALID}, {"{C_R_O_N}", BackendCmd::CRON},
        };
        auto it = map.find(token);
        if (it == map.end()) {
            std::cerr << "[BackendParser Error] Unknown command: " << token << "\n";
            return BackendCmd::SRV;
        }
        return it->second;
    }

    static bool isRouteHandler(BackendCmd cmd) {
        return cmd == BackendCmd::GET || cmd == BackendCmd::POST || 
               cmd == BackendCmd::PUT || cmd == BackendCmd::DEL ||
               cmd == BackendCmd::WS;
    }
};

} // namespace hsx
#endif