const fs = require("fs");
const path = require("path");

// 1. Path to your HSX JSON file in root
const hsxPath = path.join(__dirname, "Mist.hsx"); 
const outputDir = path.join(__dirname, "Mist_Extracted"); // folder to extract files

// 2. Read HSX JSON
const hsxJSON = JSON.parse(fs.readFileSync(hsxPath, "utf-8"));

// 3. Create output folder if it doesn't exist
if (!fs.existsSync(outputDir)) {
  fs.mkdirSync(outputDir);
}

// 4. Extract all files
for (const [filename, content] of Object.entries(hsxJSON.files)) {
  let filePath = path.join(outputDir, filename);

  // Ensure folder exists if file is in subfolder
  const dir = path.dirname(filePath);
  if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });

  // Detect if file is binary (image, font)
  if (filename.match(/\.(png|woff|jpg|jpeg)$/i)) {
    // Convert Base64 to binary
    const buffer = Buffer.from(content, "base64");
    fs.writeFileSync(filePath, buffer);
  } else {
    // Write as text
    fs.writeFileSync(filePath, content, "utf-8");
  }
}

// 5. Extract logo if exists
if (hsxJSON.metadata.logo && hsxJSON.metadata.logo.data) {
  const logoPath = path.join(outputDir, "logo.mp4");
  const logoBuffer = Buffer.from(hsxJSON.metadata.logo.data, "base64");
  fs.writeFileSync(logoPath, logoBuffer);
  console.log("Logo extracted to:", logoPath);
}

console.log("All files extracted to:", outputDir);
