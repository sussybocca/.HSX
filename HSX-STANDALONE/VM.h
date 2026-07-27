#ifndef HSX_VM_H
#define HSX_VM_H
#include "Compiler.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

namespace hsx {

class VM {
    CompiledUnit& m_unit;
    std::vector<double> m_vars;
    std::vector<std::string> m_strVars;
    uint32_t m_ip = 0;
    int m_loopCount = 0;

public:
    VM(CompiledUnit& unit) : m_unit(unit), m_vars(unit.varNames.size(), 0.0), m_strVars(unit.varNames.size()) {}

    void execute() {
        static void* dispatch[] = {
            &&L_PRINT_STR, &&L_PRINT_VAR, &&L_PRINT_INPUT,
            &&L_SET_VAR_NUM, &&L_SET_VAR_STR,
            &&L_LOOP_START, &&L_LOOP_END,
            &&L_JMP_IF_EQ, &&L_JMP_IF_NEQ, &&L_JMP,
            &&L_FRONTEND, &&L_BACKEND, &&L_HALT
        };

        #define NEXT() { auto& i = m_unit.code[m_ip]; goto *dispatch[i.op]; }

        auto& code = m_unit.code;
        NEXT();

L_PRINT_STR: {
        auto& i = code[m_ip];
        printf("%.*s\n", m_unit.constStrs[i.a].len, m_unit.constStrs[i.a].data);
        m_ip++; NEXT();
    }
L_PRINT_VAR: {
        auto& i = code[m_ip];
        printf("%g\n", m_vars[i.a]);
        m_ip++; NEXT();
    }
L_PRINT_INPUT: {
        char buf[4096];
        if (fgets(buf, sizeof(buf), stdin)) {
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') buf[len-1] = 0;
            printf("%s\n", buf);
        }
        m_ip++; NEXT();
    }
L_SET_VAR_NUM: {
        auto& i = code[m_ip];
        if (i.a < m_vars.size()) m_vars[i.a] = m_unit.constNums[i.b];
        m_ip++; NEXT();
    }
L_SET_VAR_STR: {
        auto& i = code[m_ip];
        if (i.a < m_strVars.size()) {
            auto& s = m_unit.constStrs[i.b];
            m_strVars[i.a] = std::string(s.data, s.len);
        }
        m_ip++; NEXT();
    }
L_LOOP_START: {
        auto& i = code[m_ip];
        m_loopCount = (int)i.a;
        m_ip++; NEXT();
    }
L_LOOP_END: {
        auto& i = code[m_ip];
        if (--m_loopCount > 0) m_ip = i.a;
        else m_ip++;
        NEXT();
    }
L_JMP_IF_EQ: {
        auto& i = code[m_ip];
        if (i.a < m_vars.size() && m_vars[i.a] == m_unit.constNums[i.b])
            m_ip++;
        else
            m_ip = i.b;
        NEXT();
    }
L_JMP_IF_NEQ: {
        auto& i = code[m_ip];
        if (i.a < m_vars.size() && m_vars[i.a] != m_unit.constNums[i.b])
            m_ip++;
        else
            m_ip = i.b;
        NEXT();
    }
L_JMP: {
        auto& i = code[m_ip];
        m_ip = i.a;
        NEXT();
    }
L_FRONTEND: m_ip++; NEXT();
L_BACKEND: m_ip++; NEXT();
L_HALT: return;
    }
};

} // namespace hsx
#endif