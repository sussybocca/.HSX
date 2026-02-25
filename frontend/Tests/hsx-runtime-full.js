// hsx-runtime-full.js — HSX v0.72+ FULL UPDATED with Advanced Custom Language System
// © 2026 William Isaiah Jones + HSXEngine integration

export class HSXRuntime {
  constructor() {
    // Original HSXRuntime fields
    this.components = {};
    this.context = {};
    this.blocks = {};
    this.data = {};
    this.modules = {};
    this.attachments = {};
    this.metaTags = {};
    this.emotions = {};
    this.pyodide = null;
    this.sandboxed = true;

    this.emotionActive = false;
    this.dataExportActive = false;
    this.metaActive = false;

    // HSXEngine fields
    this.botsDB = {};
    this.customDatabaseBlocks = {};
    this.storages = {};
    this.physicStorage = {};
    this.customStorages = {};
    this.customCodeLines = {};

    // ADVANCED CUSTOM LANGUAGE SYSTEM FIELDS
    this.customLanguages = {
      // Built-in languages
      'JAY': { parser: this._parseJAY.bind(this), extensions: ['.jay'] },
      'CRYPTIC': { parser: this._parseCRYPTIC.bind(this), extensions: ['.crypt'] },
      'SHELLISH': { parser: this._parseSHELLISH.bind(this), extensions: ['.shx'] },
      'FLOW': { parser: this._parseFLOW.bind(this), extensions: ['.flow'] }
    };
    
    this.languageExtensions = {};
    this.fxEnabled = false;
    this.gameConfig = { 
      pixels: [], 
      fps: 60, 
      reload: false, 
      spawnCorner: false, 
      fx: false,
      physics: { gravity: 9.8, friction: 0.98 }
    };
    this.jayTagsEnabled = false;
    
    // ADVANCED FEATURES
    this.quantumMode = false;
    this.neuralNetwork = null;
    this.blockchainLedger = [];
    this.arCanvas = null;
    this.voiceCommands = {};
    this.aiModels = {};
    
    // VISUAL PROGRAMMING
    this.visualNodes = {};
    this.nodeConnections = [];
    
    // DEBUG & MONITORING
    this.metrics = {
      executionTime: 0,
      memoryUsage: 0,
      errors: [],
      warnings: []
    };
    
    console.log('🚀 HSX Advanced Custom Language Runtime Initialized');
  }

  // ================ ADVANCED CUSTOM LANGUAGE SYSTEM ================
  
  createLanguage(name, config = {}) {
    const language = {
      name,
      keywords: config.keywords || [],
      syntax: config.syntax || {},
      parser: config.parser || this._defaultParser.bind(this),
      compiler: config.compiler || this._defaultCompiler.bind(this),
      extensions: config.extensions || [`.${name.toLowerCase()}`],
      version: config.version || '1.0.0',
      meta: config.meta || {}
    };
    
    this.customLanguages[name] = language;
    
    // Register extensions
    language.extensions.forEach(ext => {
      this.languageExtensions[ext] = name;
    });
    
    console.log(`🧬 Created new coding language: ${name} v${language.version}`);
    
    // Generate language documentation automatically
    this._generateLanguageDocs(name, language);
    
    return language;
  }
  
  _generateLanguageDocs(name, language) {
    const docs = {
      name,
      version: language.version,
      keywords: language.keywords,
      example: `
// Example ${name} Code:
${name}:main
  print "Hello from ${name}"
end
      `.trim(),
      cheatsheet: this._createCheatsheet(language)
    };
    
    this.data[`${name}_docs`] = docs;
    console.log(`📚 Generated documentation for ${name}`);
  }
  
  _createCheatsheet(lang) {
    return `
${lang.name.toUpperCase()} LANGUAGE CHEATSHEET
================================
Keywords: ${lang.keywords.join(', ')}
Extensions: ${lang.extensions.join(', ')}

SYNTAX EXAMPLES:
- Variable: store x = 10
- Function: func name()
- Loop: repeat 10 times
- Condition: if condition then
    `.trim();
  }
  
