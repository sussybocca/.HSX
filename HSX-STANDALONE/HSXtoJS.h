#ifndef HSX_TOJS_H
#define HSX_TOJS_H

#include "Parser.h"
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <unordered_map>

namespace hsx {

class HSXtoJS {
public:
    static std::string transpile(const std::vector<ASTPtr>& program) {
        std::ostringstream js;
        js << "(function() {\n";
        js << "  var _vars = {};\n";
        js << "  var _stack = [];\n";
        js << "  var _el = document.getElementById('hsx-root') || document.body;\n";
        js << "  function _push(e) { _stack.push(_el); _el = e; }\n";
        js << "  function _pop() { if (_stack.length) _el = _stack.pop(); }\n";
        js << "  function _m(t) { var e = document.createElement(t); _el.appendChild(e); return e; }\n";
        js << "  function _mt(t, x) { var e = _m(t); if (x) e.textContent = x; return e; }\n";
        js << "  var _pendingClass = null;\n\n";

        int indent = 1;

        for (auto& node : program) {
            if (auto* fe = dynamic_cast<FrontendNode*>(node.get())) {
                std::string tag = fe->tag;
                std::string c = esc(fe->content);
                
                // Theme
                if (tag == "+T_H_E_M_E+") {
                    emit(js, indent, "document.documentElement.setAttribute('data-theme', '" + c + "');");
                }
                // Style utilities that set pending class for next element
                else if (tag == "+B_G+")            { emit(js, indent, "_pendingClass = 'hsx-bg';"); }
                else if (tag == "+C_O_L_O_R+")       { emit(js, indent, "_pendingClass = 'hsx-fg';"); }
                else if (tag == "+F_O_N_T+")         { emit(js, indent, "_pendingClass = 'hsx-text-" + c + "';"); }
                else if (tag == "+F_O_N_T_F+")       { emit(js, indent, "document.body.style.fontFamily = '" + c + "';"); }
                else if (tag == "+B_O_R_D_E_R+")     { emit(js, indent, "_pendingClass = 'hsx-border';"); }
                else if (tag == "+R_A_D_I_U_S+")     { emit(js, indent, "_pendingClass = 'hsx-rounded';"); }
                else if (tag == "+S_H_A_D_O_W+")     { emit(js, indent, "_pendingClass = 'hsx-shadow';"); }
                else if (tag == "+W_I_D_T_H+")       { emit(js, indent, "_pendingClass = null; /* width set directly */"); }
                else if (tag == "+H_E_I_G_H_T+")     { emit(js, indent, "_pendingClass = null; /* height set directly */"); }
                else if (tag == "+P_A_D_D_I_N_G+")   { emit(js, indent, "_pendingClass = 'hsx-p-2';"); }
                else if (tag == "+M_A_R_G_I_N+")     { emit(js, indent, "_pendingClass = 'hsx-m-2';"); }
                else if (tag == "+G_A_P+")           { emit(js, indent, "_pendingClass = 'hsx-gap-2';"); }
                else if (tag == "+A_L_I_G_N+")       { emit(js, indent, "_pendingClass = 'hsx-center';"); }
                else if (tag == "+O_P_A_C_I_T_Y+")   { emit(js, indent, "_pendingClass = null;"); }
                else if (tag == "+C_U_R_S_O_R+")     { emit(js, indent, "_pendingClass = null;"); }
                else if (tag == "+T_R_A_N_S+")       { emit(js, indent, "_pendingClass = null;"); }
                else if (tag == "+H_O_V_E_R+")       { emit(js, indent, "_pendingClass = null;"); }
                else if (tag == "+G_R_A_D_I_E_N_T+") { emit(js, indent, "_pendingClass = null;"); }
                // Layout
                else if (tag == "+C_E_N_T_E_R+")     { emit(js, indent, "_pendingClass = 'hsx-center';"); }
                else if (tag == "+S_T_A_C_K+")       { emit(js, indent, "_pendingClass = 'hsx-stack';"); }
                else if (tag == "+R_O_W+")           { emit(js, indent, "_pendingClass = 'hsx-row';"); }
                // Typography
                else if (tag == "+T_X_T+") {
                    emit(js, indent, "var _e = _mt('p', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_1+") {
                    emit(js, indent, "var _e = _mt('h1', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_2+") {
                    emit(js, indent, "var _e = _mt('h2', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_3+") {
                    emit(js, indent, "var _e = _mt('h3', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_4+") {
                    emit(js, indent, "var _e = _mt('h4', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_5+") {
                    emit(js, indent, "var _e = _mt('h5', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+H_6+") {
                    emit(js, indent, "var _e = _mt('h6', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+S_P_N+") {
                    emit(js, indent, "var _e = _mt('span', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+B_L_D+") {
                    emit(js, indent, "var _e = _m('b'); _e.textContent = '" + c + "';");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+I_T_L+") {
                    emit(js, indent, "var _e = _m('i'); _e.textContent = '" + c + "';");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+U_N_D+") {
                    emit(js, indent, "var _e = _m('u'); _e.textContent = '" + c + "';");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+C_O_D+") {
                    emit(js, indent, "var _e = _mt('code', '" + c + "');");
                    emit(js, indent, "_e.classList.add('hsx-font-mono');");
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+P_R_E+") {
                    emit(js, indent, "var _e = _mt('pre', '" + c + "');");
                    emit(js, indent, "_e.classList.add('hsx-font-mono');");
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+Q_T_E+") {
                    emit(js, indent, "var _e = _mt('blockquote', '" + c + "');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                }
                else if (tag == "+B_R_K+")      emit(js, indent, "_el.appendChild(document.createElement('br'));");
                else if (tag == "+H_R_L+")      emit(js, indent, "_el.appendChild(document.createElement('hr'));");
                // Interactive
                else if (tag == "+B_A_I_T+") {
                    emit(js, indent, "var _e = _mt('button', '" + c + "');");
                    emit(js, indent, "_e.className = 'hsx-btn';");
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+I_N_U_T+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    emit(js, indent, "_e.placeholder = '" + c + "';");
                    emit(js, indent, "_e.setAttribute('data-hsx-hydrate', 'input');");
                    emit(js, indent, "_e.name = '_input';");
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+T_X_T_A+") {
                    emit(js, indent, "var _e = _m('textarea');");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    emit(js, indent, "_e.placeholder = '" + c + "';");
                    emit(js, indent, "_e.rows = " + num(fe, 0) + " || 3;");
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+D_R_P+") {
                    emit(js, indent, "var _e = _m('select');");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    auto items = splitList(fe);
                    for (auto& item : items) {
                        emit(js, indent, "var _opt = document.createElement('option');");
                        emit(js, indent, "_opt.textContent = '" + esc(item) + "';");
                        emit(js, indent, "_e.appendChild(_opt);");
                    }
                    emit(js, indent, "if (_pendingClass) { _e.classList.add(_pendingClass); _pendingClass = null; }");
                }
                else if (tag == "+C_H_K+") {
                    emit(js, indent, "var _cb = document.createElement('input');");
                    emit(js, indent, "_cb.type = 'checkbox';");
                    emit(js, indent, "var _lbl = document.createElement('label');");
                    emit(js, indent, "_lbl.textContent = '" + c + "';");
                    emit(js, indent, "_lbl.className = 'hsx-fg';");
                    emit(js, indent, "_el.append(_cb, _lbl);");
                }
                else if (tag == "+R_D_O+") {
                    emit(js, indent, "var _rd = document.createElement('input');");
                    emit(js, indent, "_rd.type = 'radio';");
                    emit(js, indent, "var _lbl = document.createElement('label');");
                    emit(js, indent, "_lbl.textContent = '" + c + "';");
                    emit(js, indent, "_lbl.className = 'hsx-fg';");
                    emit(js, indent, "_el.append(_rd, _lbl);");
                }
                else if (tag == "+T_G_L+") {
                    emit(js, indent, "var _tg = document.createElement('input');");
                    emit(js, indent, "_tg.type = 'checkbox';");
                    emit(js, indent, "var _lbl = document.createElement('label');");
                    emit(js, indent, "_lbl.textContent = '" + c + "';");
                    emit(js, indent, "_lbl.className = 'hsx-fg';");
                    emit(js, indent, "_el.append(_tg, _lbl);");
                }
                else if (tag == "+S_L_D+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'range';");
                    emit(js, indent, "_e.min = " + num(fe, 0) + ";");
                    emit(js, indent, "_e.max = " + num(fe, 1) + ";");
                    emit(js, indent, "_e.value = " + num(fe, 2) + ";");
                }
                else if (tag == "+D_T_P+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'date';");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    emit(js, indent, "_e.placeholder = '" + c + "';");
                }
                else if (tag == "+C_L_R+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'color';");
                    emit(js, indent, "_e.value = '" + c + "';");
                }
                else if (tag == "+P_R_G+") {
                    emit(js, indent, "var _e = _m('progress');");
                    emit(js, indent, "_e.min = " + num(fe, 0) + ";");
                    emit(js, indent, "_e.max = " + num(fe, 1) + ";");
                    emit(js, indent, "_e.value = " + num(fe, 0) + ";");
                }
                // Media
                else if (tag == "+I_M_G+") {
                    emit(js, indent, "var _e = _m('img');");
                    emit(js, indent, "_e.src = '" + c + "';");
                    if (fe->extraNumbers.size() >= 2) {
                        emit(js, indent, "_e.width = " + num(fe, 0) + ";");
                        emit(js, indent, "_e.height = " + num(fe, 1) + ";");
                    }
                }
                else if (tag == "+V_I_D+") {
                    emit(js, indent, "var _e = _m('video');");
                    emit(js, indent, "_e.src = '" + c + "';");
                    emit(js, indent, "_e.controls = true;");
                }
                else if (tag == "+A_U_D+") {
                    emit(js, indent, "var _e = _m('audio');");
                    emit(js, indent, "_e.src = '" + c + "';");
                    emit(js, indent, "_e.controls = true;");
                }
                else if (tag == "+I_C_N+") {
                    emit(js, indent, "var _e = _mt('i', '" + c + "');");
                }
                else if (tag == "+C_N_V+") {
                    emit(js, indent, "var _e = _m('canvas');");
                    emit(js, indent, "_e.width = " + num(fe, 0) + ";");
                    emit(js, indent, "_e.height = " + num(fe, 1) + ";");
                }
                else if (tag == "+V_I_D_B+") {
                    emit(js, indent, "var _e = _m('iframe');");
                    emit(js, indent, "_e.src = '" + c + "';");
                    emit(js, indent, "_e.width = 560;");
                    emit(js, indent, "_e.height = 315;");
                }
                else if (tag == "+I_F_R_M+") {
                    emit(js, indent, "var _e = _m('iframe');");
                    emit(js, indent, "_e.src = '" + c + "';");
                    emit(js, indent, "_e.width = " + num(fe, 0) + ";");
                    emit(js, indent, "_e.height = " + num(fe, 1) + ";");
                }
                // Data display
                else if (tag == "+T_B_L+") {
                    emit(js, indent, "var _e = _m('table'); _e.className = 'hsx-table'; _push(_e);");
                    indent++;
                }
                else if (tag == "+T_R_W+") {
                    emit(js, indent, "var _tr = document.createElement('tr'); _el.appendChild(_tr);");
                }
                else if (tag == "+T_C_L+") {
                    emit(js, indent, "var _td = _mt('td', '" + c + "');");
                }
                else if (tag == "+L_S_T+") {
                    emit(js, indent, "var _ul = _m('ul');");
                    auto items = splitList(fe);
                    for (auto& item : items) {
                        emit(js, indent, "var _li = document.createElement('li');");
                        emit(js, indent, "_li.textContent = '" + esc(item) + "';");
                        emit(js, indent, "_ul.appendChild(_li);");
                    }
                }
                else if (tag == "+O_L_S_T+") {
                    emit(js, indent, "var _ol = _m('ol');");
                    auto items = splitList(fe);
                    for (auto& item : items) {
                        emit(js, indent, "var _li = document.createElement('li');");
                        emit(js, indent, "_li.textContent = '" + esc(item) + "';");
                        emit(js, indent, "_ol.appendChild(_li);");
                    }
                }
                else if (tag == "+C_A_R_D+") {
                    emit(js, indent, "var _e = _m('div');");
                    emit(js, indent, "_e.className = 'hsx-card';");
                    if (!c.empty()) {
                        emit(js, indent, "var _ch = document.createElement('h3');");
                        emit(js, indent, "_ch.textContent = '" + c + "';");
                        emit(js, indent, "_e.appendChild(_ch);");
                    }
                    std::string body = fe->extraParams.size() > 0 ? esc(fe->extraParams[0]) : "";
                    if (!body.empty()) {
                        emit(js, indent, "var _cp = document.createElement('p');");
                        emit(js, indent, "_cp.textContent = '" + body + "';");
                        emit(js, indent, "_e.appendChild(_cp);");
                    }
                }
                else if (tag == "+B_A_D_G+") {
                    emit(js, indent, "var _e = _mt('span', '" + c + "');");
                    emit(js, indent, "_e.className = 'hsx-badge';");
                }
                else if (tag == "+A_L_R_T+")    emit(js, indent, "alert('" + c + "');");
                else if (tag == "+T_O_L_T+") {
                    emit(js, indent, "var _e = _mt('span', '" + c + "');");
                    emit(js, indent, "_e.title = '" + c + "';");
                    emit(js, indent, "_e.style.borderBottom = '1px dotted';");
                }
                else if (tag == "+A_C_C_R+") {
                    emit(js, indent, "var _e = _m('details');");
                    emit(js, indent, "_e.innerHTML = '<summary>" + c + "</summary><p>" + (fe->extraParams.size() > 0 ? esc(fe->extraParams[0]) : "") + "</p>';");
                }
                else if (tag == "+S_E_A_R_C_H+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'search';");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    emit(js, indent, "_e.placeholder = '" + c + "';");
                }
                // Forms
                else if (tag == "+F_R_M+") {
                    emit(js, indent, "var _e = _m('form'); _e.action = '" + c + "'; _push(_e);");
                    indent++;
                }
                else if (tag == "+L_B_L+") {
                    emit(js, indent, "var _e = _mt('label', '" + c + "');");
                }
                else if (tag == "+F_L_D+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.className = 'hsx-input';");
                    emit(js, indent, "_e.name = '" + c + "';");
                    std::string type = fe->extraParams.size() > 0 ? fe->extraParams[0] : "text";
                    emit(js, indent, "_e.type = '" + type + "';");
                }
                else if (tag == "+S_U_B_M+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'submit';");
                    emit(js, indent, "_e.className = 'hsx-btn';");
                    emit(js, indent, "_e.value = '" + c + "';");
                }
                else if (tag == "+R_S_E_T+") {
                    emit(js, indent, "var _e = _m('input');");
                    emit(js, indent, "_e.type = 'reset';");
                    emit(js, indent, "_e.className = 'hsx-btn';");
                    emit(js, indent, "_e.value = '" + c + "';");
                }
                // Navigation
                else if (tag == "+M_E_N_U+") {
                    emit(js, indent, "var _e = _m('nav'); _push(_e);");
                    indent++;
                }
                else if (tag == "+M_E_N_I+") {
                    emit(js, indent, "var _e = _m('a');");
                    emit(js, indent, "_e.textContent = '" + c + "';");
                    std::string href = fe->extraParams.size() > 0 ? fe->extraParams[0] : "#";
                    emit(js, indent, "_e.setAttribute('href', '" + href + "');");
                    emit(js, indent, "_e.setAttribute('data-hsx-link', 'true');");
                    emit(js, indent, "_e.className = 'hsx-link';");
                }
                // Layout helpers
                else if (tag == "+S_P_C_R+") {
                    emit(js, indent, "var _e = _m('div'); _e.style.height = '" + num(fe, 0) + "rem';");
                }
                else if (tag == "+D_I_V+") {
                    emit(js, indent, "var _e = _m('div');");
                    emit(js, indent, "if (_pendingClass) { _e.className = _pendingClass; _pendingClass = null; }");
                    emit(js, indent, "_push(_e);");
                    indent++;
                }
                else if (tag == "+S_E_C+") {
                    emit(js, indent, "var _e = _m('section'); _e.id = '" + c + "'; _push(_e);");
                    indent++;
                }
                else if (tag == "+H_D_R+") {
                    emit(js, indent, "var _e = _m('header'); _push(_e);");
                    indent++;
                }
                else if (tag == "+F_T_R+") {
                    emit(js, indent, "var _e = _m('footer'); _push(_e);");
                    indent++;
                }
                else if (tag == "+N_A_V+") {
                    emit(js, indent, "var _e = _m('nav'); _e.id = '" + c + "'; _push(_e);");
                    indent++;
                }
                else if (tag == "+M_A_I_N+") {
                    emit(js, indent, "var _e = _m('main'); _push(_e);");
                    indent++;
                }
                else if (tag == "+A_S_D_E+") {
                    emit(js, indent, "var _e = _m('aside'); _push(_e);");
                    indent++;
                }
                else if (tag == "+G_R_D+") {
                    emit(js, indent, "var _e = _m('div');");
                    emit(js, indent, "_e.className = 'hsx-grid';");
                    emit(js, indent, "_e.style.gridTemplateColumns = 'repeat(" + num(fe, 0) + ", 1fr)';");
                    emit(js, indent, "_push(_e);");
                    indent++;
                }
                else if (tag == "+F_L_X+") {
                    emit(js, indent, "var _e = _m('div');");
                    emit(js, indent, "_e.className = 'hsx-flex hsx-gap-2';");
                    emit(js, indent, "_push(_e);");
                    indent++;
                }
                // Closers
                else if (tag[0] == '+' && tag[1] == '/') {
                    indent = std::max(1, indent - 1);
                    emit(js, indent, "_pop();");
                }
                // State
                else if (tag == "+I_F_R_E_N_D+") {
                    emit(js, indent, "if (_vars['" + c + "'] == " + num(fe, 0) + ") {");
                    indent++;
                }
                else if (tag == "+L_O_O_P_R_N_D+") {
                    emit(js, indent, "(_vars['" + c + "'] || []).forEach(function(_item) {");
                    indent++;
                }
            }
            else if (auto* be = dynamic_cast<BackendNode*>(node.get())) {
                std::string cmd = be->command;
                if (cmd == "{A_P_I}") {
                    emit(js, indent, "fetch('" + esc(be->params) + "').then(function(r) { return r.json(); }).then(function(d) {");
                    emit(js, indent + 1, "var _pre = _m('pre');");
                    emit(js, indent + 1, "_pre.textContent = JSON.stringify(d, null, 2);");
                    emit(js, indent + 1, "_pre.className = 'hsx-font-mono hsx-bg-card hsx-p-2';");
                    emit(js, indent, "});");
                }
                else if (cmd == "{F_I_L}") {
                    emit(js, indent, "fetch('" + esc(be->params) + "').then(function(r) { return r.text(); }).then(function(d) {");
                    emit(js, indent + 1, "var _pre = _m('pre');");
                    emit(js, indent + 1, "_pre.textContent = d;");
                    emit(js, indent + 1, "_pre.className = 'hsx-font-mono hsx-bg-card hsx-p-2';");
                    emit(js, indent, "});");
                }
            }
            else if (auto* p = dynamic_cast<PrintNode*>(node.get())) {
                emit(js, indent, "var _pre = _mt('pre', '" + esc(p->value) + "');");
                emit(js, indent, "_pre.className = 'hsx-font-mono hsx-bg-card hsx-p-2';");
            }
            else if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
                if (a->type == AssignmentNode::ValType::NUMBER)
                    emit(js, indent, "_vars['" + esc(a->name) + "'] = " + std::to_string(std::get<double>(a->value)) + ";");
                else if (a->type == AssignmentNode::ValType::STRING)
                    emit(js, indent, "_vars['" + esc(a->name) + "'] = '" + esc(std::get<std::string>(a->value)) + "';");
            }
            else if (auto* l = dynamic_cast<LoopNode*>(node.get())) {
                emit(js, indent, "for (var _i = 0; _i < " + std::to_string(l->count) + "; _i++) {");
                indent++;
            }
            else if (auto* ie = dynamic_cast<IfElsePrintNode*>(node.get())) {
                emit(js, indent, "if (_vars['" + esc(ie->condVar) + "'] == " + std::to_string(ie->condValue) + ") {");
                emit(js, indent + 1, "var _pre = _mt('pre', '" + esc(ie->ifBranch) + "');");
                emit(js, indent + 1, "_pre.className = 'hsx-font-mono hsx-bg-card hsx-p-2';");
                emit(js, indent, "} else {");
                emit(js, indent + 1, "var _pre = _mt('pre', '" + esc(ie->elseBranch) + "');");
                emit(js, indent + 1, "_pre.className = 'hsx-font-mono hsx-bg-card hsx-p-2';");
                emit(js, indent, "}");
            }
        }

        while (indent > 1) {
            indent--;
            emit(js, indent, "_pop();");
        }

        js << "  if (window.__HSX_PAGE_INIT__) window.__HSX_PAGE_INIT__(_vars);\n";
        js << "})();\n";
        return js.str();
    }

private:
    static void emit(std::ostringstream& js, int indent, const std::string& line) {
        for (int i = 0; i < indent; i++) js << "  ";
        js << line << "\n";
    }

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

    static std::string num(const FrontendNode* fe, size_t idx) {
        if (idx < fe->extraNumbers.size()) return std::to_string(fe->extraNumbers[idx]);
        return "0";
    }

    static std::vector<std::string> splitList(const FrontendNode* fe) {
        std::vector<std::string> items;
        if (!fe->content.empty()) items.push_back(fe->content);
        for (auto& p : fe->extraParams) items.push_back(p);
        return items;
    }
};

} // namespace hsx
#endif