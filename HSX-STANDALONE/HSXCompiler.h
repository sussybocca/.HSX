#ifndef HSX_COMPILER_PAGE_H
#define HSX_COMPILER_PAGE_H

#include "Parser.h"
#include "HSXtoJS.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <functional>

namespace hsx {

struct CompiledPage {
    std::string js;
    std::string json;
    std::string staticHTML;
    std::string hash;
};

class HSXPageCompiler {
public:
    static CompiledPage compile(const std::string& source) {
        CompiledPage result;

        std::vector<std::unique_ptr<ASTNode>> program;
        std::istringstream stream(source);
        std::string line;
        while (std::getline(stream, line)) {
            if (!line.empty()) {
                try {
                    program.push_back(Parser::parseLine(line));
                } catch (...) {}
            }
        }

        result.js = HSXtoJS::transpile(program);
        result.json = extractData(program);
        result.staticHTML = preRender(source);
        result.hash = hashString(result.js + result.json + result.staticHTML);

        return result;
    }

private:
    static std::string extractData(const std::vector<std::unique_ptr<ASTNode>>& program) {
        std::ostringstream json;
        json << "{";
        bool first = true;
        for (const auto& node : program) {
            if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
                if (!first) json << ",";
                first = false;
                json << "\"" << a->name << "\":";
                if (a->type == AssignmentNode::ValType::NUMBER) {
                    json << std::get<double>(a->value);
                } else if (a->type == AssignmentNode::ValType::STRING) {
                    json << "\"" << std::get<std::string>(a->value) << "\"";
                } else {
                    json << "[]";
                }
            }
        }
        json << "}";
        return json.str();
    }

