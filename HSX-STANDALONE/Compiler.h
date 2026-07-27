#ifndef HSX_COMPILER_H
#define HSX_COMPILER_H
#include "Tokenizer.h"
#include <vector>
#include <iostream>

namespace hsx {

enum Op : uint8_t {
    OP_PRINT_STR, OP_PRINT_VAR, OP_PRINT_INPUT,
    OP_SET_VAR_NUM, OP_SET_VAR_STR,
    OP_LOOP_START, OP_LOOP_END,
    OP_JMP_IF_EQ, OP_JMP_IF_NEQ, OP_JMP,
    OP_FRONTEND, OP_BACKEND,
    OP_HALT
};

struct Instr {
    Op op;
    uint32_t a, b;
    Str str;
};

struct CompiledUnit {
    std::vector<Instr> code;
    std::vector<double> constNums;
    std::vector<Str> constStrs;
    std::vector<Str> varNames;

    void emit(Op op, uint32_t a = 0, uint32_t b = 0, Str s = {}) {
        code.push_back({op, a, b, s});
    }
    uint32_t addStr(Str s) { constStrs.push_back(s); return (uint32_t)constStrs.size() - 1; }
    uint32_t addNum(double d) { constNums.push_back(d); return (uint32_t)constNums.size() - 1; }
    uint32_t addVar(Str s) {
        for (uint32_t i = 0; i < varNames.size(); i++)
            if (varNames[i] == s) return i;
        varNames.push_back(s);
        return (uint32_t)varNames.size() - 1;
    }
};

class Compiler {
    Tokenizer m_tok;
    Arena& m_arena;
    CompiledUnit& m_unit;
    FToken m_current;

public:
    Compiler(const char* src, uint32_t len, Arena& arena, CompiledUnit& unit)
        : m_tok(src, len), m_arena(arena), m_unit(unit) { advance(); }

    bool compile() {
        while (m_current.kind != TK_EOF) {
            if (!compileLine()) return false;
        }
        m_unit.emit(OP_HALT);
        return true;
    }

private:
    void advance() { m_current = m_tok.next(m_arena); }

    void error(const char* msg) {
        std::cerr << "[HSX Compile Error] Line " << m_current.line 
                  << ": " << msg;
        if (m_current.value.data && m_current.value.len > 0) {
            std::cerr << " (got '";
            std::cerr.write(m_current.value.data, m_current.value.len);
            std::cerr << "')";
        }
        std::cerr << "\n";
    }

    const char* tokenName(TokenKind k) {
        switch (k) {
        case TK_HSX: return "HSX:";
        case TK_GT: return "{GT}";
        case TK_STAR: return "*";
        case TK_ID: return "identifier";
        case TK_NUM: return "number";
        case TK_STR: return "string";
        case TK_EQ: return "=";
        case TK_LP: return "!LP!";
        case TK_WHL: return "@WHL@";
        case TK_C_OPEN: return "C>";
        case TK_C_CLOSE: return ">C";
        case TK_F_OPEN: return "F>";
        case TK_F_CLOSE: return ">F";
        case TK_E_OPEN: return "E>";
        case TK_E_CLOSE: return ">E";
        case TK_PRINT: return "=Print";
        case TK_PLUS: return "+";
        case TK_CURLY: return "{}";
        case TK_SLASH: return "/";
        case TK_EOF: return "end of file";
        case TK_ERR: return "error token";
        default: return "unknown";
        }
    }

    bool expect(TokenKind k) {
        if (m_current.kind != k) {
            std::cerr << "[HSX Compile Error] Line " << m_current.line
                      << ": Expected " << tokenName(k)
                      << " but got " << tokenName(m_current.kind);
            if (m_current.value.data && m_current.value.len > 0) {
                std::cerr << " ('";
                std::cerr.write(m_current.value.data, m_current.value.len);
                std::cerr << "')";
            }
            std::cerr << "\n";
            return false;
        }
        advance();
        return true;
    }

    bool compileLine() {
        if (m_current.kind != TK_HSX) {
            error("Every line must start with HSX:");
            return false;
        }
        advance();
        switch (m_current.kind) {
        case TK_STAR: return compileAssignment();
        case TK_LP: return compileLoop();
        case TK_WHL: return compileWhile();
        case TK_C_OPEN: return compileCondition();
        case TK_PLUS: return compileFrontend();
        case TK_CURLY: return compileBackend();
        case TK_GT: advance(); m_unit.emit(OP_PRINT_INPUT); return true;
        case TK_SLASH: while (m_current.kind != TK_EOF && m_current.kind != TK_HSX) advance(); return true;
        default:
            error("Unknown command after HSX:");
            return false;
        }
    }

