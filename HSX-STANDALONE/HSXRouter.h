#ifndef HSX_ROUTER_H
#define HSX_ROUTER_H

#include "HSXBundler.h"
#include <string>
#include <sstream>

namespace hsx {

class HSXRouter {
public:
    static std::string generate(const Bundle& bundle) {
        std::ostringstream js;
        js << "// HSX Router v1.0\n";
        js << "(function() {\n";
        js << "  var _routes = {\n";
        
        for (size_t i = 0; i < bundle.pages.size(); i++) {
            const auto& page = bundle.pages[i];
            std::string safeRoute = sanitizeRoute(page.route);
            js << "    '" << page.route << "': 'pages/" << safeRoute << "." << page.hash << ".js'";
            if (i < bundle.pages.size() - 1) js << ",";
            js << "\n";
        }
        
        js << "  };\n\n";
        js << "  var _appEl = document.getElementById('hsx-app') || document.body;\n\n";
        js << "  function _loadPageScript(src) {\n";
        js << "    _appEl.innerHTML = '';\n";
        js << "    var s = document.createElement('script');\n";
        js << "    s.src = src;\n";
        js << "    s.setAttribute('data-hsx-page', 'true');\n";
        js << "    var old = document.querySelector('script[data-hsx-page]');\n";
        js << "    if (old) old.remove();\n";
        js << "    _appEl.appendChild(s);\n";
        js << "  }\n\n";
        js << "  function _navigate(path) {\n";
        js << "    var file = _routes[path] || _routes['/'];\n";
        js << "    if (!file) return;\n";
        js << "    _loadPageScript(file);\n";
        js << "    window.history.pushState({}, '', path);\n";
        js << "  }\n\n";
        js << "  window.addEventListener('popstate', function() {\n";
        js << "    _navigate(window.location.pathname);\n";
        js << "  });\n\n";
        js << "  document.addEventListener('click', function(e) {\n";
        js << "    var link = e.target.closest('a[data-hsx-link]');\n";
        js << "    if (link) {\n";
        js << "      e.preventDefault();\n";
        js << "      _navigate(link.getAttribute('href'));\n";
        js << "    }\n";
        js << "  });\n\n";
        js << "  _navigate(window.location.pathname || '/');\n";
        js << "})();\n";
        
        return js.str();
    }

private:
    static std::string sanitizeRoute(const std::string& route) {
        std::string result = route == "/" ? "index" : route;
        if (result[0] == '/') result = result.substr(1);
        for (char& c : result) {
            if (!std::isalnum(c) && c != '-' && c != '_') c = '-';
        }
        return result;
    }
};

} // namespace hsx

#endif