#ifndef HSX_REGISTER_H
#define HSX_REGISTER_H
#include <windows.h>
#include <string>

void RegisterHSXFileAssociation() {
    HKEY hKey;
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exe(exePath);
    std::string editorExe = exe;
    size_t lastSlash = editorExe.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        editorExe = editorExe.substr(0, lastSlash) + "\\HSXEditor.exe";
    }

    auto setKey = [&](const char* path, const char* value) {
        if (RegCreateKeyExA(HKEY_CURRENT_USER, path, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueA(hKey, NULL, REG_SZ, value, (DWORD)strlen(value));
            RegCloseKey(hKey);
        }
    };

    setKey("Software\\Classes\\.hsx", "HSXEditor.File");
    setKey("Software\\Classes\\HSXEditor.File", "HSX Source File");
    setKey("Software\\Classes\\HSXEditor.File\\DefaultIcon", (editorExe + ",0").c_str());
    setKey("Software\\Classes\\HSXEditor.File\\shell\\open\\command", ("\"" + editorExe + "\" \"%1\"").c_str());
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}
#endif