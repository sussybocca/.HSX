// hsx-runtime.js — HSX v0.61 Core (Safe JS + DOM Ready)
// © 2025 William Isaiah Jones

export class HSXRuntime {
  constructor() {
    this.components = {};
    this.context = {};
    this.pyodide = null;
  }

  async initPyodide() {
    if (!this.pyodide) {
      console.log("🐍 Initializing Pyodide...");
      try {
        const { loadPyodide } = await import("./pyodide/pyodide.mjs");
        this.pyodide = await loadPyodide({ indexURL: "./pyodide/" });
      } catch (e) {
        console.warn("⚠️ Pyodide not available, Python blocks will be skipped.");
      }
    }
  }

  async loadFiles(filePaths) {
    if (!Array.isArray(filePaths)) filePaths = [filePaths];
    for (const path of filePaths) await this.load(path);
  }

  async load(filePath) {
    console.log(`🌀 Loading HSX file: ${filePath}`);
    try {
      const response = await fetch(filePath);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const code = await response.text();
      await this.execute(code);
    } catch (e) {
      console.error(`❌ Failed to load HSX file: ${filePath}`, e);
    }
  }

  async execute(code) {
    const lines = code.split("\n").map(l => l.trimEnd()); // keep indentation for JS blocks

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim();

      // Media attachments
      if (line.startsWith("hsx attach image")) {
        const match = line.match(/"(.*?)"/);
        if (match) {
          const src = match[1];
          const img = document.createElement("img");
          img.src = src;
          img.alt = "hsx-image";
          img.style.width = "400px";
          document.body.appendChild(img);
          console.log(`🖼️ Attached image: ${src}`);
        }
        continue;
      }

      if (line.startsWith("hsx attach video")) {
        const match = line.match(/"(.*?)"/);
        if (match) {
          const src = match[1];
          const video = document.createElement("video");
          video.src = src;
          video.controls = true;
          document.body.appendChild(video);
          console.log(`🎞️ Attached video: ${src}`);
        }
        continue;
      }

      // Components
      if (line.startsWith("hsx define component")) {
        const name = line.replace("hsx define component", "").trim();
        let body = "";
        i++;
        while (i < lines.length && !lines[i].startsWith("hsx end")) {
          body += lines[i] + "\n";
          i++;
        }
        this.components[name] = body;
        console.log(`🧩 Component defined: ${name}`);
        continue;
      }

      // Render
      if (line.startsWith("hsx render")) {
        const comp = line.replace("hsx render", "").trim();
        if (this.components[comp]) {
          const el = document.createElement("div");
          el.innerHTML = this.components[comp];
          document.body.appendChild(el);
          console.log(`✨ Rendered component: ${comp}`);
        } else {
          console.warn(`⚠️ Component not found: ${comp}`);
        }
        continue;
      }

      // === {js ... } ===
      if (line.startsWith("{js")) {
        let jsCode = "";
        i++;
        while (i < lines.length && !lines[i].match(/^}\s*$/)) {
          jsCode += lines[i] + "\n";
          i++;
        }
        jsCode = jsCode.trim();
        if (jsCode) {
          // Wrap in DOMContentLoaded to ensure elements exist
          try {
            new Function(`
              document.addEventListener('DOMContentLoaded', () => {
                ${jsCode}
              });
            `)();
            console.log("💻 JS block executed (DOM safe).");
          } catch (e) {
            console.error("❌ JavaScript error:", e, "\nCode:\n", jsCode);
          }
        } else {
          console.warn("⚠️ Skipped empty JS block.");
        }
        continue;
      }

      // === {py ... } ===
      if (line.startsWith("{py")) {
        let pyCode = "";
        i++;
        while (i < lines.length && !lines[i].match(/^}\s*$/)) {
          pyCode += lines[i] + "\n";
          i++;
        }
        if (this.pyodide) {
          try {
            await this.pyodide.runPythonAsync(pyCode);
            console.log("🐍 Python block executed.");
          } catch (e) {
            console.error("❌ Python error:", e, "\nCode:\n", pyCode);
          }
        } else {
          console.warn("⚠️ Skipping Python block (Pyodide not loaded).");
        }
        continue;
      }
    }

    console.log("✅ HSX execution complete!");
  }
}

// === Auto-load Support ===
if (location.search.includes("hsxFiles=")) {
  const filesParam = new URLSearchParams(location.search).get("hsxFiles");
  const files = filesParam.split(",");
  const hsx = new HSXRuntime();
  hsx.loadFiles(files);
} else if (location.pathname.endsWith(".hsx")) {
  const hsx = new HSXRuntime();
  hsx.load(location.pathname);
}
