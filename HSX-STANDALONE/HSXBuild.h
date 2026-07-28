#ifndef HSX_BUILD_H
#define HSX_BUILD_H

#include "HSXProject.h"
#include "HSXRenderer.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>

namespace hsx {

class HSXBuild {
public:
    static bool build(const std::string& projectDir, const std::string& outputDir) {
        auto project = HSXProject::load(projectDir);
        auto output = HSXRenderer::render(project);

        std::string hsxDir = outputDir + "/_hsx";
        fs::create_directories(hsxDir + "/pages");
        fs::create_directories(hsxDir + "/chunks");

        // Write core CSS from the project's LIB folder if it exists, otherwise use built-in
        std::string coreCss = loadCoreCSS(projectDir);
        std::string cssHash = hashString(coreCss);
        std::string cssFile = "hsx-core." + cssHash + ".css";
        writeFile(hsxDir + "/" + cssFile, coreCss);

        // Write manifest
        writeFile(hsxDir + "/manifest.json", output.manifestJson);

        // Write hydration script
        std::string hydrationHash = hashString(output.hydrationJs);
        std::string hydrationFile = "chunks/hydration." + hydrationHash + ".js";
        writeFile(hsxDir + "/" + hydrationFile, output.hydrationJs);

        // Write router
        std::string routerHash = hashString(output.routerJs);
        std::string routerFile = "chunks/router." + routerHash + ".js";
        writeFile(hsxDir + "/" + routerFile, output.routerJs);

        // Write each page
        for (const auto& page : output.pages) {
            std::string safeRoute = page.route == "/" ? "index" : page.route.substr(1);
            std::string pageJsFile = "pages/" + safeRoute + "." + page.compiled.hash + ".js";
            std::string pageJsonFile = "pages/" + safeRoute + "." + page.compiled.hash + ".json";

            writeFile(hsxDir + "/" + pageJsFile, page.compiled.js);
            writeFile(hsxDir + "/" + pageJsonFile, page.compiled.json);

            std::string htmlPath = outputDir + "/" + safeRoute + ".html";
            if (page.route == "/") htmlPath = outputDir + "/index.html";
            writeFile(htmlPath, generatePageHTML(page, cssFile, hydrationFile, routerFile));
        }

        printSummary(output, cssFile, outputDir);
        return true;
    }

private:
    static std::string loadCoreCSS(const std::string& projectDir) {
        // First try to load user-defined CSS from the project
        std::string userCssPath = projectDir + "/H.Source/_.HSX/H_APP/core.css";
        if (fs::exists(userCssPath)) {
            return HSXProject::readFile(userCssPath);
        }
        // Fall back to built-in core CSS
        return getBuiltinCoreCSS();
    }

