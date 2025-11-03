// mega.js

// ===== Node.js Dependencies =====
// These imports are only for Node bundling; in the browser they do nothing.
import 'react';
import 'react-dom';
import 'axios';
import 'lodash';
import 'typescript';
import 'ts-node';
import 'express';
import 'httpx';
import 'fastify';
import 'uuid';

console.log('All Node.js dependencies loaded!');

// ===== Pyodide Integration for Python in-browser =====
async function initPyodide() {
    // Make sure pyodide.js and pyodide.wasm are in "./pyodide/" folder
    const pyodide = await loadPyodide({ indexURL: "./pyodide/" });
    console.log("Pyodide loaded!");

    // List of Python packages to pre-load
    await pyodide.loadPackage([
        "numpy",
        "pandas",
        "matplotlib",
        "flask",
        "fastapi",
        "seaborn",
        "ipywidgets",
        "pydantic",
        "jsonschema",
        "requests"
        // Add more as needed
    ]);

    console.log("Python packages loaded in Pyodide!");
}

// Initialize Pyodide (browser only)
if (typeof window !== "undefined") {
    initPyodide().catch(err => console.error(err));
}
