#ifndef HSX_OPTIMIZER_H
#define HSX_OPTIMIZER_H

#include "Parser.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>

namespace hsx {

class Optimizer {
public:
    static void optimize(std::vector<ASTPtr>& program) {
        std::unordered_map<std::string, double> constants;
        for (auto& node : program) {
            if (auto* a = dynamic_cast<AssignmentNode*>(node.get())) {
                if (a->type == AssignmentNode::ValType::NUMBER &&
                    std::holds_alternative<double>(a->value)) {
                    constants[a->name] = std::get<double>(a->value);
                }
            }
        }

        for (size_t i = 0; i < program.size(); ++i) {
            if (auto* lp = dynamic_cast<LoopNode*>(program[i].get())) {
                if (i + 1 >= program.size()) break;
                int count = lp->count;
                if (count <= 0) {
                    program.erase(program.begin() + i, program.begin() + i + 2);
                    i--;
                    continue;
                }
                if (count > 1000) continue;
                auto body = std::move(program[i+1]);
                program.erase(program.begin() + i, program.begin() + i + 2);
                for (int c = 0; c < count; ++c) {
                    program.insert(program.begin() + i, body->clone());
                }
                i += count - 1;
            }
        }

        for (auto& node : program) {
            if (auto* w = dynamic_cast<WhileLoopNode*>(node.get())) {
                if (constants.count(w->varName)) {
                    double constVal = constants[w->varName];
                    if (constVal != w->compareValue) {
                        w->varName = "__DEAD__";
                    }
                }
            }
        }
    }
};

} // namespace hsx
#endif