    static std::string preRender(const std::string& source) {
        std::ostringstream html;
        html << "<div id=\"hsx-root\">\n";
        
        std::istringstream stream(source);
        std::string line;
        std::string pendingClass;
        bool inDiv = false;
        bool inGrid = false;
        bool inFlex = false;
        bool inCard = false;
        bool inForm = false;
        bool inNav = false;
        bool inSection = false;
        bool inHeader = false;
        bool inFooter = false;
        bool inMain = false;
        bool inAside = false;
        
        while (std::getline(stream, line)) {
            // Theme
            if (line.find("+T_H_E_M_E+") != std::string::npos) {
                std::string theme = extractContent(line);
                html << "<script>document.documentElement.setAttribute('data-theme','" << theme << "');</script>\n";
            }
            // Style pending
            else if (line.find("+B_G+") != std::string::npos)            pendingClass = "hsx-bg";
            else if (line.find("+C_O_L_O_R+") != std::string::npos)      pendingClass = "hsx-fg";
            else if (line.find("+F_O_N_T+") != std::string::npos)        pendingClass = "hsx-text-" + extractContent(line);
            else if (line.find("+P_A_D_D_I_N_G+") != std::string::npos)  pendingClass = "hsx-p-2";
            else if (line.find("+M_A_R_G_I_N+") != std::string::npos)    pendingClass = "hsx-m-2";
            else if (line.find("+R_A_D_I_U_S+") != std::string::npos)    pendingClass = "hsx-rounded";
            else if (line.find("+S_H_A_D_O_W+") != std::string::npos)    pendingClass = "hsx-shadow";
            else if (line.find("+C_E_N_T_E_R+") != std::string::npos)    pendingClass = "hsx-center";
            else if (line.find("+S_T_A_C_K+") != std::string::npos)      pendingClass = "hsx-stack";
            else if (line.find("+R_O_W+") != std::string::npos)          pendingClass = "hsx-row";
            // Typography
            else if (line.find("+T_X_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <p" << classAttr(pendingClass) << ">" << text << "</p>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_1+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h1" << classAttr(pendingClass) << ">" << text << "</h1>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_2+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h2" << classAttr(pendingClass) << ">" << text << "</h2>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_3+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h3" << classAttr(pendingClass) << ">" << text << "</h3>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_4+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h4" << classAttr(pendingClass) << ">" << text << "</h4>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_5+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h5" << classAttr(pendingClass) << ">" << text << "</h5>\n";
                pendingClass.clear();
            }
            else if (line.find("+H_6+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <h6" << classAttr(pendingClass) << ">" << text << "</h6>\n";
                pendingClass.clear();
            }
            else if (line.find("+S_P_N+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <span" << classAttr(pendingClass) << ">" << text << "</span>\n";
                pendingClass.clear();
            }
            else if (line.find("+B_L_D+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <b" << classAttr(pendingClass) << ">" << text << "</b>\n";
                pendingClass.clear();
            }
            else if (line.find("+I_T_L+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <i" << classAttr(pendingClass) << ">" << text << "</i>\n";
                pendingClass.clear();
            }
            else if (line.find("+U_N_D+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <u" << classAttr(pendingClass) << ">" << text << "</u>\n";
                pendingClass.clear();
            }
            else if (line.find("+C_O_D+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <code class=\"hsx-font-mono" << (pendingClass.empty() ? "" : " " + pendingClass) << "\">" << text << "</code>\n";
                pendingClass.clear();
            }
            else if (line.find("+P_R_E+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <pre class=\"hsx-font-mono" << (pendingClass.empty() ? "" : " " + pendingClass) << "\">" << text << "</pre>\n";
                pendingClass.clear();
            }
            else if (line.find("+Q_T_E+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <blockquote" << classAttr(pendingClass) << ">" << text << "</blockquote>\n";
                pendingClass.clear();
            }
            else if (line.find("+B_R_K+") != std::string::npos) {
                html << "  <br />\n";
            }
            else if (line.find("+H_R_L+") != std::string::npos) {
                html << "  <hr />\n";
            }
            // Interactive
            else if (line.find("+B_A_I_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <button class=\"hsx-btn" << (pendingClass.empty() ? "" : " " + pendingClass) << "\" data-hsx-hydrate=\"button\">" << text << "</button>\n";
                pendingClass.clear();
            }
            else if (line.find("+I_N_U_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input class=\"hsx-input" << (pendingClass.empty() ? "" : " " + pendingClass) << "\" placeholder=\"" << text << "\" data-hsx-hydrate=\"input\" />\n";
                pendingClass.clear();
            }
            else if (line.find("+T_X_T_A+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <textarea class=\"hsx-input" << (pendingClass.empty() ? "" : " " + pendingClass) << "\" placeholder=\"" << text << "\" data-hsx-hydrate=\"input\"></textarea>\n";
                pendingClass.clear();
            }
            else if (line.find("+D_R_P+") != std::string::npos) {
                html << "  <select class=\"hsx-input" << (pendingClass.empty() ? "" : " " + pendingClass) << "\" data-hsx-hydrate=\"select\">\n";
                pendingClass.clear();
                html << "  </select>\n";
            }
            else if (line.find("+C_H_K+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"checkbox\" /> <label class=\"hsx-fg\">" << text << "</label>\n";
            }
            else if (line.find("+R_D_O+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"radio\" /> <label class=\"hsx-fg\">" << text << "</label>\n";
            }
            else if (line.find("+T_G_L+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"checkbox\" /> <label class=\"hsx-fg\">" << text << "</label>\n";
            }
            else if (line.find("+S_L_D+") != std::string::npos) {
                html << "  <input type=\"range\" />\n";
            }
            else if (line.find("+D_T_P+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"date\" class=\"hsx-input\" placeholder=\"" << text << "\" />\n";
            }
            else if (line.find("+C_L_R+") != std::string::npos) {
                html << "  <input type=\"color\" />\n";
            }
            else if (line.find("+P_R_G+") != std::string::npos) {
                html << "  <progress></progress>\n";
            }
            // Media
            else if (line.find("+I_M_G+") != std::string::npos) {
                std::string src = extractContent(line);
                html << "  <img src=\"" << src << "\" />\n";
            }
            // Data
            else if (line.find("+T_B_L+") != std::string::npos) {
                html << "  <table class=\"hsx-table\">\n";
            }
            else if (line.find("+T_R_W+") != std::string::npos) {
                html << "    <tr>\n";
            }
            else if (line.find("+T_C_L+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "      <td>" << text << "</td>\n";
            }
            else if (line.find("+L_S_T+") != std::string::npos) {
                html << "  <ul>\n";
                auto items = extractList(line);
                for (auto& item : items) {
                    html << "    <li>" << item << "</li>\n";
                }
                html << "  </ul>\n";
            }
            else if (line.find("+O_L_S_T+") != std::string::npos) {
                html << "  <ol>\n";
                auto items = extractList(line);
                for (auto& item : items) {
                    html << "    <li>" << item << "</li>\n";
                }
                html << "  </ol>\n";
            }
            else if (line.find("+C_A_R_D+") != std::string::npos) {
                html << "  <div class=\"hsx-card\">\n";
                std::string title = extractContent(line);
                if (!title.empty()) html << "    <h3>" << title << "</h3>\n";
                html << "  </div>\n";
            }
            else if (line.find("+B_A_D_G+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <span class=\"hsx-badge\">" << text << "</span>\n";
            }
            // Navigation
            else if (line.find("+M_E_N_I+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <a class=\"hsx-link\" data-hsx-link=\"true\" href=\"#\">" << text << "</a>\n";
            }
            // Layout containers - open
            else if (line.find("+D_I_V+") != std::string::npos && line.find("+/D_I_V+") == std::string::npos) {
                html << "  <div" << classAttr(pendingClass) << ">\n";
                pendingClass.clear();
            }
            else if (line.find("+S_E_C+") != std::string::npos) {
                std::string id = extractContent(line);
                html << "  <section id=\"" << id << "\">\n";
            }
            else if (line.find("+H_D_R+") != std::string::npos) {
                html << "  <header>\n";
            }
            else if (line.find("+F_T_R+") != std::string::npos) {
                html << "  <footer>\n";
            }
            else if (line.find("+N_A_V+") != std::string::npos) {
                std::string id = extractContent(line);
                html << "  <nav id=\"" << id << "\">\n";
            }
            else if (line.find("+M_A_I_N+") != std::string::npos) {
                html << "  <main>\n";
            }
            else if (line.find("+A_S_D_E+") != std::string::npos) {
                html << "  <aside>\n";
            }
            else if (line.find("+G_R_D+") != std::string::npos) {
                html << "  <div class=\"hsx-grid\" style=\"grid-template-columns:repeat(3,1fr)\">\n";
            }
            else if (line.find("+F_L_X+") != std::string::npos) {
                html << "  <div class=\"hsx-flex hsx-gap-2\">\n";
            }
            else if (line.find("+F_R_M+") != std::string::npos) {
                std::string action = extractContent(line);
                html << "  <form action=\"" << action << "\">\n";
            }
            else if (line.find("+M_E_N_U+") != std::string::npos) {
                html << "  <nav>\n";
            }
            // Layout containers - close
            else if (line.find("+/D_I_V+") != std::string::npos || line.find("+/S_E_C+") != std::string::npos ||
                     line.find("+/H_D_R+") != std::string::npos || line.find("+/F_T_R+") != std::string::npos ||
                     line.find("+/N_A_V+") != std::string::npos || line.find("+/M_A_I_N+") != std::string::npos ||
                     line.find("+/A_S_D_E+") != std::string::npos || line.find("+/G_R_D+") != std::string::npos ||
                     line.find("+/F_L_X+") != std::string::npos || line.find("+/F_R_M+") != std::string::npos ||
                     line.find("+/M_E_N_U+") != std::string::npos || line.find("+/T_B_L+") != std::string::npos) {
                // close the last open container
                if (line.find("+/T_B_L+") != std::string::npos) html << "  </table>\n";
                else if (line.find("+/F_R_M+") != std::string::npos) html << "  </form>\n";
                else if (line.find("+/M_E_N_U+") != std::string::npos) html << "  </nav>\n";
                else html << "  </div>\n";
            }
            // Form elements
            else if (line.find("+L_B_L+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <label>" << text << "</label>\n";
            }
            else if (line.find("+F_L_D+") != std::string::npos) {
                std::string name = extractContent(line);
                html << "  <input class=\"hsx-input\" name=\"" << name << "\" />\n";
            }
            else if (line.find("+S_U_B_M+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"submit\" class=\"hsx-btn\" value=\"" << text << "\" />\n";
            }
            else if (line.find("+R_S_E_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"reset\" class=\"hsx-btn\" value=\"" << text << "\" />\n";
            }
            // Spacer
            else if (line.find("+S_P_C_R+") != std::string::npos) {
                html << "  <div style=\"height:2rem\"></div>\n";
            }
            // Search
            else if (line.find("+S_E_A_R_C_H+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <input type=\"search\" class=\"hsx-input\" placeholder=\"" << text << "\" />\n";
            }
            // Alert
            else if (line.find("+A_L_R_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <div class=\"hsx-card hsx-p-2\"><strong>Alert:</strong> " << text << "</div>\n";
            }
            // Tooltip
            else if (line.find("+T_O_L_T+") != std::string::npos) {
                std::string text = extractContent(line);
                html << "  <span title=\"" << text << "\" style=\"border-bottom:1px dotted\">" << text << "</span>\n";
            }
            // Accordion
            else if (line.find("+A_C_C_R+") != std::string::npos) {
                std::string title = extractContent(line);
                html << "  <details><summary>" << title << "</summary><p></p></details>\n";
            }
        }
        html << "</div>\n";
        return html.str();
    }

    static std::string extractContent(const std::string& line) {
        size_t start = line.find("\"\"") + 2;
        size_t end = line.find("\"\"", start);
        if (start != std::string::npos && end != std::string::npos && end > start) {
            return line.substr(start, end - start);
        }
        return "";
    }

    static std::vector<std::string> extractList(const std::string& line) {
        std::vector<std::string> items;
        std::string content = line;
        size_t pos = 0;
        while ((pos = content.find("\"\"", pos)) != std::string::npos) {
            pos += 2;
            size_t end = content.find("\"\"", pos);
            if (end != std::string::npos && end > pos) {
                items.push_back(content.substr(pos, end - pos));
                pos = end + 2;
            } else break;
        }
        return items;
    }

    static std::string classAttr(const std::string& cls) {
        if (cls.empty()) return "";
        return " class=\"" + cls + "\"";
    }

    static std::string hashString(const std::string& input) {
        std::hash<std::string> hasher;
        std::ostringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hasher(input);
        return ss.str().substr(0, 8);
    }
};

} // namespace hsx
#endif