    bool compileAssignment() {
        advance();
        if (m_current.kind != TK_ID) {
            error("Expected variable name after *");
            return false;
        }
        uint32_t var = m_unit.addVar(m_current.value);
        advance();
        if (!expect(TK_EQ)) return false;
        if (m_current.kind == TK_NUM) {
            m_unit.emit(OP_SET_VAR_NUM, var, m_unit.addNum(m_current.value.toDouble()));
            advance();
        } else if (m_current.kind == TK_STR) {
            m_unit.emit(OP_SET_VAR_STR, var, m_unit.addStr(m_current.value));
            advance();
        } else {
            error("Expected number or string value after =");
            return false;
        }
        return expect(TK_STAR);
    }

    bool compileLoop() {
        advance();
        if (!expect(TK_EQ)) return false;
        if (m_current.kind != TK_NUM) {
            error("Expected number for loop count");
            return false;
        }
        int count = m_current.value.toInt();
        advance();
        uint32_t loopStart = (uint32_t)m_unit.code.size();
        m_unit.emit(OP_LOOP_START, count);
        if (!compileLine()) return false;
        m_unit.emit(OP_LOOP_END, loopStart);
        return true;
    }

    bool compileWhile() {
        advance();
        if (!expect(TK_STAR)) return false;
        if (m_current.kind != TK_ID) {
            error("Expected variable name in while condition");
            return false;
        }
        uint32_t var = m_unit.addVar(m_current.value);
        advance();
        if (!expect(TK_EQ)) return false;
        if (m_current.kind != TK_NUM) {
            error("Expected number in while condition");
            return false;
        }
        uint32_t numIdx = m_unit.addNum(m_current.value.toDouble());
        advance();
        if (!expect(TK_STAR)) return false;
        uint32_t jmpBack = (uint32_t)m_unit.code.size();
        uint32_t jmpIdx = (uint32_t)m_unit.code.size();
        m_unit.emit(OP_JMP_IF_NEQ, var, numIdx);
        if (!compileLine()) return false;
        m_unit.emit(OP_JMP, jmpBack);
        m_unit.code[jmpIdx].b = (uint32_t)m_unit.code.size();
        return true;
    }

    bool compileCondition() {
        advance();
        if (!expect(TK_STAR)) return false;
        if (m_current.kind != TK_ID) {
            error("Expected variable name in condition");
            return false;
        }
        uint32_t var = m_unit.addVar(m_current.value);
        advance();
        if (!expect(TK_EQ)) return false;
        if (m_current.kind != TK_NUM) {
            error("Expected number in condition");
            return false;
        }
        uint32_t numIdx = m_unit.addNum(m_current.value.toDouble());
        advance();
        if (!expect(TK_STAR) || !expect(TK_C_CLOSE) || !expect(TK_EQ)) return false;
        uint32_t jmpIfNot = (uint32_t)m_unit.code.size();
        m_unit.emit(OP_JMP_IF_NEQ, var, numIdx);
        if (!expect(TK_F_OPEN)) return false;
        // Read raw text until >F — NOT tokenized
        if (m_current.kind == TK_ID || m_current.kind == TK_STR || m_current.kind == TK_NUM) {
            Str content = m_current.value;
            advance();
            while (m_current.kind != TK_F_CLOSE && m_current.kind != TK_EOF) {
                advance();
            }
            m_unit.emit(OP_PRINT_STR, m_unit.addStr(content));
        }
        if (!expect(TK_F_CLOSE)) return false;
        uint32_t jmpEnd = (uint32_t)m_unit.code.size();
        m_unit.emit(OP_JMP, 0);
        m_unit.code[jmpIfNot].b = (uint32_t)m_unit.code.size();
        // Skip "error" keyword
        while (m_current.kind == TK_ID && m_current.value != Str("error")) advance();
        if (m_current.kind != TK_ID || m_current.value != Str("error")) {
            error("Expected 'error' keyword before else branch");
            return false;
        }
        advance();
        if (!expect(TK_E_OPEN)) return false;
        // Read raw text until >E
        if (m_current.kind == TK_ID || m_current.kind == TK_STR || m_current.kind == TK_NUM) {
            Str content = m_current.value;
            advance();
            while (m_current.kind != TK_E_CLOSE && m_current.kind != TK_EOF) {
                advance();
            }
            m_unit.emit(OP_PRINT_STR, m_unit.addStr(content));
        }
        if (!expect(TK_E_CLOSE) || !expect(TK_PRINT)) return false;
        m_unit.code[jmpEnd].b = (uint32_t)m_unit.code.size();
        return true;
    }

    bool compileFrontend() {
        while (m_current.kind != TK_EOF && m_current.kind != TK_HSX) advance();
        return true;
    }
    bool compileBackend() {
        while (m_current.kind != TK_EOF && m_current.kind != TK_HSX) advance();
        return true;
    }
};

} // namespace hsx
#endif