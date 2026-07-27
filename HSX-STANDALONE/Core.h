#ifndef HSX_CORE_H
#define HSX_CORE_H
#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace hsx {

class Arena {
    char* m_buf;
    size_t m_pos = 0;
    size_t m_cap;
public:
    Arena(size_t cap = 1024*1024) : m_cap(cap) { m_buf = (char*)malloc(cap); }
    ~Arena() { free(m_buf); }
    void* alloc(size_t sz) {
        sz = (sz + 7) & ~7;
        if (m_pos + sz > m_cap) { m_cap *= 2; m_buf = (char*)realloc(m_buf, m_cap); }
        void* p = m_buf + m_pos; m_pos += sz; return p;
    }
    void reset() { m_pos = 0; }
    size_t used() const { return m_pos; }
};

struct Str {
    const char* data;
    uint32_t len;
    Str() : data(nullptr), len(0) {}
    Str(const char* s, uint32_t l) : data(s), len(l) {}
    Str(const char* s) : data(s), len((uint32_t)strlen(s)) {}
    bool operator==(Str o) const {
        return len == o.len && (len == 0 || memcmp(data, o.data, len) == 0);
    }
    bool operator==(const char* s) const { return *this == Str(s); }
    bool operator!=(Str o) const { return !(*this == o); }
    bool operator!=(const char* s) const { return !(*this == Str(s)); }
    double toDouble() const {
        char buf[64]; uint32_t n = len < 63 ? len : 63;
        memcpy(buf, data, n); buf[n] = 0;
        return strtod(buf, nullptr);
    }
    int toInt() const { return (int)toDouble(); }
};

inline uint32_t hashStr(Str s) {
    uint32_t h = 0x811c9dc5;
    for (uint32_t i = 0; i < s.len; i++) {
        h ^= (uint8_t)s.data[i]; h *= 0x01000193;
    }
    return h;
}

} // namespace hsx
#endif