// hsx-runtime.js
export async function loadHSX(url) {
  const res = await fetch(url);
  const text = await res.text();

  // Extract content inside <hsx> ... </hsx>
  const inner = text.match(/<hsx[^>]*>([\s\S]*)<\/hsx>/i)?.[1];
  if (!inner) throw new Error("Invalid HSX file");

  const temp = document.createElement("div");
  temp.innerHTML = inner;

  // -----------------------------
  // 1️⃣ Handle standard <script> modules
  // -----------------------------
  const scripts = temp.querySelectorAll("script");
  for (const s of scripts) {
    const newScript = document.createElement("script");
    if (s.src) newScript.src = s.src;
    if (s.type) newScript.type = s.type;
    newScript.textContent = s.textContent;
    document.body.appendChild(newScript);
  }

  // -----------------------------
  // 2️⃣ Handle HSX custom statements
  // Example: "hsx exist import correct file jsx/App.jsx"
  // -----------------------------
  const hsxLines = inner.split(/\r?\n/).map(line => line.trim());
  for (const line of hsxLines) {
    if (!line.startsWith("hsx ")) continue;

    try {
      if (line.includes("exist import correct file")) {
        const file = line.split("exist import correct file")[1].trim();
        const script = document.createElement("script");
        script.type = "module";
        script.src = file;
        document.body.appendChild(script);
        console.log(`📦 HSX: imported file module ${file}`);
      } else if (line.includes("exist import simple file")) {
        const file = line.split("exist import simple file")[1].trim();
        const script = document.createElement("script");
        script.src = file;
        document.body.appendChild(script);
        console.log(`📦 HSX: imported simple file ${file}`);
      } else if (line.includes("exist import node module")) {
        const mod = line.split("exist import node module")[1].trim();
        // Only works if bundled via MistBundle.js
        console.log(`📦 HSX: node module "${mod}" expected to be in bundle`);
      } else if (line.includes("exist import node built-in module")) {
        const mod = line.split("exist import node built-in module")[1].trim();
        console.log(`📦 HSX: node built-in module "${mod}"`);
      } else if (line.includes("file import all to")) {
        const dest = line.split("file import all to")[1].trim();
        console.log(`📦 HSX: bundling all files to ${dest}`);
      } else if (line.includes("file import/make/rename")) {
        const info = line.split("file import/make/rename")[1].trim();
        console.log(`📦 HSX: rename/move ${info}`);
      }
    } catch (err) {
      console.warn("⚠️ HSX runtime failed to parse line:", line, err);
    }
  }

  // -----------------------------
  // 3️⃣ Handle media elements
  // -----------------------------
  temp.querySelectorAll("img,video,canvas,div").forEach(el => {
    document.body.appendChild(el.cloneNode(true));
  });

  console.log("✅ HSX runtime fully loaded:", url);
}

// Auto-load if linked via <script data-src="...">
const current = document.currentScript?.dataset?.src;
if (current) loadHSX(current);
