#ifndef HSX_BACKEND_AST_H
#define HSX_BACKEND_AST_H

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>

namespace hsx {

enum class BackendCmd {
    SRV, DBC, QRY, TBL, INSERT, FIND, UPDATE, DELETE_ROW,
    AUTH, LOGIN, SIGNUP, LOGOUT, SESSION, MIDDLEWARE,
    GET, POST, PUT, DEL, RES, JSON, STATUS, HEADER,
    FILE_READ, FILE_WRITE, DEL_FILE, MKDIR, LIST_DIR,
    EMAIL, SUBJECT, BODY,
    CACHE, SET, GET_KEY, EXPIRE, DEL_KEY,
    WS, EMIT, ON, BCAST,
    ROUTE_GROUP, MIDDLEWARE_APPLY,
    TRY, THROW, ON_ERROR, VALID, END_TRY,
    IMPORT, MODULE_REF,
    CRON
};

struct BackendStmt {
    BackendCmd cmd;
    std::vector<std::string> stringArgs;
    std::vector<double> numberArgs;
    std::vector<BackendStmt> children;
    bool hasCondition = false;
    std::string condVar;
    double condValue = 0.0;
    std::vector<BackendStmt> ifBranch;
    std::vector<BackendStmt> elseBranch;
};

struct BackendProgram {
    std::vector<BackendStmt> statements;
    std::vector<std::string> imports;
    std::unordered_map<std::string, std::vector<BackendStmt>> errorHandlers;
};

} // namespace hsx
#endif