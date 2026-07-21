#ifndef HSX_STANDARDLIBRARY_H
#define HSX_STANDARDLIBRARY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <variant>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace hsx {

using LibValue = std::variant<double, std::string, std::vector<double>>;

inline std::string libValueToString(const LibValue& v) {
    std::ostringstream ss;
    if (std::holds_alternative<double>(v)) {
        ss << std::get<double>(v);
    } else if (std::holds_alternative<std::string>(v)) {
        ss << std::get<std::string>(v);
    } else if (std::holds_alternative<std::vector<double>>(v)) {
        auto& vec = std::get<std::vector<double>>(v);
        ss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i) ss << ", ";
            ss << vec[i];
        }
        ss << "]";
    }
    return ss.str();
}

class StandardLibrary {
public:
    using BuiltinFunc = std::function<LibValue(const std::vector<LibValue>&)>;

    static std::unordered_map<std::string, BuiltinFunc>& getFunctions() {
        static std::unordered_map<std::string, BuiltinFunc> funcs;
        if (funcs.empty()) {
            funcs["MATH.ADD"] = [](const std::vector<LibValue>& args) -> LibValue {
                double sum = 0;
                for (auto& a : args) sum += std::get<double>(a);
                return sum;
            };
            funcs["MATH.SUB"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("SUB requires 2 args");
                return std::get<double>(args[0]) - std::get<double>(args[1]);
            };
            funcs["MATH.MUL"] = [](const std::vector<LibValue>& args) -> LibValue {
                double prod = 1;
                for (auto& a : args) prod *= std::get<double>(a);
                return prod;
            };
            funcs["MATH.DIV"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("DIV requires 2 args");
                double b = std::get<double>(args[1]);
                if (b == 0) throw std::runtime_error("Division by zero");
                return std::get<double>(args[0]) / b;
            };
            funcs["MATH.SQRT"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("SQRT requires 1 arg");
                return std::sqrt(std::get<double>(args[0]));
            };
            funcs["MATH.POW"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("POW requires 2 args");
                return std::pow(std::get<double>(args[0]), std::get<double>(args[1]));
            };
            funcs["MATH.ABS"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("ABS requires 1 arg");
                return std::abs(std::get<double>(args[0]));
            };
            funcs["MATH.MOD"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("MOD requires 2 args");
                return std::fmod(std::get<double>(args[0]), std::get<double>(args[1]));
            };

            funcs["STR.CONCAT"] = [](const std::vector<LibValue>& args) -> LibValue {
                std::string result;
                for (auto& a : args) result += std::get<std::string>(a);
                return result;
            };
            funcs["STR.UPPER"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("UPPER requires 1 arg");
                std::string s = std::get<std::string>(args[0]);
                for (char& c : s) c = static_cast<char>(std::toupper(c));
                return s;
            };
            funcs["STR.LOWER"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("LOWER requires 1 arg");
                std::string s = std::get<std::string>(args[0]);
                for (char& c : s) c = static_cast<char>(std::tolower(c));
                return s;
            };
            funcs["STR.LEN"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("LEN requires 1 arg");
                return static_cast<double>(std::get<std::string>(args[0]).size());
            };
            funcs["STR.SPLIT"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("SPLIT requires string and delimiter");
                std::string s = std::get<std::string>(args[0]);
                std::string delim = std::get<std::string>(args[1]);
                std::vector<double> result;
                size_t pos = 0;
                while ((pos = s.find(delim)) != std::string::npos) {
                    result.push_back(std::stod(s.substr(0, pos)));
                    s.erase(0, pos + delim.length());
                }
                if (!s.empty()) result.push_back(std::stod(s));
                return result;
            };
            funcs["STR.REPLACE"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 3) throw std::runtime_error("REPLACE requires str, old, new");
                std::string s = std::get<std::string>(args[0]);
                std::string oldStr = std::get<std::string>(args[1]);
                std::string newStr = std::get<std::string>(args[2]);
                size_t pos = 0;
                while ((pos = s.find(oldStr, pos)) != std::string::npos) {
                    s.replace(pos, oldStr.length(), newStr);
                    pos += newStr.length();
                }
                return s;
            };

            funcs["LIST.APPEND"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("APPEND requires list and value");
                auto list = std::get<std::vector<double>>(args[0]);
                list.push_back(std::get<double>(args[1]));
                return list;
            };
            funcs["LIST.SUM"] = [](const std::vector<LibValue>& args) -> LibValue {
                auto list = std::get<std::vector<double>>(args[0]);
                double s = 0;
                for (double v : list) s += v;
                return s;
            };
            funcs["LIST.LEN"] = [](const std::vector<LibValue>& args) -> LibValue {
                return static_cast<double>(std::get<std::vector<double>>(args[0]).size());
            };
            funcs["LIST.MAX"] = [](const std::vector<LibValue>& args) -> LibValue {
                auto list = std::get<std::vector<double>>(args[0]);
                if (list.empty()) throw std::runtime_error("MAX on empty list");
                return *std::max_element(list.begin(), list.end());
            };
            funcs["LIST.MIN"] = [](const std::vector<LibValue>& args) -> LibValue {
                auto list = std::get<std::vector<double>>(args[0]);
                if (list.empty()) throw std::runtime_error("MIN on empty list");
                return *std::min_element(list.begin(), list.end());
            };
            funcs["LIST.SORT"] = [](const std::vector<LibValue>& args) -> LibValue {
                auto list = std::get<std::vector<double>>(args[0]);
                std::sort(list.begin(), list.end());
                return list;
            };
            funcs["LIST.REVERSE"] = [](const std::vector<LibValue>& args) -> LibValue {
                auto list = std::get<std::vector<double>>(args[0]);
                std::reverse(list.begin(), list.end());
                return list;
            };
            funcs["LIST.GET"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 2) throw std::runtime_error("GET requires list and index");
                auto list = std::get<std::vector<double>>(args[0]);
                int idx = static_cast<int>(std::get<double>(args[1]));
                if (idx < 0 || idx >= static_cast<int>(list.size()))
                    throw std::runtime_error("Index out of bounds");
                return list[idx];
            };

            funcs["CONV.TOSTRING"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("TOSTRING requires 1 arg");
                return libValueToString(args[0]);
            };
            funcs["CONV.TONUMBER"] = [](const std::vector<LibValue>& args) -> LibValue {
                if (args.size() != 1) throw std::runtime_error("TONUMBER requires 1 arg");
                return std::stod(std::get<std::string>(args[0]));
            };
        }
        return funcs;
    }

    static LibValue call(const std::string& name, const std::vector<LibValue>& args) {
        auto& funcs = getFunctions();
        auto it = funcs.find(name);
        if (it == funcs.end()) throw std::runtime_error("Unknown built-in function: " + name);
        return it->second(args);
    }
};

} // namespace hsx
#endif