const { exec } = require("child_process");
const path = require("path");
const fs = require("fs");

const projectDir = path.join(__dirname, "Mist_Extracted");

// Map file extensions to commands
const runtimes = {
  ".js": "node",
  ".jsx": "node",    // For React JSX, may need babel/ts-node if using import/export
  ".ts": "ts-node",
  ".tsx": "ts-node",
  ".py": "python",
  ".rb": "ruby",
  ".go": "go run",
  ".php": "php",
  ".sh": "bash"
};

// Run files sequentially
fs.readdirSync(projectDir).forEach(file => {
  const ext = path.extname(file).toLowerCase();
  const cmd = runtimes[ext];

  if (cmd) {
    const filePath = path.join(projectDir, file);
    console.log(`Running ${file}...`);

    exec(`${cmd} "${filePath}"`, (error, stdout, stderr) => {
      if (error) {
        console.error(`Error running ${file}:`, error.message);
      }
      if (stdout) console.log(stdout);
      if (stderr) console.error(stderr);
    });
  }
});
