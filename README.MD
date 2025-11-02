# HSX VS Code Extension

![HSX Logo](icon/logo.png) <!-- Use a high-quality PNG or SVG logo here -->

## Overview

**HSX** is a custom file extension and VS Code language support for Node.js + Pyodide scripts with bundled dependencies.  
It allows you to write `.hsx` files that combine HTML-like structure, JavaScript modules, Python via Pyodide, and media elements seamlessly.

With HSX, you can:

- Create `.hsx` files using VS Code.
- Automatically load Node.js modules and Python packages.
- Include images, video, and other media.
- Use a custom compiler and runtime for your HSX scripts.

---

## Features

- **Language Support:** Syntax highlighting, comments, strings, keywords, and custom HSX statements.
- **Commands:** Easily create a new `.hsx` file with a prebuilt template.
- **Bundled Dependencies:** Automatically manage required JS and Pyodide libraries.
- **Media Integration:** Load images, videos, and canvas elements inside `.hsx` files.

---

## Installation

1. Download the latest `.vsix` package from the [Releases](https://github.com/sussybocca/.HSX/releases) page.
2. In VS Code, go to `Extensions` → `...` (top-right) → `Install from VSIX...`.
3. Select the downloaded `.vsix` file.
4. Reload VS Code if necessary.

---

## Usage

1. Open VS Code and create a new `.hsx` file:
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on macOS) → type **Create New HSX File** → press Enter.
   - This will create a template `.hsx` file with compiler and runtime scripts included.
2. Write your HSX code using HTML-like tags, JS modules, or Pyodide Python scripts.
3. To run your HSX file in a web environment:
   - Include `<script src="hsx-runtime.js"></script>` in your HTML.
   - Load your `.hsx` file via `loadHSX('your-file.hsx')`.

---

## File Structure