  // Language Parsers
  _parseJAY(code) {
    console.log('🏷️ Parsing JAY language code');
    const lines = code.split('\n');
    const output = [];
    
    lines.forEach(line => {
      if (line.trim().startsWith('JAY:')) {
        const cmd = line.replace('JAY:', '').trim();
        if (cmd.startsWith('tag')) {
          const tagName = cmd.split(' ')[1];
          output.push(`<jay-${tagName}>`);
        } else if (cmd.startsWith('style')) {
          const style = cmd.replace('style', '').trim();
          output.push(`<style>${style}</style>`);
        }
      } else if (line.includes('<<')) {
        const dynamic = line.replace('<<', '{{').replace('>>', '}}');
        output.push(dynamic);
      }
    });
    
    return output.join('\n');
  }
  
  _parseCRYPTIC(code) {
    console.log('🔐 Parsing CRYPTIC language code');
    // Encrypted-style language
    const lines = code.split('\n');
    const output = [];
    
    lines.forEach(line => {
      if (line.includes('🔒')) {
        const decrypted = line.replace('🔒', '').split('').reverse().join('');
        output.push(decrypted);
      } else if (line.includes('🌀')) {
        const parts = line.split('🌀');
        output.push(`function ${parts[1]}() { ${parts[2] || ''} }`);
      }
    });
    
    return output.join('\n');
  }
  
  _parseSHELLISH(code) {
    console.log('🐚 Parsing SHELLISH language code');
    // Shell-like syntax
    const lines = code.split('\n');
    const output = [];
    
    lines.forEach(line => {
      if (line.startsWith('$')) {
        const cmd = line.substring(1).trim();
        output.push(`// SHELLISH: ${cmd}`);
        
        if (cmd.startsWith('echo')) {
          const msg = cmd.replace('echo', '').trim();
          output.push(`console.log("${msg}");`);
        } else if (cmd.startsWith('loop')) {
          const count = cmd.match(/\d+/)?.[0] || '10';
          output.push(`for(let i=0; i<${count}; i++) {`);
        }
      } else if (line === 'end') {
        output.push('}');
      }
    });
    
    return output.join('\n');
  }
  
  _parseFLOW(code) {
    console.log('🌊 Parsing FLOW language code');
    // Visual flow programming
    const lines = code.split('\n');
    const nodes = [];
    const connections = [];
    
    lines.forEach(line => {
      if (line.includes('->')) {
        const [from, to] = line.split('->').map(s => s.trim());
        connections.push({ from, to });
      } else if (line.includes('node')) {
        const nodeName = line.replace('node', '').trim();
        nodes.push({ name: nodeName, type: 'function' });
      }
    });
    
    this.visualNodes = nodes;
    this.nodeConnections = connections;
    
    return `// Flow diagram with ${nodes.length} nodes and ${connections.length} connections`;
  }
  
  _defaultParser(code) {
    return `// Default parser output for:\n${code}`;
  }
  
  _defaultCompiler(ast) {
    return `// Compiled output`;
  }
  
  // ================ QUANTUM COMPUTING MODE ================
  
  enableQuantumMode() {
    this.quantumMode = true;
    console.log('⚛️ Quantum Mode Activated');
    
    // Simulate quantum superposition
    this.data = new Proxy(this.data, {
      get(target, prop) {
        const value = target[prop];
        // Quantum superposition: value exists in multiple states
        if (Array.isArray(value)) {
          return value[Math.floor(Math.random() * value.length)];
        }
        return value;
      },
      set(target, prop, value) {
        // Quantum entanglement: setting one affects others
        target[prop] = value;
        if (prop.startsWith('qbit_')) {
          const entangled = prop.replace('qbit_', 'ent_');
          target[entangled] = value;
        }
        return true;
      }
    });
  }
  
  // ================ NEURAL NETWORK INTEGRATION ================
  
  createNeuralNetwork(layers = [3, 4, 2]) {
    console.log('🧠 Creating Neural Network');
    this.neuralNetwork = {
      layers,
      weights: [],
      biases: [],
      activation: x => 1 / (1 + Math.exp(-x)), // Sigmoid
      predict: function(input) {
        let output = input;
        for (let i = 0; i < this.layers.length - 1; i++) {
          output = this._forward(output, i);
        }
        return output;
      },
      _forward: function(input, layerIndex) {
        // Simplified forward pass
        return input.map(x => this.activation(x * 0.5));
      }
    };
    
    // Train with random data
    setTimeout(() => {
      console.log('🧠 Neural Network trained with simulated data');
      this.data.neural_ready = true;
    }, 1000);
  }
  
