#ifndef HSX_PARSER_H
#define HSX_PARSER_H

#include "Lexer.h"
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <variant>

namespace hsx {

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual std::unique_ptr<ASTNode> clone() const = 0;
};

struct PrintNode : ASTNode {
    std::string value;
    explicit PrintNode(const std::string& v) : value(v) {}
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<PrintNode>(*this); }
};

struct AssignmentNode : ASTNode {
    std::string name;
    std::variant<double, std::string, std::vector<double>> value;
    enum class ValType { NUMBER, STRING, LIST } type;
    std::unique_ptr<ASTNode> clone() const override {
        auto c = std::make_unique<AssignmentNode>();
        c->name = name; c->value = value; c->type = type;
        return c;
    }
};

struct LoopNode : ASTNode {
    int count;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<LoopNode>(*this); }
};

struct WhileLoopNode : ASTNode {
    std::string varName;
    double compareValue;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<WhileLoopNode>(*this); }
};

struct IfElsePrintNode : ASTNode {
    std::string condVar;
    double condValue;
    std::string ifBranch;
    std::string elseBranch;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<IfElsePrintNode>(*this); }
};

struct FrontendNode : ASTNode {
    std::string tag;
    std::string content;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<FrontendNode>(*this); }
};

struct BackendNode : ASTNode {
    std::string command;
    std::string params;
    bool hasNumber = false;
    double numberVal = 0.0;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<BackendNode>(*this); }
};

struct FunctionDefNode : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<FunctionDefNode>(*this); }
};

struct FunctionCallNode : ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<FunctionCallNode>(*this); }
};

struct ReturnNode : ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<ReturnNode>(*this); }
};

struct CommentNode : ASTNode {
    std::unique_ptr<ASTNode> clone() const override { return std::make_unique<CommentNode>(); }
};

using ASTPtr = std::unique_ptr<ASTNode>;

