#ifndef HSX_PROJECT_H
#define HSX_PROJECT_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace hsx {

struct PageFile {
    std::string route;
    std::string sourcePath;
    std::string source;
};

struct ProjectConfig {
    std::string appSource;
    std::string routerLib;
    std::string linksSource;
    std::vector<PageFile> pages;
};

class HSXProject {
public:
    static ProjectConfig load(const std::string& projectDir) {
        ProjectConfig config;
        std::string hsxRoot = projectDir + "/H.Source/_.HSX";
        
        // Load app source
        std::string appPath = hsxRoot + "/H_APP/source.hsx";
        if (fs::exists(appPath)) {
            config.appSource = readFile(appPath);
        }

        // Load router library
        std::string routerPath = hsxRoot + "/H_APP/LIB/Router.hsx";
        if (fs::exists(routerPath)) {
            config.routerLib = readFile(routerPath);
        }

        // Load links/routes definition
        std::string linksPath = hsxRoot + "/HSX.Routes/links.hsx";
        if (fs::exists(linksPath)) {
            config.linksSource = readFile(linksPath);
        }

        // Load all route pages
        std::string routesDir = hsxRoot + "/HSX.Routes";
        if (fs::exists(routesDir)) {
            for (const auto& entry : fs::directory_iterator(routesDir)) {
                if (entry.path().extension() == ".hsx" && 
                    entry.path().filename() != "links.hsx") {
                    PageFile page;
                    page.sourcePath = entry.path().string();
                    page.source = readFile(page.sourcePath);
                    
                    std::string filename = entry.path().stem().string();
                    page.route = filename == "index" ? "/" : "/" + filename;
                    
                    if (!page.source.empty()) {
                        config.pages.push_back(page);
                    }
                }
            }
        }

        return config;
    }

    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) return "";
        std::ostringstream ss;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) ss << line << "\n";
        }
        return ss.str();
    }
};

} // namespace hsx

#endif