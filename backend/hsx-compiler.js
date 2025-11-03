import fs from "fs";
import path from "path";

console.log("🔮 Starting HSX compiler...");

// Utility: resolve file paths relative to the HSX file
function resolveFile(filePath, hsxDir) {
  const abs = path.resolve(hsxDir, filePath);
  if (!fs.existsSync(abs)) throw new Error(`File not found: ${abs}`);
  return abs;
}

// Parse a single HSX file
function parseHSX(filePath) {
  const content = fs.readFileSync(filePath, "utf-8");
  const lines = content.split(/\r?\n/);

  const commands = [];

  for (let line of lines) {
    line = line.trim();
    if (!line || line.startsWith("//")) continue;

    const hsxExistMatch = line.match(
      /^hsx exist import (correct|simple|node module|node built-in) file (.+)$/
    );
    const hsxFileMatch = line.match(/^hsx file import all to (.+)$/);
    const hsxRenameMatch = line.match(
      /^hsx file import\/make\/rename\s+(.+)-to-(.+)$/
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
  const hsxDir = path.dirname(filePath); // resolve paths relative to HSX file
  const commands = parseHSX(filePath);
  let combinedContent = "";
  let outputPath = null;

  for (const cmd of commands) {
    switch (cmd.type) {
      case "exist-import":
        console.log(`📦 Import ${cmd.category} file: ${cmd.file}`);
        const absPath = resolveFile(cmd.file, hsxDir);

        // Include content only for 'correct' or 'simple' imports
        if (cmd.category === "correct" || cmd.category === "simple") {
          const content = fs.readFileSync(absPath, "utf-8");
          combinedContent += `\n-- Begin ${cmd.file} --\n`;
          combinedContent += content;
          combinedContent += `\n-- End ${cmd.file} --\n`;
        }
        break;

      case "file-import-all":
        console.log(`🔗 Import all to: ${cmd.dest}`);
        outputPath = path.resolve(hsxDir, cmd.dest);
        break;

      case "file-rename":
        console.log(`✏️ Rename ${cmd.from} -> ${cmd.to}`);
        const renameFrom = path.resolve(hsxDir, cmd.from);
        const renameTo = path.resolve(hsxDir, cmd.to);
        if (fs.existsSync(renameFrom)) fs.renameSync(renameFrom, renameTo);
        break;

      default:
        console.warn("⚠️ Unknown command type:", cmd);
    }
  }

  // Write combined content to output path
  if (outputPath) {
    fs.mkdirSync(path.dirname(outputPath), { recursive: true });
    fs.writeFileSync(outputPath, combinedContent, "utf-8");
    console.log(`✅ HSX compilation finished! File written to: ${outputPath}`);
  } else {
    console.warn("⚠️ No output file specified via 'file import all to'");
  }
}

// Entry point
const hsxFile = process.argv[2] || "Mist.hsx";
buildHSX(hsxFile);

