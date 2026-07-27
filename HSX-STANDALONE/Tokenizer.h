#ifndef HSX_TOKENIZER_H
#define HSX_TOKENIZER_H
#include "Core.h"
#include <cstdint>

namespace hsx {

enum TokenKind : uint8_t {
    TK_HSX, TK_GT, TK_STAR, TK_ID, TK_NUM, TK_STR, TK_EQ,
    TK_LP, TK_WHL, TK_C_OPEN, TK_C_CLOSE, TK_F_OPEN, TK_F_CLOSE,
    TK_E_OPEN, TK_E_CLOSE, TK_PRINT, TK_PLUS, TK_CURLY, TK_SLASH,
    TK_FNC, TK_RET, TK_EOF, TK_ERR
};

struct FToken {
    TokenKind kind;
    Str value;
    uint16_t line;
};

class Tokenizer {
    const char* m_src;
    uint32_t m_pos;
    uint32_t m_len;
    uint16_t m_line;
public:
    Tokenizer(const char* src, uint32_t len) : m_src(src), m_pos(0), m_len(len), m_line(1) {}

    FToken next(Arena& arena) {
        skipWS();
        if (m_pos >= m_len) return {TK_EOF, {}, m_line};
        char c = m_src[m_pos];

        // Check ALL multi-char tokens BEFORE identifier check
        // HSX:
        if (c == 'H' && m_pos+3 < m_len && m_src[m_pos+1] == 'S' && m_src[m_pos+2] == 'X' && m_src[m_pos+3] == ':') {
            m_pos += 4;
            return {TK_HSX, {}, m_line};
        }
        // C>
        if (c == 'C' && m_pos+1 < m_len && m_src[m_pos+1] == '>') {
            m_pos += 2;
            return {TK_C_OPEN, {}, m_line};
        }
        // F>
        if (c == 'F' && m_pos+1 < m_len && m_src[m_pos+1] == '>') {
            m_pos += 2;
            return {TK_F_OPEN, {}, m_line};
        }
        // E>
        if (c == 'E' && m_pos+1 < m_len && m_src[m_pos+1] == '>') {
            m_pos += 2;
            return {TK_E_OPEN, {}, m_line};
        }
        // >C, >F, >E
        if (c == '>' && m_pos+1 < m_len) {
            if (m_src[m_pos+1] == 'C') { m_pos += 2; return {TK_C_CLOSE, {}, m_line}; }
            if (m_src[m_pos+1] == 'F') { m_pos += 2; return {TK_F_CLOSE, {}, m_line}; }
            if (m_src[m_pos+1] == 'E') { m_pos += 2; return {TK_E_CLOSE, {}, m_line}; }
            m_pos++;
            return {TK_ERR, {}, m_line};
        }

        // Numbers
        if ((c >= '0' && c <= '9') || c == '.') {
            uint32_t start = m_pos; bool hasDot = false;
            while (m_pos < m_len && ((m_src[m_pos] >= '0' && m_src[m_pos] <= '9') || m_src[m_pos] == '.')) {
                if (m_src[m_pos] == '.') hasDot = true;
                m_pos++;
            }
            if (!hasDot) return {TK_ERR, Str("Number missing decimal", 20), m_line};
            return {TK_NUM, Str(m_src + start, m_pos - start), m_line};
        }

        // Strings
        if (c == '"' && m_pos+1 < m_len && m_src[m_pos+1] == '"') {
            m_pos += 2; uint32_t start = m_pos;
            while (m_pos+1 < m_len && !(m_src[m_pos] == '"' && m_src[m_pos+1] == '"')) m_pos++;
            FToken t = {TK_STR, Str(m_src + start, m_pos - start), m_line};
            if (m_pos < m_len) m_pos += 2;
            return t;
        }

        // Identifiers (only AFTER all multi-char checks failed)
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            uint32_t start = m_pos;
            while (m_pos < m_len && ((m_src[m_pos] >= 'a' && m_src[m_pos] <= 'z') ||
                   (m_src[m_pos] >= 'A' && m_src[m_pos] <= 'Z') ||
                   (m_src[m_pos] >= '0' && m_src[m_pos] <= '9') || m_src[m_pos] == '_' || m_src[m_pos] == '.'))
                m_pos++;
            return {TK_ID, Str(m_src + start, m_pos - start), m_line};
        }

        // Single-char tokens
        m_pos++;
        switch (c) {
        case '*': return {TK_STAR, {}, m_line};
        case '=': {
            if (m_pos+4 < m_len && m_src[m_pos] == 'P' && m_src[m_pos+1] == 'r' && 
                m_src[m_pos+2] == 'i' && m_src[m_pos+3] == 'n' && m_src[m_pos+4] == 't')
                { m_pos += 5; return {TK_PRINT, {}, m_line}; }
            return {TK_EQ, {}, m_line};
        }
        case '{': return {TK_CURLY, {}, m_line};
        case '}': return {TK_CURLY, {}, m_line};
        case '+': return {TK_PLUS, {}, m_line};
        case '/': return {TK_SLASH, {}, m_line};
        case '!': {
            if (m_pos+2 < m_len && m_src[m_pos] == 'L' && m_src[m_pos+1] == 'P' && m_src[m_pos+2] == '!')
                { m_pos += 3; return {TK_LP, {}, m_line}; }
            return {TK_ERR, {}, m_line};
        }
        case '@': {
            if (m_pos+3 < m_len && m_src[m_pos] == 'W' && m_src[m_pos+1] == 'H' && m_src[m_pos+2] == 'L' && m_src[m_pos+3] == '@')
                { m_pos += 4; return {TK_WHL, {}, m_line}; }
            return {TK_ERR, {}, m_line};
        }
        default: return {TK_ERR, {}, m_line};
        }
    }

private:
    void skipWS() {
        while (m_pos < m_len) {
            char c = m_src[m_pos];
            if (c == ' ' || c == '\t' || c == '\r') m_pos++;
            else if (c == '\n') { m_pos++; m_line++; }
            else if (c == '/' && m_pos+1 < m_len && m_src[m_pos+1] == '/') {
                while (m_pos < m_len && m_src[m_pos] != '\n') m_pos++;
            }
            else break;
        }
    }
};

} // namespace hsx
#endif