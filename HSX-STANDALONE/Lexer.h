#ifndef HSX_LEXER_H
#define HSX_LEXER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <limits>

namespace hsx {

enum class TokenType {
    HSX,
    GT,
    STAR,
    IDENTIFIER,
    EQUALS,
    NUMBER,
    STRING,
    EXCL_LP_EXCL,
    AT_WHL_AT,
    C_OPEN,
    C_CLOSE,
    F_OPEN,
    F_CLOSE,
    E_OPEN,
    E_CLOSE,
    EQUALSPRINT,
    PLUS_FE,
    PLUS_FE_CLOSE,
    CURLY_BE,
    CURLY_BE_GT,
    F_N_C_OPEN,
    F_N_C_CLOSE,
    RETURN,
    SLASH_C_O_M_SLASH,
    COMMENT_TEXT,
    ERROR_TOKEN,
    END_OF_LINE
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    explicit Lexer(const std::string& line) : input(line), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < input.size()) {
            skipWhitespace();
            if (pos >= input.size()) break;

            if (input.compare(pos, 4, "HSX:") == 0) {
                tokens.push_back({TokenType::HSX, "HSX:"});
                pos += 4;
                continue;
            }

            if (input.compare(pos, 4, "{GT}") == 0) {
                tokens.push_back({TokenType::GT, "{GT}"});
                pos += 4;
                continue;
            }

            if (input.compare(pos, 4, "!LP!") == 0) {
                tokens.push_back({TokenType::EXCL_LP_EXCL, "!LP!"});
                pos += 4;
                continue;
            }

            if (input.compare(pos, 5, "@WHL@") == 0) {
                tokens.push_back({TokenType::AT_WHL_AT, "@WHL@"});
                pos += 5;
                continue;
            }

            if (input.compare(pos, 6, "=Print") == 0) {
                tokens.push_back({TokenType::EQUALSPRINT, "=Print"});
                pos += 6;
                continue;
            }

            if (input.compare(pos, 2, "C>") == 0) {
                tokens.push_back({TokenType::C_OPEN, "C>"});
                pos += 2;
                continue;
            }
            if (input.compare(pos, 2, ">C") == 0) {
                tokens.push_back({TokenType::C_CLOSE, ">C"});
                pos += 2;
                continue;
            }
            if (input.compare(pos, 2, "F>") == 0) {
                tokens.push_back({TokenType::F_OPEN, "F>"});
                pos += 2;
                continue;
            }
            if (input.compare(pos, 2, ">F") == 0) {
                tokens.push_back({TokenType::F_CLOSE, ">F"});
                pos += 2;
                continue;
            }
            if (input.compare(pos, 2, "E>") == 0) {
                tokens.push_back({TokenType::E_OPEN, "E>"});
                pos += 2;
                continue;
            }
            if (input.compare(pos, 2, ">E") == 0) {
                tokens.push_back({TokenType::E_CLOSE, ">E"});
                pos += 2;
                continue;
            }

            if (input.compare(pos, 6, "F_N_C>") == 0) {
                tokens.push_back({TokenType::F_N_C_OPEN, "F_N_C>"});
                pos += 6;
                continue;
            }
            if (input.compare(pos, 6, ">F_N_C") == 0) {
                tokens.push_back({TokenType::F_N_C_CLOSE, ">F_N_C"});
                pos += 6;
                continue;
            }

            if (input.compare(pos, 7, "{R_T_N}") == 0) {
                tokens.push_back({TokenType::RETURN, "{R_T_N}"});
                pos += 7;
                continue;
            }

            if (input.compare(pos, 7, "/C_O_M/") == 0) {
                tokens.push_back({TokenType::SLASH_C_O_M_SLASH, "/C_O_M/"});
                pos += 7;
                skipWhitespace();
                if (pos < input.size() && input[pos] == '"') {
                    tokens.push_back({TokenType::STRING, readDoubleQuoteString()});
                }
                skipWhitespace();
                if (pos < input.size() && input[pos] == '/') {
                    pos++;
                }
                continue;
            }

            if (pos < input.size() && input[pos] == '+') {
                size_t start = pos;
                pos++;
                while (pos < input.size() && input[pos] != '+') pos++;
                if (pos < input.size()) pos++;
                std::string fe = input.substr(start, pos - start);
                tokens.push_back({TokenType::PLUS_FE, fe});
                continue;
            }

            if (pos < input.size() && input[pos] == '{') {
                size_t start = pos;
                pos++;
                while (pos < input.size() && input[pos] != '}') pos++;
                if (pos < input.size()) pos++;
                std::string be = input.substr(start, pos - start);
                tokens.push_back({TokenType::CURLY_BE, be});
                continue;
            }

            if (input[pos] == '*') {
                tokens.push_back({TokenType::STAR, "*"});
                pos++;
                continue;
            }

            if (input[pos] == '=') {
                tokens.push_back({TokenType::EQUALS, "="});
                pos++;
                continue;
            }

            if (std::isdigit(input[pos]) || input[pos] == '.') {
                std::string num;
                while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.')) {
                    num += input[pos++];
                }
                if (num.find('.') == std::string::npos) {
                    throw std::runtime_error("Number missing decimal: " + num);
                }
                tokens.push_back({TokenType::NUMBER, num});
                continue;
            }

            if (input[pos] == '"' && pos+1 < input.size() && input[pos+1] == '"') {
                tokens.push_back({TokenType::STRING, readDoubleQuoteString()});
                continue;
            }

            if (std::isalpha(input[pos]) || input[pos] == '_') {
                std::string id;
                while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '.' || input[pos] == '_')) {
                    id += input[pos++];
                }
                tokens.push_back({TokenType::IDENTIFIER, id});
                continue;
            }

            tokens.push_back({TokenType::ERROR_TOKEN, std::string(1, input[pos])});
            pos++;
        }
        tokens.push_back({TokenType::END_OF_LINE, ""});
        return tokens;
    }

private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(input[pos])) pos++;
    }

    std::string readDoubleQuoteString() {
        if (pos+1 >= input.size() || input[pos] != '"' || input[pos+1] != '"')
            throw std::runtime_error("Malformed double-quote string");
        pos += 2;
        std::string content;
        while (pos+1 < input.size()) {
            if (input[pos] == '"' && input[pos+1] == '"') {
                pos += 2;
                return content;
            }
            content += input[pos++];
        }
        throw std::runtime_error("Unterminated double-quote string");
    }
};

} // namespace hsx
#endif