    static std::string getBuiltinCoreCSS() {
        return R"(
:root {
  --hsx-bg: #ffffff;
  --hsx-fg: #1f2328;
  --hsx-btn: #0969da;
  --hsx-btn-fg: #ffffff;
  --hsx-card: #f6f8fa;
  --hsx-card-bdr: #d0d7de;
  --hsx-link: #0969da;
  --hsx-radius: 6px;
  --hsx-shadow: 0 1px 3px rgba(0,0,0,0.12);
}

[data-theme="dark"] {
  --hsx-bg: #0d1117;
  --hsx-fg: #c9d1d9;
  --hsx-btn: #238636;
  --hsx-btn-fg: #ffffff;
  --hsx-card: #161b22;
  --hsx-card-bdr: #30363d;
  --hsx-link: #58a6ff;
}

[data-theme="ocean"] {
  --hsx-bg: #0a192f;
  --hsx-fg: #ccd6f6;
  --hsx-btn: #64ffda;
  --hsx-btn-fg: #0a192f;
  --hsx-card: #112240;
  --hsx-card-bdr: #233554;
  --hsx-link: #64ffda;
}

[data-theme="forest"] {
  --hsx-bg: #1b4332;
  --hsx-fg: #d8f3dc;
  --hsx-btn: #95d5b2;
  --hsx-btn-fg: #1b4332;
  --hsx-card: #2d6a4f;
  --hsx-card-bdr: #40916c;
  --hsx-link: #95d5b2;
}

[data-theme="sunset"] {
  --hsx-bg: #2d132c;
  --hsx-fg: #ffd4b8;
  --hsx-btn: #ee6b4b;
  --hsx-btn-fg: #ffffff;
  --hsx-card: #3d1c3c;
  --hsx-card-bdr: #5c2d5c;
  --hsx-link: #f9a26c;
}

body {
  background: var(--hsx-bg);
  color: var(--hsx-fg);
  font-family: system-ui, sans-serif;
}

.hsx-flex { display: flex; }
.hsx-grid { display: grid; }
.hsx-center { text-align: center; }
.hsx-stack { display: flex; flex-direction: column; }
.hsx-row { display: flex; flex-direction: row; }
.hsx-wrap { flex-wrap: wrap; }
.hsx-gap-1 { gap: 0.5rem; }
.hsx-gap-2 { gap: 1rem; }
.hsx-gap-3 { gap: 1.5rem; }
.hsx-gap-4 { gap: 2rem; }

.hsx-bg { background: var(--hsx-bg); }
.hsx-bg-card { background: var(--hsx-card); border: 1px solid var(--hsx-card-bdr); border-radius: var(--hsx-radius); }
.hsx-bg-btn { background: var(--hsx-btn); color: var(--hsx-btn-fg); }
.hsx-bg-transparent { background: transparent; }

.hsx-fg { color: var(--hsx-fg); }
.hsx-link { color: var(--hsx-link); cursor: pointer; text-decoration: none; }
.hsx-link:hover { text-decoration: underline; }

.hsx-text-sm { font-size: 0.875rem; }
.hsx-text-base { font-size: 1rem; }
.hsx-text-lg { font-size: 1.25rem; }
.hsx-text-xl { font-size: 1.5rem; }
.hsx-text-2xl { font-size: 2rem; }
.hsx-text-bold { font-weight: 700; }
.hsx-font-mono { font-family: monospace; }
.hsx-font-sans { font-family: system-ui, sans-serif; }

.hsx-p-1 { padding: 0.5rem; }
.hsx-p-2 { padding: 1rem; }
.hsx-p-3 { padding: 1.5rem; }
.hsx-p-4 { padding: 2rem; }
.hsx-m-1 { margin: 0.5rem; }
.hsx-m-2 { margin: 1rem; }
.hsx-m-3 { margin: 1.5rem; }
.hsx-m-4 { margin: 2rem; }

.hsx-rounded { border-radius: var(--hsx-radius); }
.hsx-rounded-lg { border-radius: 12px; }
.hsx-rounded-full { border-radius: 9999px; }
.hsx-border { border: 1px solid var(--hsx-card-bdr); }
.hsx-shadow { box-shadow: var(--hsx-shadow); }

.hsx-btn { 
  display: inline-block;
  padding: 0.5rem 1rem;
  background: var(--hsx-btn);
  color: var(--hsx-btn-fg);
  border: none;
  border-radius: var(--hsx-radius);
  cursor: pointer;
  font-size: 0.875rem;
  transition: opacity 0.2s;
}
.hsx-btn:hover { opacity: 0.85; }
.hsx-btn-sm { padding: 0.25rem 0.75rem; font-size: 0.75rem; }
.hsx-btn-lg { padding: 0.75rem 1.5rem; font-size: 1rem; }

.hsx-input {
  padding: 0.5rem;
  border: 1px solid var(--hsx-card-bdr);
  border-radius: var(--hsx-radius);
  background: var(--hsx-card);
  color: var(--hsx-fg);
  font-size: 0.875rem;
}
.hsx-input:focus { outline: 2px solid var(--hsx-link); }

.hsx-card {
  background: var(--hsx-card);
  border: 1px solid var(--hsx-card-bdr);
  border-radius: var(--hsx-radius);
  padding: 1rem;
  box-shadow: var(--hsx-shadow);
}

.hsx-badge {
  display: inline-block;
  padding: 0.125rem 0.5rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
  background: var(--hsx-btn);
  color: var(--hsx-btn-fg);
}

.hsx-table { width: 100%; border-collapse: collapse; }
.hsx-table td, .hsx-table th { padding: 0.5rem; border: 1px solid var(--hsx-card-bdr); }

.hsx-scroll { overflow-y: auto; }
.hsx-scroll-h { max-height: 400px; }

.hsx-hidden { display: none; }
.hsx-visible { display: block; }

.hsx-sticky { position: sticky; top: 0; }
.hsx-fixed-top { position: fixed; top: 0; left: 0; right: 0; }
.hsx-fixed-bottom { position: fixed; bottom: 0; left: 0; right: 0; }
)";
    }

    static std::string generatePageHTML(const RenderedPage& page, 
                                         const std::string& cssFile,
                                         const std::string& hydrationFile,
                                         const std::string& routerFile) {
        std::ostringstream html;
        html << "<!DOCTYPE html>\n";
        html << "<html lang=\"en\">\n";
        html << "<head>\n";
        html << "  <meta charset=\"UTF-8\">\n";
        html << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html << "  <title>HSX - " << page.route << "</title>\n";
        html << "  <link rel=\"stylesheet\" href=\"_hsx/" << cssFile << "\">\n";
        html << "</head>\n";
        html << "<body>\n";
        html << page.compiled.staticHTML;
        html << "  <script src=\"_hsx/" << hydrationFile << "\"></script>\n";
        html << "  <script src=\"_hsx/" << routerFile << "\"></script>\n";
        html << "</body>\n";
        html << "</html>\n";
        return html.str();
    }

    static void writeFile(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        if (file) file << content;
    }

    static std::string hashString(const std::string& input) {
        std::hash<std::string> hasher;
        std::ostringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hasher(input);
        return ss.str().substr(0, 8);
    }

    static void printSummary(const BuildOutput& output, const std::string& cssFile, const std::string& outputDir) {
        std::cout << "\n=== HSX Build Complete ===\n";
        std::cout << "Output: " << outputDir << "/\n\n";
        std::cout << "_hsx/" << cssFile << " (core styles)\n";
        for (const auto& page : output.pages) {
            std::string safeRoute = page.route == "/" ? "index" : page.route.substr(1);
            std::cout << safeRoute << ".html  ->  " << page.route << "\n";
        }
        std::cout << "\nStatic HTML with hydration. Deploy '" << outputDir << "' anywhere.\n";
        std::cout << "Run: npx serve " << outputDir << "\n\n";
    }
};

} // namespace hsx
#endif