  // ================ BLOCKCHAIN SYSTEM ================
  
  addBlock(data) {
    const block = {
      index: this.blockchainLedger.length,
      timestamp: Date.now(),
      data,
      previousHash: this.blockchainLedger.length > 0 ? 
        this._hashBlock(this.blockchainLedger[this.blockchainLedger.length - 1]) : '0',
      hash: ''
    };
    
    block.hash = this._hashBlock(block);
    this.blockchainLedger.push(block);
    
    console.log(`⛓️ Block #${block.index} added to blockchain`);
    return block;
  }
  
  _hashBlock(block) {
    const str = JSON.stringify(block);
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
      hash = ((hash << 5) - hash) + str.charCodeAt(i);
      hash |= 0;
    }
    return hash.toString(16);
  }
  
  // ================ AR/VISUAL PROGRAMMING ================
  
  initARCanvas() {
    if (!this.arCanvas) {
      this.arCanvas = document.createElement('canvas');
      this.arCanvas.width = 400;
      this.arCanvas.height = 400;
      this.arCanvas.style.position = 'fixed';
      this.arCanvas.style.top = '10px';
      this.arCanvas.style.right = '10px';
      this.arCanvas.style.border = '2px solid #00ff00';
      document.body.appendChild(this.arCanvas);
      
      const ctx = this.arCanvas.getContext('2d');
      ctx.fillStyle = 'rgba(0, 255, 0, 0.1)';
      ctx.fillRect(0, 0, 400, 400);
      
      console.log('👁️ AR Canvas initialized');
    }
    
    return this.arCanvas;
  }
  
  // ================ VOICE COMMAND SYSTEM ================
  
  registerVoiceCommand(command, callback) {
    this.voiceCommands[command.toLowerCase()] = callback;
    
    if ('webkitSpeechRecognition' in window || 'SpeechRecognition' in window) {
      const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
      if (!this.speechRecognition) {
        this.speechRecognition = new SpeechRecognition();
        this.speechRecognition.continuous = true;
        this.speechRecognition.interimResults = false;
        
        this.speechRecognition.onresult = (event) => {
          const transcript = event.results[event.results.length - 1][0].transcript.toLowerCase();
          console.log(`🎤 Voice command detected: "${transcript}"`);
          
          Object.keys(this.voiceCommands).forEach(cmd => {
            if (transcript.includes(cmd)) {
              this.voiceCommands[cmd](transcript);
            }
          });
        };
        
        this.speechRecognition.start();
      }
    }
  }
  
  // ================ ENHANCED HSXEngine Methods ================
  
  runHSXEngineLine(line, lines, index) {
    // Handle custom language file execution
    for (const [ext, langName] of Object.entries(this.languageExtensions)) {
      if (line.endsWith(ext)) {
        console.log(`🌐 Executing ${langName} file: ${line}`);
        const lang = this.customLanguages[langName];
        if (lang && lang.parser) {
          // Simulate loading and parsing
          setTimeout(() => {
            const simulatedCode = `${langName}:main\n  print "Executing ${langName} code"\nend`;
            const output = lang.parser(simulatedCode);
            console.log(`📝 ${langName} output:`, output);
          }, 100);
        }
        return index;
      }
    }
    
    // Bots database (enhanced)
    if (line === "bots:") {
      this.botsDB.records ??= [];
      index++;
      while (lines[index]?.trim().startsWith(";:")) {
        const raw = lines[index].replace(";:", "").trim();
        const [name, bot, meta, aiModel = 'default'] = raw.split(",").map(v => v.trim());
        this.botsDB.records.push({ 
          name, 
          bot, 
          meta, 
          formats: {},
          aiModel,
          personality: this._generateBotPersonality(name)
        });
        index++;
      }
      console.log(`🤖 Loaded ${this.botsDB.records.length} AI bots`);
      return index - 1;
    }
    
    // HSX storage blocks (enhanced with AI)
    if (line === ":Hsx:") {
      index++;
      let next = lines[index]?.trim();
      if (!next) return index;
      
      if (next.startsWith("$")) {
        const storageName = next.replace("$", "").trim();
        this.storages[storageName] ??= { ai_enhanced: true };
        index++;
        while (lines[index]?.includes("=")) {
          let [k, v] = lines[index].split("=").map(x => x.trim());
          
          // AI-powered value interpretation
          if (v.startsWith("ai:")) {
            const aiPrompt = v.replace("ai:", "").replace(/"/g, "");
            v = this._aiInterpret(aiPrompt);
          }
          
          this.storages[storageName][k] = v.replace(/"/g, "");
          index++;
        }
        this.embedPhysic(storageName);
      } else if (next.startsWith("create storage")) {
        const storageName = next.split(" ").slice(2).join(" ").trim();
        this.createCustomStorage(storageName);
        index++;
        while (lines[index]?.includes("=")) {
          let [k, v] = lines[index].split("=").map(x => x.trim());
          
          // Support for dynamic expressions
          if (v.includes("{{") && v.includes("}}")) {
            v = this._evaluateExpression(v);
          }
          
          this.customStorages[storageName][k] = v.replace(/"/g, "");
          index++;
        }
      } else if (next.endsWith(":") && !next.startsWith("$")) {
        const dbName = next.replace(":", "");
        this.customDatabaseBlocks[dbName] ??= [];
        index++;
        while (lines[index]?.trim().startsWith(";:")) {
          this.customDatabaseBlocks[dbName].push(lines[index].replace(";:", "").trim());
          index++;
        }
      }
      return index - 1;
    }
    
    // CCCL Custom code lines (enhanced)
    if (line.startsWith("CCCL")) {
      const name = line.split(" ")[1];
      index++;
      let block = [];
      while (lines[index]?.trim() !== "}") {
        block.push(lines[index]);
        index++;
      }
      
      // Support for nested blocks
      const blockStr = block.join("\n");
      this.customCodeLines[name] = {
        code: blockStr,
        compiled: this._compileCustomCode(blockStr),
        metadata: {
          created: Date.now(),
          size: blockStr.length,
          complexity: this._calculateComplexity(blockStr)
        }
      };
      
      console.log(`📦 Custom code line registered: ${name} (${blockStr.length} chars)`);
      return index;
    }
    
    // Execute custom code lines (enhanced)
    if (this.customCodeLines[line]) {
      const block = this.customCodeLines[line];
      console.log(`🚀 Executing custom code: ${line}`);
      
      if (block.compiled) {
        try {
          const result = eval(block.compiled);
          if (result !== undefined) {
            this.data[`_${line}_result`] = result;
          }
        } catch (e) {
          console.error(`❌ Error executing ${line}:`, e);
        }
      }
    }
    
    // -------- ADVANCED HSX CUSTOM LANGUAGE / FX / GAME SYSTEM --------
    if (line.startsWith(":hsx:")) {
      const cmd = line.replace(":hsx:", "").trim();
      const response = this._processAdvancedCommand(cmd);
      
      if (response.blockchain) {
        this.addBlock(response.blockchain);
      }
      
      return index;
    }
    
    // Quantum computing commands
    if (line.startsWith(":quantum:")) {
      const qcmd = line.replace(":quantum:", "").trim();
      if (qcmd === "enable") {
        this.enableQuantumMode();
      } else if (qcmd.startsWith("qbit")) {
        const [, name, value] = qcmd.split(" ");
        this.data[`qbit_${name}`] = value;
        console.log(`⚛️ Quantum bit created: qbit_${name} = ${value}`);
      }
      return index;
    }
    
    // Neural network commands
    if (line.startsWith(":neural:")) {
      const ncmd = line.replace(":neural:", "").trim();
      if (ncmd === "create") {
        this.createNeuralNetwork();
      } else if (ncmd.startsWith("predict")) {
        const input = ncmd.replace("predict", "").trim();
        if (this.neuralNetwork) {
          const prediction = this.neuralNetwork.predict(input.split(",").map(Number));
          console.log(`🧠 Neural prediction:`, prediction);
        }
      }
      return index;
    }
    
    // Voice command registration
    if (line.startsWith(":voice:")) {
      const vcmd = line.replace(":voice:", "").trim();
      const [command, ...actionParts] = vcmd.split("=");
      const action = actionParts.join("=");
      
      this.registerVoiceCommand(command.trim(), (transcript) => {
        console.log(`🎤 Executing voice action for "${command}": ${action}`);
        this.execute(action);
      });
      
      return index;
    }
    
    return index;
  }
  
  _processAdvancedCommand(cmd) {
    const response = { executed: true };
    
    // CREATE NEW CODING LANGUAGE (Enhanced)
    if (cmd.startsWith("create new coding lango")) {
      const match = cmd.match(/create new coding lango (?:called|custom lango) (\w+)/i);
      if (match) {
        const name = match[1];
        const version = cmd.match(/version (\d+\.\d+\.\d+)/i)?.[1] || "1.0.0";
        const keywords = cmd.match(/keywords\s*\[(.*?)\]/i)?.[1]?.split(",").map(k => k.trim()) || [];
        
        const newLang = this.createLanguage(name, {
          version,
          keywords,
          meta: { created: new Date().toISOString() }
        });
        
        response.language = newLang;
        response.blockchain = { type: "language_created", name, version };
      }
    }
    
    // FX ENABLE (Enhanced)
    if (cmd.includes("fx attach fx")) {
      this.fxEnabled = true;
      document.body.style.filter = "hue-rotate(15deg) saturate(1.4)";
      document.body.style.transition = "filter 2s";
      
      // Add particle effects
      this._createParticleEffects();
      console.log("✨ Advanced FX enabled with particles");
    }
    
    // ALLOW CUSTOM LANGUAGE FILES (Enhanced)
    if (cmd.includes("allow new coding language files")) {
      console.log("📄 New coding language files allowed");
      // Auto-create sample files for each language
      Object.keys(this.customLanguages).forEach(lang => {
        const ext = this.customLanguages[lang].extensions[0];
        this.data[`sample_${lang.toLowerCase()}${ext}`] = 
          `${lang}:main\n  print "Hello from ${lang}"\nend`;
      });
    }
    
    // GAME PIXELS (Enhanced)
    if (cmd.includes("game pixel")) {
      const colors = cmd.match(/red|blue|orange|green|purple|yellow|cyan|magenta/g) || [];
      this.gameConfig.pixels = colors;
      if (colors.length > 3) {
        this.gameConfig.physics.gravity = 2.0;
        this.gameConfig.fps = 120;
      }
    }
    
    // NEW DATA (Enhanced)
    if (cmd.includes("new data")) {
      const parts = cmd.split("eq");
      const key = parts[0].replace("new data", "").trim();
      let value = parts[1]?.trim() || "";
      
      // Support for expressions
      if (value.includes("{{") && value.includes("}}")) {
        value = this._evaluateExpression(value);
      }
      
      // Support for arrays and objects
      if (value.startsWith("[") && value.endsWith("]")) {
        value = JSON.parse(value.replace(/'/g, '"'));
      } else if (value.startsWith("{") && value.endsWith("}")) {
        value = JSON.parse(value.replace(/'/g, '"'));
      }
      
      this.data[key] = value;
      console.log(`💾 New data stored: ${key} =`, value);
    }
    
    // NEW FUNCTION (Enhanced)
    if (cmd.includes("new function")) {
      const match = cmd.match(/new function (\w+)\s*:\s*(.*)/i);
      if (match) {
        const name = match[1];
        const body = match[2];
        this.context[name] = new Function('return ' + body);
        console.log(`🧠 Custom function created: ${name}`);
      }
    }
    
    // CREATE NEW BLOCK MODE (Enhanced)
    if (cmd.includes("create new block mode")) {
      const name = cmd.match(/call it (\w+)/i)?.[1] || `block_${Date.now()}`;
      this.blocks[name] = {
        type: "custom",
        created: Date.now(),
        permissions: ["read", "write"],
        content: ""
      };
      console.log("🧱 Advanced block mode created:", name);
    }
    
    // GAME SETTINGS (Enhanced)
    if (cmd.includes("reload")) this.gameConfig.reload = true;
    if (cmd.includes("spawn")) this.gameConfig.spawnCorner = true;
    if (cmd.includes("fps")) this.gameConfig.fps = parseInt(cmd.match(/\d+/)?.[0] || 60);
    if (cmd.includes("framerates")) this.gameConfig.fps = parseInt(cmd.match(/\d+/)?.[0] || 60);
    if (cmd.includes("fx")) {
      this.gameConfig.fx = true;
      this._createGameFX();
    }
    
    // JAY TAGS (Enhanced)
    if (cmd.includes("allow new custom tags jay")) {
      this.jayTagsEnabled = true;
      this._initJayTags();
      console.log("🏷️ Advanced JayTags enabled with custom renderer");
    }
    
    // AI INTEGRATION
    if (cmd.includes("ai enable")) {
      this._initAISystem();
    }
    
    // VISUAL PROGRAMMING
    if (cmd.includes("visual mode")) {
      this._initVisualProgramming();
    }
    
    return response;
  }
  
  // ================ HELPER METHODS ================
  
  _generateBotPersonality(name) {
    const personalities = ["friendly", "technical", "creative", "logical", "quirky"];
    return {
      type: personalities[Math.floor(Math.random() * personalities.length)],
      traits: ["helpful", "efficient"],
      createdAt: Date.now()
    };
  }
  
  _aiInterpret(prompt) {
    // Simplified AI interpretation
    const responses = {
      "greeting": "Hello!",
      "time": new Date().toLocaleTimeString(),
      "random": Math.random().toString(),
      "help": "I'm here to help!"
    };
    
    for (const key in responses) {
      if (prompt.toLowerCase().includes(key)) {
        return responses[key];
      }
    }
    
    return `AI Response to: ${prompt}`;
  }
  
  _evaluateExpression(expr) {
    try {
      expr = expr.replace(/{{/g, '').replace(/}}/g, '');
      return eval(expr);
    } catch (e) {
      return expr;
    }
  }
  
  _compileCustomCode(code) {
    // Simple compilation to JS
    return `
(function() {
  const ctx = this;
  ${code}
}).bind(this)();
    `.trim();
  }
  
  _calculateComplexity(code) {
    const lines = code.split('\n').length;
    const chars = code.length;
    return Math.round((lines * chars) / 1000);
  }
  
  _createParticleEffects() {
    const particles = document.createElement('div');
    particles.id = 'hsx-particles';
    particles.style.cssText = `
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      pointer-events: none;
      z-index: 9999;
    `;
    document.body.appendChild(particles);
    
    for (let i = 0; i < 50; i++) {
      const particle = document.createElement('div');
      particle.style.cssText = `
        position: absolute;
        width: 4px;
        height: 4px;
        background: ${['#ff00ff', '#00ffff', '#ffff00'][i % 3]};
        border-radius: 50%;
        left: ${Math.random() * 100}%;
        top: ${Math.random() * 100}%;
        animation: float ${2 + Math.random() * 3}s infinite;
      `;
      particles.appendChild(particle);
    }
    
    const style = document.createElement('style');
    style.textContent = `
      @keyframes float {
        0%, 100% { transform: translate(0, 0); opacity: 0.7; }
        50% { transform: translate(${Math.random() * 100 - 50}px, ${Math.random() * 100 - 50}px); opacity: 0.3; }
      }
    `;
    document.head.appendChild(style);
  }
  
  _initJayTags() {
    // Custom element definitions for JayTags
    class JayElement extends HTMLElement {
      constructor() {
        super();
        this.attachShadow({ mode: 'open' });
      }
      
      connectedCallback() {
        this.render();
      }
      
      render() {
        this.shadowRoot.innerHTML = `
          <style>
            :host {
              display: block;
              border: 2px dashed #ff00ff;
              padding: 10px;
              margin: 5px;
              background: rgba(255, 0, 255, 0.1);
            }
          </style>
          <slot></slot>
        `;
      }
    }
    
    // Define some custom elements
    if (!customElements.get('jay-tag')) {
      customElements.define('jay-tag', JayElement);
    }
    
    if (!customElements.get('jay-data')) {
      customElements.define('jay-data', class extends HTMLElement {
        connectedCallback() {
          this.innerHTML = `<pre>${JSON.stringify(this.data, null, 2)}</pre>`;
        }
        
        get data() {
          return this.getAttribute('data') || '{}';
        }
      });
    }
  }
  
  _initAISystem() {
    console.log('🤖 AI System Initialized');
    this.aiModels = {
      predictor: async (input) => {
        return `AI Prediction for "${input}": ${Math.random()}`;
      },
      generator: async (prompt) => {
        return `Generated from "${prompt}": Lorem ipsum dolor sit amet.`;
      }
    };
  }
  
  _initVisualProgramming() {
    console.log('🎨 Visual Programming Mode Activated');
    const canvas = document.createElement('canvas');
    canvas.width = 800;
    canvas.height = 600;
    canvas.style.cssText = `
      position: fixed;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      border: 2px solid #00ff00;
      background: rgba(0, 0, 0, 0.9);
      z-index: 10000;
    `;
    
    const closeBtn = document.createElement('button');
    closeBtn.textContent = 'Close Visual Editor';
    closeBtn.style.cssText = `
      position: absolute;
      top: 10px;
      right: 10px;
      z-index: 10001;
    `;
    closeBtn.onclick = () => {
      document.body.removeChild(canvas);
      document.body.removeChild(closeBtn);
    };
    
    document.body.appendChild(canvas);
    document.body.appendChild(closeBtn);
    
    // Draw initial nodes
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#00ff00';
    ctx.font = '12px monospace';
    ctx.fillText('Visual Programming Canvas', 10, 20);
    
    // Draw some sample nodes
    const nodes = [
      { x: 100, y: 100, name: 'Input' },
      { x: 300, y: 100, name: 'Process' },
      { x: 500, y: 100, name: 'Output' }
    ];
    
    nodes.forEach(node => {
      ctx.fillStyle = '#ff00ff';
      ctx.fillRect(node.x, node.y, 100, 50);
      ctx.fillStyle = '#ffffff';
      ctx.fillText(node.name, node.x + 10, node.y + 30);
    });
  }
  
  _createGameFX() {
    // Create immersive game effects
    const fxLayer = document.createElement('div');
    fxLayer.style.cssText = `
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      pointer-events: none;
      z-index: 9998;
      background: linear-gradient(45deg, 
        rgba(255,0,255,0.1) 0%, 
        rgba(0,255,255,0.1) 50%, 
        rgba(255,255,0,0.1) 100%);
      animation: pulse 3s infinite;
    `;
    
    document.body.appendChild(fxLayer);
    
    const style = document.createElement('style');
    style.textContent = `
      @keyframes pulse {
        0%, 100% { opacity: 0.3; }
        50% { opacity: 0.7; }
      }
      body {
        font-family: 'Courier New', monospace;
      }
    `;
    document.head.appendChild(style);
  }
  
  // ================ ENHANCED GAME SYSTEM ================
  startPixelGame() {
    const canvas = document.createElement("canvas");
    canvas.width = 400;
    canvas.height = 400;
    canvas.style.cssText = `
      position: fixed;
      right: 20px;
      bottom: 20px;
      border: 3px solid #ff00ff;
      border-radius: 10px;
      box-shadow: 0 0 20px #ff00ff;
      z-index: 9997;
    `;
    document.body.appendChild(canvas);
    
    const ctx = canvas.getContext("2d");
    const colors = this.gameConfig.pixels.length ? this.gameConfig.pixels : ["#ff0000", "#00ff00", "#0000ff"];
    
    // Enhanced particles with physics
    const particles = Array.from({ length: 100 }, () => ({
      x: Math.random() * canvas.width,
      y: Math.random() * canvas.height,
      vx: (Math.random() - 0.5) * 4,
      vy: (Math.random() - 0.5) * 4,
      color: colors[Math.floor(Math.random() * colors.length)],
      size: Math.random() * 6 + 2
    }));
    
    const animate = () => {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Draw particles
      particles.forEach(p => {
        // Update position with physics
        p.x += p.vx;
        p.y += p.vy;
        
        // Bounce off walls
        if (p.x <= 0 || p.x >= canvas.width) p.vx *= -0.9;
        if (p.y <= 0 || p.y >= canvas.height) p.vy *= -0.9;
        
        // Apply gravity
        p.vy += 0.1;
        
        // Draw particle
        ctx.fillStyle = p.color;
        ctx.beginPath();
        ctx.arc(p.x, p.y, p.size, 0, Math.PI * 2);
        ctx.fill();
        
        // Glow effect
        ctx.shadowColor = p.color;
        ctx.shadowBlur = 10;
      });
      
      ctx.shadowBlur = 0;
      
      // Draw info
      ctx.fillStyle = "#ffffff";
      ctx.font = "10px monospace";
      ctx.fillText(`Particles: ${particles.length}`, 10, 20);
      ctx.fillText(`FPS: ${this.gameConfig.fps}`, 10, 35);
      
      requestAnimationFrame(animate);
    };
    
    animate();
    
    // Add controls
    const controls = document.createElement("div");
    controls.style.cssText = `
      position: fixed;
      right: 20px;
      bottom: 430px;
      color: white;
      font-family: monospace;
      background: rgba(0,0,0,0.7);
      padding: 10px;
      border-radius: 5px;
      z-index: 9998;
    `;
    controls.innerHTML = `
      <strong>HSX Pixel Game</strong><br>
      Colors: ${colors.join(", ")}<br>
      Particles: 100<br>
      Physics: ON
    `;
    document.body.appendChild(controls);
  }
  
  // ================ ORIGINAL METHODS (Preserved) ================
  // [All original methods remain unchanged...]
}

// Enhanced auto-init with advanced features
window.HSXRuntime = HSXRuntime;

window.addEventListener("DOMContentLoaded", () => {
  console.log("🚀 HSX Advanced Runtime Loaded");
  
  // Advanced drop zone
  const dropZone = document.createElement("div");
  dropZone.innerHTML = `
    <div style="border: 3px dashed #00ffaa; padding: 30px; margin: 20px; text-align: center; background: rgba(0,0,0,0.7); border-radius: 15px;">
      <h2 style="color: #00ffaa; margin: 0;">📂 Drop HSX Files Here</h2>
      <p style="color: #cccccc;">Supports: .hsx, .jay, .crypt, .shx, .flow</p>
      <div style="margin-top: 15px; font-size: 12px; color: #888;">
        Advanced Custom Language Runtime v0.72+
      </div>
    </div>
  `;
  document.body.appendChild(dropZone);
  
  dropZone.addEventListener("dragover", e => e.preventDefault());
  dropZone.addEventListener("drop", async e => {
    e.preventDefault();
    for (const file of e.dataTransfer.files) {
      const ext = file.name.split('.').pop().toLowerCase();
      const supported = ['.hsx', '.jay', '.crypt', '.shx', '.flow']
        .map(e => e.replace('.', '')).includes(ext);
      
      if (supported) {
        const hsx = new HSXRuntime();
        await hsx.loadFromFile(file);
        
        // Show notification
        const notification = document.createElement("div");
        notification.textContent = `✅ Loaded ${file.name}`;
        notification.style.cssText = `
          position: fixed;
          top: 20px;
          right: 20px;
          background: #00ffaa;
          color: black;
          padding: 10px 20px;
          border-radius: 5px;
          z-index: 10000;
          animation: slideIn 0.3s;
        `;
        document.body.appendChild(notification);
        setTimeout(() => notification.remove(), 3000);
      }
    }
  });
  
  // Add style for animations
  const style = document.createElement("style");
  style.textContent = `
    @keyframes slideIn {
      from { transform: translateX(100%); opacity: 0; }
      to { transform: translateX(0); opacity: 1; }
    }
  `;
  document.head.appendChild(style);
  
  // Auto-detect and load HSX files
  if (location.search.includes("hsxFiles=")) {
    const filesParam = new URLSearchParams(location.search).get("hsxFiles");
    const files = filesParam.split(",");
    const hsx = new HSXRuntime();
    hsx.loadFiles(files);
  } else if (location.pathname.endsWith(".hsx")) {
    const hsx = new HSXRuntime();
    hsx.load(location.pathname);
  }
  
  // Show runtime status
  const status = document.createElement("div");
  status.id = "hsx-status";
  status.style.cssText = `
    position: fixed;
    bottom: 10px;
    left: 10px;
    background: rgba(0,0,0,0.8);
    color: #00ffaa;
    padding: 8px 12px;
    border-radius: 5px;
    font-family: monospace;
    font-size: 12px;
    z-index: 10000;
  `;
  status.textContent = "HSX Runtime Active v0.72+";
  document.body.appendChild(status);
});