class Parser {
public:
    static ASTPtr parseLine(const std::string& line) {
        Lexer lexer(line);
        auto tokens = lexer.tokenize();
        if (tokens.empty() || tokens[0].type != TokenType::HSX)
            throw std::runtime_error("Line must start with HSX:");
        size_t idx = 1;
        return parseCommand(tokens, idx);
    }

private:
    static ASTPtr parseCommand(const std::vector<Token>& tokens, size_t& i) {
        if (i >= tokens.size()) return nullptr;

        if (tokens[i].type == TokenType::GT) {
            i++;
            return std::make_unique<PrintNode>("__INPUT__");
        }

        if (tokens[i].type == TokenType::STAR) {
            return parseValueAssignment(tokens, i);
        }

        if (tokens[i].type == TokenType::EXCL_LP_EXCL) {
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::EQUALS)
                throw std::runtime_error("Expected = after !LP!");
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::NUMBER)
                throw std::runtime_error("Expected decimal number for loop count");
            double count = std::stod(tokens[i].value);
            if (count != static_cast<int>(count)) throw std::runtime_error("Loop count must be whole number");
            i++;
            auto node = std::make_unique<LoopNode>();
            node->count = static_cast<int>(count);
            return node;
        }

        if (tokens[i].type == TokenType::AT_WHL_AT) {
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
                throw std::runtime_error("Expected * after @WHL@");
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER)
                throw std::runtime_error("Expected variable name in while condition");
            std::string var = tokens[i].value;
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::EQUALS)
                throw std::runtime_error("Expected = in while condition");
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::NUMBER)
                throw std::runtime_error("Expected number in while condition");
            double val = std::stod(tokens[i].value);
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
                throw std::runtime_error("Expected closing *");
            i++;
            auto node = std::make_unique<WhileLoopNode>();
            node->varName = var;
            node->compareValue = val;
            return node;
        }

        if (tokens[i].type == TokenType::C_OPEN) {
            return parseIfElse(tokens, i);
        }

        if (tokens[i].type == TokenType::F_N_C_OPEN) {
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER)
                throw std::runtime_error("Expected function name");
            std::string fname = tokens[i].value;
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::F_N_C_CLOSE)
                throw std::runtime_error("Expected >F_N_C");
            i++;
            auto node = std::make_unique<FunctionDefNode>();
            node->name = fname;
            return node;
        }

        if (tokens[i].type == TokenType::RETURN) {
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
                throw std::runtime_error("Expected * after return");
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER)
                throw std::runtime_error("Expected variable name");
            std::string vname = tokens[i].value;
            i++;
            if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
                throw std::runtime_error("Expected closing *");
            i++;
            auto node = std::make_unique<ReturnNode>();
            node->varName = vname;
            return node;
        }

        if (tokens[i].type == TokenType::PLUS_FE) {
            return parseFrontend(tokens, i);
        }

        if (tokens[i].type == TokenType::CURLY_BE) {
            return parseBackend(tokens, i);
        }

        if (tokens[i].type == TokenType::SLASH_C_O_M_SLASH) {
            i++;
            return std::make_unique<CommentNode>();
        }

        throw std::runtime_error("Unexpected token: " + tokens[i].value);
    }

    static ASTPtr parseValueAssignment(const std::vector<Token>& tokens, size_t& i) {
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected identifier after *");
        std::string name = tokens[i].value;
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::EQUALS)
            throw std::runtime_error("Expected =");
        i++;
        auto node = std::make_unique<AssignmentNode>();
        node->name = name;

        if (i >= tokens.size()) throw std::runtime_error("Unexpected end");
        if (tokens[i].type == TokenType::NUMBER) {
            node->value = std::stod(tokens[i].value);
            node->type = AssignmentNode::ValType::NUMBER;
            i++;
        } else if (tokens[i].type == TokenType::STRING) {
            node->value = tokens[i].value;
            node->type = AssignmentNode::ValType::STRING;
            i++;
        } else if (tokens[i].value == "[") {
            i++;
            std::vector<double> list;
            while (i < tokens.size() && tokens[i].type == TokenType::NUMBER) {
                list.push_back(std::stod(tokens[i].value));
                i++;
                if (i < tokens.size() && tokens[i].value == ",") i++;
            }
            if (i >= tokens.size() || tokens[i].value != "]")
                throw std::runtime_error("Expected ]");
            i++;
            node->value = list;
            node->type = AssignmentNode::ValType::LIST;
        } else {
            throw std::runtime_error("Expected value after =");
        }

        if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
            throw std::runtime_error("Expected closing *");
        i++;
        return node;
    }

    static ASTPtr parseIfElse(const std::vector<Token>& tokens, size_t& i) {
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
            throw std::runtime_error("Expected * in condition");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected variable");
        std::string var = tokens[i].value;
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::EQUALS)
            throw std::runtime_error("Expected =");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::NUMBER)
            throw std::runtime_error("Expected number");
        double val = std::stod(tokens[i].value);
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::STAR)
            throw std::runtime_error("Expected *");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::C_CLOSE)
            throw std::runtime_error("Expected >C");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::EQUALS)
            throw std::runtime_error("Expected = after >C");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::F_OPEN)
            throw std::runtime_error("Expected F>");
        i++;
        std::string ifContent;
        while (i < tokens.size() && tokens[i].type != TokenType::F_CLOSE) {
            ifContent += tokens[i].value + " ";
            i++;
        }
        if (i >= tokens.size()) throw std::runtime_error("Expected >F");
        i++;
        if (i >= tokens.size() || tokens[i].value != "error")
            throw std::runtime_error("Expected 'error'");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::E_OPEN)
            throw std::runtime_error("Expected E>");
        i++;
        std::string elseContent;
        while (i < tokens.size() && tokens[i].type != TokenType::E_CLOSE) {
            elseContent += tokens[i].value + " ";
            i++;
        }
        if (i >= tokens.size()) throw std::runtime_error("Expected >E");
        i++;
        if (i >= tokens.size() || tokens[i].type != TokenType::EQUALSPRINT)
            throw std::runtime_error("Expected =Print");
        i++;

        auto node = std::make_unique<IfElsePrintNode>();
        node->condVar = var;
        node->condValue = val;
        if (!ifContent.empty()) ifContent.pop_back();
        if (!elseContent.empty()) elseContent.pop_back();
        node->ifBranch = ifContent;
        node->elseBranch = elseContent;
        return node;
    }

    static ASTPtr parseFrontend(const std::vector<Token>& tokens, size_t& i) {
        std::string tag = tokens[i].value;
        i++;
        std::string content;
        if (i < tokens.size() && tokens[i].type == TokenType::STRING) {
            content = tokens[i].value;
            i++;
        }
        auto node = std::make_unique<FrontendNode>();
        node->tag = tag;
        node->content = content;
        return node;
    }

    static ASTPtr parseBackend(const std::vector<Token>& tokens, size_t& i) {
        std::string cmd = tokens[i].value;
        i++;
        auto node = std::make_unique<BackendNode>();
        node->command = cmd;
        if (i < tokens.size()) {
            if (tokens[i].type == TokenType::STRING) {
                node->params = tokens[i].value;
                i++;
            } else if (tokens[i].type == TokenType::NUMBER) {
                node->hasNumber = true;
                node->numberVal = std::stod(tokens[i].value);
                i++;
            } else if (tokens[i].type == TokenType::EQUALS) {
                i++;
                if (i < tokens.size() && tokens[i].type == TokenType::NUMBER) {
                    node->hasNumber = true;
                    node->numberVal = std::stod(tokens[i].value);
                    i++;
                }
            }
        }
        return node;
    }
};

} // namespace hsx
#endif