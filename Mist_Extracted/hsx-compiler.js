// hsx-compiler.js
import fs from "fs";
import path from "path";

console.log("🔮 Starting HSX compiler...");

// Utility: resolve file paths
function resolveFile(filePath) {
  const abs = path.resolve(filePath);
  if (!fs.existsSync(abs)) throw new Error(`File not found: ${abs}`);
  return abs;
}

// Parse a single HSX file
function parseHSX(filePath) {
  const content = fs.readFileSync(filePath, "utf-8");
  const lines = content.split(/\r?\n/);

  const commands = [];

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    if (!line || line.startsWith("//")) continue;

    // Match hsx commands
    const hsxExistMatch = line.match(
      /^hsx exist import (correct|simple|node module|node built-in) file (.+)$/
    );
    const hsxFileMatch = line.match(/^hsx file import all to (.+)$/);
    const hsxRenameMatch = line.match(
      /^hsx file import\/make\/rename\/(.+)-to-(.+)$/
    );

    if (hsxExistMatch) {
      commands.push({
        type: "exist-import",
        category: hsxExistMatch[1],
        file: hsxExistMatch[2],
      });
    } else if (hsxFileMatch) {
      commands.push({ type: "file-import-all", dest: hsxFileMatch[1] });
    } else if (hsxRenameMatch) {
      commands.push({
        type: "file-rename",
        from: hsxRenameMatch[1],
        to: hsxRenameMatch[2],
      });
    } else {
      console.warn(`⚠️ Unknown HSX line: ${line}`);
    }
  }

  return commands;
}

// Execute HSX commands
function buildHSX(filePath) {
  const commands = parseHSX(filePath);

  for (const cmd of commands) {
    switch (cmd.type) {
      case "exist-import":
        console.log(`📦 Import ${cmd.category} file: ${cmd.file}`);
        resolveFile(cmd.file); // Check file exists
        break;

      case "file-import-all":
        console.log(`🔗 Import all to: ${cmd.dest}`);
        break;

      case "file-rename":
        console.log(`✏️ Rename ${cmd.from} -> ${cmd.to}`);
        break;

      default:
        console.warn("⚠️ Unknown command type:", cmd);
    }
  }

  console.log("✅ HSX compilation finished!");
}

// Entry point
const hsxFile = process.argv[2] || "Mist.hsx";
buildHSX(hsxFile);
