#ifndef HSX_BUNDLER_H
#define HSX_BUNDLER_H

#include "HSXProject.h"
#include "HSXCompiler.h"
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>

namespace hsx {

struct Bundle {
    std::string appJs;
    std::string appHash;
    std::vector<PageBundle> pages;
    std::string layoutHash;
};

struct PageBundle {
    std::string route;
    std::string js;
    std::string hash;
};

class HSXBundler {
public:
    static Bundle bundle(const ProjectConfig& project) {
        Bundle bundle;

        if (!project.appSource.empty()) {
            bundle.appJs = HSXPageCompiler::compilePage(project.appSource);
            bundle.appHash = hashString(bundle.appJs);
        }

        if (!project.layoutSource.empty()) {
            bundle.layoutHash = hashString(project.layoutSource);
        }

        for (const auto& page : project.pages) {
            PageBundle pb;
            pb.route = page.route;
            
            std::string pageJs = HSXPageCompiler::compilePage(page.source);
            
            if (!project.layoutSource.empty()) {
                std::string layoutJs = HSXPageCompiler::compilePage(project.layoutSource);
                pb.js = layoutJs + "\n" + pageJs;
            } else {
                pb.js = pageJs;
            }
            
            pb.hash = hashString(pb.js);
            bundle.pages.push_back(pb);
        }

        return bundle;
    }

    static std::string hashString(const std::string& input) {
        std::hash<std::string> hasher;
        std::ostringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hasher(input);
        return ss.str().substr(0, 8);
    }
};

} // namespace hsx

#endif