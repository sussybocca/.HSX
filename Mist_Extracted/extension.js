const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

function activate(context) {
  let disposable = vscode.commands.registerCommand('hsx.createNewFile', async () => {
    const uri = await vscode.window.showSaveDialog({
      filters: { 'HSX Files': ['hsx'] },
      saveLabel: 'Create HSX File'
    });
    if (!uri) return;

    const template = `<hsx>
  <script type="module" src="MistBundle.js"></script>
  <script type="module">
    async function initPython() {
        const pyodide = await loadPyodide({ indexURL: "./pyodide/" });
        await pyodide.loadPackage([
            "numpy","pandas","matplotlib","flask",
            "fastapi","seaborn","ipywidgets",
            "pydantic","jsonschema","requests"
        ]);

        const response = await fetch("mega.py");
        const pyCode = await response.text();
        pyodide.runPython(pyCode);

        const img = document.createElement("img");
        img.src = "image.png";
        img.style.width = "300px";
        document.body.appendChild(img);

        const video = document.createElement("video");
        video.src = "logo.mp4";
        video.controls = true;
        video.style.width = "400px";
        document.body.appendChild(video);
    }
    initPython();
  </script>
</hsx>`;

    fs.writeFileSync(uri.fsPath, template, 'utf8');
    vscode.window.showInformationMessage(`Created new HSX file: ${uri.fsPath}`);
  });

  context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = { activate, deactivate };
