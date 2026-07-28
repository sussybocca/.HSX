#ifndef HSX_RENDERER_H
#define HSX_RENDERER_H

#include "HSXProject.h"
#include "HSXCompiler.h"
#include <string>
#include <vector>
#include <sstream>

namespace hsx {

struct RenderedPage {
    std::string route;
    CompiledPage compiled;
};

struct BuildOutput {
    std::vector<RenderedPage> pages;
    std::string manifestJson;
    std::string hydrationJs;
    std::string routerJs;
};

class HSXRenderer {
public:
    static BuildOutput render(const ProjectConfig& project) {
        BuildOutput output;

        for (const auto& page : project.pages) {
            RenderedPage rp;
            rp.route = page.route;
            rp.compiled = HSXPageCompiler::compile(page.source);
            output.pages.push_back(rp);
        }

        output.manifestJson = generateManifest(output.pages);
        output.hydrationJs = generateHydrationScript();
        output.routerJs = generateRouter(output.pages);

        return output;
    }

private:
    static std::string generateManifest(const std::vector<RenderedPage>& pages) {
        std::ostringstream json;
        json << "{\n";
        for (size_t i = 0; i < pages.size(); i++) {
            const auto& page = pages[i];
            std::string safeRoute = page.route == "/" ? "index" : page.route.substr(1);
            json << "  \"" << page.route << "\": {\n";
            json << "    \"js\": \"_hsx/pages/" << safeRoute << "." << page.compiled.hash << ".js\",\n";
            json << "    \"json\": \"_hsx/pages/" << safeRoute << "." << page.compiled.hash << ".json\"\n";
            json << "  }";
            if (i < pages.size() - 1) json << ",";
            json << "\n";
        }
        json << "}\n";
        return json.str();
    }

    static std::string generateHydrationScript() {
        return
            "// HSX Hydration v1.0\n"
            "window.__HSX_MANIFEST__ = {};\n"
            "window.__HSX_HYDRATE__ = function(route, data) {\n"
            "  var root = document.getElementById('hsx-root');\n"
            "  if (!root) return;\n"
            "  window.__HSX_DATA__ = data;\n"
            "  var nodes = root.querySelectorAll('[data-hsx-hydrate]');\n"
            "  nodes.forEach(function(node) {\n"
            "    var type = node.getAttribute('data-hsx-hydrate');\n"
            "    if (type === 'button') {\n"
            "      node.addEventListener('click', function() {\n"
            "        console.log('HSX: button clicked');\n"
            "      });\n"
            "    } else if (type === 'input') {\n"
            "      node.addEventListener('input', function(e) {\n"
            "        window.__HSX_DATA__[node.name || 'input'] = e.target.value;\n"
            "      });\n"
            "    }\n"
            "  });\n"
            "};\n"
            "window.__HSX_NAVIGATE__ = function(path) {\n"
            "  var manifest = window.__HSX_MANIFEST__;\n"
            "  var entry = manifest[path] || manifest['/'];\n"
            "  if (!entry) return;\n"
            "  fetch(entry.json)\n"
            "    .then(function(r) { return r.json(); })\n"
            "    .then(function(data) {\n"
            "      window.__HSX_HYDRATE__(path, data);\n"
            "      window.history.pushState({}, '', path);\n"
            "    });\n"
            "};\n"
            "fetch('_hsx/manifest.json')\n"
            "  .then(function(r) { return r.json(); })\n"
            "  .then(function(m) {\n"
            "    window.__HSX_MANIFEST__ = m;\n"
            "    var route = window.location.pathname || '/';\n"
            "    var entry = m[route] || m['/'];\n"
            "    if (entry) {\n"
            "      var script = document.createElement('script');\n"
            "      script.src = entry.js;\n"
            "      script.onload = function() {\n"
            "        if (window.__HSX_PAGE_INIT__) window.__HSX_PAGE_INIT__();\n"
            "      };\n"
            "      document.body.appendChild(script);\n"
            "    }\n"
            "  });\n";
    }

    static std::string generateRouter(const std::vector<RenderedPage>& pages) {
        std::ostringstream js;
        js << "// HSX Router\n";
        js << "(function() {\n";
        js << "  document.addEventListener('click', function(e) {\n";
        js << "    var link = e.target.closest('a[data-hsx-link]');\n";
        js << "    if (link) {\n";
        js << "      e.preventDefault();\n";
        js << "      var path = link.getAttribute('href');\n";
        js << "      if (window.__HSX_NAVIGATE__) {\n";
        js << "        window.__HSX_NAVIGATE__(path);\n";
        js << "      }\n";
        js << "    }\n";
        js << "  });\n";
        js << "  window.addEventListener('popstate', function() {\n";
        js << "    if (window.__HSX_NAVIGATE__) {\n";
        js << "      window.__HSX_NAVIGATE__(window.location.pathname || '/');\n";
        js << "    }\n";
        js << "  });\n";
        js << "})();\n";
        return js.str();
    }
};

} // namespace hsx

#endif