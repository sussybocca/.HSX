#ifndef HSX_TOJS_H
#define HSX_TOJS_H

#include "Parser.h"
#include <sstream>
#include <unordered_set>
#include <algorithm>

namespace hsx {

class HSXtoJS {
public:
    static std::string transpile(const std::vector<ASTPtr>& program) {
        std::ostringstream js;
        js << "(function() {\n";
        js << "  const h = window.__hsx;\n";
        js << "  const $ = (id) => document.getElementById(id);\n\n";

        int indent = 1;
        std::unordered_set<std::string> selfClosing = {
            "+B_R_K+", "+H_R_L+", "+S_H_O_W+", "+H_I_D_E+", "+C_E_N_T+", "+S_T_I_C_K+"
        };
        std::unordered_set<std::string> closers = {
            "+/D_I_V+", "+/S_E_C+", "+/H_D_R+", "+/F_T_R+", "+/N_A_V+",
            "+/M_A_I_N+", "+/A_S_D_E+", "+/G_R_D+", "+/F_L_X+", "+/T_B_L+",
            "+/T_R_W+", "+/T_C_L+", "+/T_A_B_S+", "+/F_R_M+", "+/M_E_N_U+"
        };

        for (auto& node : program) {
            if (auto* fe = dynamic_cast<FrontendNode*>(node.get())) {
                std::string tag = fe->tag;
                
                if (closers.count(tag)) {
                    indent = std::max(1, indent - 1);
                    js << std::string(indent * 2, ' ') << "h.pop();\n";
                    continue;
                }

                js << std::string(indent * 2, ' ');

                if (tag == "+T_X_T+")          js << "h.text(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_1+")        js << "h.heading(1, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_2+")        js << "h.heading(2, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_3+")        js << "h.heading(3, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_4+")        js << "h.heading(4, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_5+")        js << "h.heading(5, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+H_6+")        js << "h.heading(6, \"" << esc(fe->content) << "\");\n";
                else if (tag == "+S_P_N+")      js << "h.span(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+B_L_D+")      js << "h.bold(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+I_T_L+")      js << "h.italic(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+U_N_D+")      js << "h.underline(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+C_O_D+")      js << "h.code(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+P_R_E+")      js << "h.pre(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+Q_T_E+")      js << "h.blockquote(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+B_R_K+")      js << "h.br();\n";
                else if (tag == "+H_R_L+")      js << "h.hr();\n";

                else if (tag == "+B_A_I_T+")    js << "h.button(\"" << esc(fe->content) << "\", function() {});\n";
                else if (tag == "+B_T_N_G+")    js << "h.buttongroup(" << num(fe, 0) << ", \"" << esc(fe->content) << "\");\n";
                else if (tag == "+I_N_U_T+")    js << "h.input(\"" << esc(fe->content) << "\", function(v) { h.store('_input', v); });\n";
                else if (tag == "+T_X_T_A+")    js << "h.textarea(\"" << esc(fe->content) << "\", " << num(fe, 0) << ");\n";
                else if (tag == "+D_R_P+")      js << "h.dropdown([" << strList(fe) << "]);\n";
                else if (tag == "+C_H_K+")      js << "h.checkbox(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+R_D_O+")      js << "h.radio(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+T_G_L+")      js << "h.toggle(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+S_L_D+")      js << "h.slider(" << num(fe, 0) << ", " << num(fe, 1) << ", " << num(fe, 2) << ");\n";
                else if (tag == "+D_T_P+")      js << "h.datepicker(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+C_L_R+")      js << "h.colorpicker(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+F_I_L_U+")    js << "h.fileupload(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+P_R_G+")      js << "h.progress(" << num(fe, 0) << ", " << num(fe, 1) << ");\n";
                else if (tag == "+R_A_N_G+")    js << "h.range(" << num(fe, 0) << ", " << num(fe, 1) << ", " << num(fe, 2) << ");\n";

                else if (tag == "+I_M_G+") {
                    if (fe->extraNumbers.size() >= 2)
                        js << "h.image(\"" << esc(fe->content) << "\", " << num(fe, 0) << ", " << num(fe, 1) << ");\n";
                    else
                        js << "h.image(\"" << esc(fe->content) << "\");\n";
                }
                else if (tag == "+V_I_D+")      js << "h.video(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+A_U_D+")      js << "h.audio(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+I_C_N+")      js << "h.icon(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+C_N_V+")      js << "h.canvas(" << num(fe, 0) << ", " << num(fe, 1) << ");\n";
                else if (tag == "+S_V_G+")      js << "h.svg(`" << esc(fe->content) << "`);\n";
                else if (tag == "+V_I_D_B+")    js << "h.videoEmbed(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+M_A_P+")      js << "h.map(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+I_F_R_M+")    js << "h.iframe(\"" << esc(fe->content) << "\", " << num(fe, 0) << ", " << num(fe, 1) << ");\n";

                else if (tag == "+T_B_L+")      { js << "h.tableOpen();\n"; indent++; }
                else if (tag == "+T_R_W+")      { js << "h.tableRow();\n"; indent++; }
                else if (tag == "+T_C_L+")      js << "h.tableCell(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+L_S_T+")      js << "h.list([" << strList(fe) << "]);\n";
                else if (tag == "+O_L_S_T+")    js << "h.orderedList([" << strList(fe) << "]);\n";
                else if (tag == "+C_A_R_D+")    js << "h.card(\"" << esc(fe->content) << "\", \"" << esc(fe, 0) << "\", \"" << esc(fe, 1) << "\");\n";
                else if (tag == "+B_A_D_G+")    js << "h.badge(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+A_L_R_T+")    js << "h.alert(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+M_O_D_L+")    js << "h.modal(\"" << esc(fe->content) << "\", \"" << esc(fe, 0) << "\");\n";
                else if (tag == "+T_O_L_T+")    js << "h.tooltip(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+A_C_C_R+")    js << "h.accordion(\"" << esc(fe->content) << "\", \"" << esc(fe, 0) << "\");\n";
                else if (tag == "+T_A_B_S+")    { js << "h.tabsOpen(" << num(fe, 0) << ");\n"; indent++; }
                else if (tag == "+T_A_B+")      js << "h.tab(\"" << esc(fe->content) << "\");\n";

                else if (tag == "+S_T_Y_L+")    js << "h.style(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+C_L_A_S_S+")  js << "h.className(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+I_D+")        js << "h.id(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+A_N_I_M+")    js << "h.animate(\"" << esc(fe->content) << "\", " << num(fe, 0) << ");\n";
                else if (tag == "+H_O_V_R+")    js << "h.hover(\"" << esc(fe->content) << "\");\n";

                else if (tag == "+F_R_M+")      { js << "h.formOpen(\"" << esc(fe->content) << "\");\n"; indent++; }
                else if (tag == "+L_B_L+")      js << "h.label(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+F_L_D+")      js << "h.field(\"" << esc(fe->content) << "\", \"" << esc(fe, 0) << "\");\n";
                else if (tag == "+S_U_B_M+")    js << "h.submit(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+R_S_E_T+")    js << "h.reset(\"" << esc(fe->content) << "\");\n";

                else if (tag == "+M_E_N_U+")    { js << "h.menuOpen();\n"; indent++; }
                else if (tag == "+M_E_N_I+")    js << "h.menuItem(\"" << esc(fe->content) << "\", \"" << esc(fe, 0) << "\");\n";
                else if (tag == "+B_R_E_A_D+")  js << "h.breadcrumb(\"" << esc(fe->content) << "\");\n";
                else if (tag == "+P_A_G_E+")    js << "h.pagination(" << num(fe, 0) << ", " << num(fe, 1) << ");\n";
                else if (tag == "+D_R_P_M+")    js << "h.dropdownMenu([" << strList(fe) << "]);\n";
                else if (tag == "+T_A_B_B+")    js << "h.tabBar([" << strList(fe) << "]);\n";
                else if (tag == "+S_E_A_R_C_H+")js << "h.search(\"" << esc(fe->content) << "\");\n";

                else if (tag == "+S_P_C_R+")    js << "h.spacer(" << num(fe, 0) << ");\n";
                else if (tag == "+P_A_D+")      js << "h.pad(" << num(fe, 0) << ");\n";
                else if (tag == "+M_A_R_G+")    js << "h.margin(" << num(fe, 0) << ");\n";
                else if (tag == "+C_E_N_T+")    js << "h.center();\n";
                else if (tag == "+H_I_D_E+")    js << "h.hide();\n";
                else if (tag == "+S_H_O_W+")    js << "h.show();\n";
                else if (tag == "+S_C_R_L_L+")  js << "h.scroll(" << num(fe, 0) << ");\n";
                else if (tag == "+S_T_I_C_K+")  js << "h.sticky();\n";
                else if (tag == "+F_I_X_D+")    js << "h.fixed(\"" << esc(fe->content) << "\");\n";

                else if (tag == "+O_N_C_L+")    js << "h.onClick(function() { " << esc(fe->content) << "; });\n";
                else if (tag == "+O_N_C_H_G+")  js << "h.onChange(function(e) { " << esc(fe->content) << "; });\n";
                else if (tag == "+O_N_L_D+")    js << "h.onLoad(function() { " << esc(fe->content) << "; });\n";

                else if (tag == "+D_I_V+")      { js << "h.divOpen();\n"; indent++; }
                else if (tag == "+S_E_C+")      { js << "h.section(\"" << esc(fe->content) << "\");\n"; indent++; }
                else if (tag == "+H_D_R+")      { js << "h.headerOpen();\n"; indent++; }
                else if (tag == "+F_T_R+")      { js << "h.footerOpen();\n"; indent++; }
                else if (tag == "+N_A_V+")      { js << "h.navOpen(\"" << esc(fe->content) << "\");\n"; indent++; }
                else if (tag == "+M_A_I_N+")    { js << "h.mainOpen();\n"; indent++; }
                else if (tag == "+A_S_D_E+")    { js << "h.asideOpen();\n"; indent++; }
                else if (tag == "+G_R_D+")      { js << "h.gridOpen(" << num(fe, 0) << ");\n"; indent++; }
                else if (tag == "+F_L_X+")      { js << "h.flexOpen(" << num(fe, 0) << ");\n"; indent++; }

                else if (tag == "+I_F_R_E_N_D+")js << "if (h.getVar('" << esc(fe->content) << "') == " << num(fe, 0) << ") {\n";
                else if (tag == "+L_O_O_P_R_N_D+")js << "h.loopRender('" << esc(fe->content) << "', function(item) {\n";
                else if (tag == "+B_I_N_D+")    js << "h.bind('" << esc(fe->content) << "');\n";
                else if (tag == "+M_O_D_L+")    js << "h.model('" << esc(fe->content) << "');\n";

                else js << "// Unknown frontend: " << tag << "\n";
            }
            else if (auto* be = dynamic_cast<BackendNode*>(node.get())) {
                js << std::string(indent * 2, ' ');
                std::string cmd = be->command;
                if (cmd == "{A_P_I}")           js << "h.api('" << esc(be->params) << "');\n";
                else if (cmd == "{F_I_L}")      js << "h.fileRead('" << esc(be->params) << "');\n";
                else if (cmd == "{S_R_V}")      js << "// Server start: port " << be->numberVal << " (browser cannot run servers)\n";
                else if (cmd == "{D_B_C}")      js << "h.dbConnect('" << esc(be->params) << "');\n";
                else if (cmd == "{Q_R_Y}")      js << "h.query('" << esc(be->params) << "');\n";
                else if (cmd == "{J_S_N}")      js << "h.jsonParse();\n";
                else js << "// Backend: " << cmd << "\n";
            }
            else if (auto* p = dynamic_cast<PrintNode*>(node.get())) {
                js << std::string(indent * 2, ' ') << "h.print(\"" << esc(p->value) << "\");\n";
            }
            else if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
                js << std::string(indent * 2, ' ');
                if (a->type == AssignmentNode::ValType::NUMBER)
                    js << "h.setVar('" << a->name << "', " << std::get<double>(a->value) << ");\n";
                else if (a->type == AssignmentNode::ValType::STRING)
                    js << "h.setVar('" << a->name << "', \"" << esc(std::get<std::string>(a->value)) << "\");\n";
                else if (a->type == AssignmentNode::ValType::LIST) {
                    auto& vec = std::get<std::vector<double>>(a->value);
                    js << "h.setVar('" << a->name << "', [";
                    for (size_t j = 0; j < vec.size(); ++j) {
                        if (j) js << ", ";
                        js << vec[j];
                    }
                    js << "]);\n";
                }
            }
            else if (auto* l = dynamic_cast<LoopNode*>(node.get())) {
                js << std::string(indent * 2, ' ') << "for (let _i = 0; _i < " << l->count << "; _i++) {\n";
                indent++;
            }
            else if (auto* w = dynamic_cast<WhileLoopNode*>(node.get())) {
                js << std::string(indent * 2, ' ') << "while (h.getVar('" << w->varName << "') == " << w->compareValue << ") {\n";
                indent++;
            }
            else if (auto* ie = dynamic_cast<IfElsePrintNode*>(node.get())) {
                js << std::string(indent * 2, ' ') << "if (h.getVar('" << ie->condVar << "') == " << ie->condValue << ") {\n";
                js << std::string((indent+1)*2, ' ') << "h.print(\"" << esc(ie->ifBranch) << "\");\n";
                js << std::string(indent * 2, ' ') << "} else {\n";
                js << std::string((indent+1)*2, ' ') << "h.print(\"" << esc(ie->elseBranch) << "\");\n";
                js << std::string(indent * 2, ' ') << "}\n";
            }
        }

        while (indent > 1) {
            indent--;
            js << std::string(indent * 2, ' ') << "h.pop();\n";
        }

        js << "})();\n";
        return js.str();
    }

private:
    static std::string esc(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else out += c;
        }
        return out;
    }

    static std::string esc(const FrontendNode* fe, size_t idx) {
        if (idx < fe->extraParams.size()) return esc(fe->extraParams[idx]);
        return "";
    }

    static std::string num(const FrontendNode* fe, size_t idx) {
        if (idx < fe->extraNumbers.size()) return std::to_string(fe->extraNumbers[idx]);
        return "0";
    }

    static std::string strList(const FrontendNode* fe) {
        std::string out;
        if (!fe->content.empty()) out += "\"" + esc(fe->content) + "\"";
        for (auto& p : fe->extraParams) {
            if (!out.empty()) out += ", ";
            out += "\"" + esc(p) + "\"";
        }
        return out;
    }
};

} // namespace hsx
#endif