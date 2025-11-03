class HSXEngine {
  constructor(containerId) {
    this.container = document.getElementById(containerId) || document.body;
    this.components = {};
    this.renders = [];
  }

  parseLine(line) {
    line = line.trim();
    if(line.startsWith("hsx comment")) {
      console.log("💬", line.match(/"(.+)"/)?.[1] || "");
    } 
    else if(line.startsWith("hsx define component")) {
      const name = line.split(" ")[3];
      this.currentComponent = { name, content: "" };
    } 
    else if(line === "hsx end") {
      this.components[this.currentComponent.name] = this.currentComponent.content;
      this.currentComponent = null;
    } 
    else if(this.currentComponent) {
      this.currentComponent.content += line + "\n";
    } 
    else if(line.startsWith("hsx render")) {
      const name = line.split(" ")[2];
      this.renders.push(name);
    } 
    else if(line.startsWith("hsx attach")) {
      this.handleMedia(line);
    } 
    else if(line.startsWith("{js")) {
      this.jsBuffer = "";
      this.inJS = true;
    } 
    else if(line.startsWith("}") && this.inJS) {
      this.inJS = false;
      this.runJS(this.jsBuffer);
    } 
    else if(this.inJS) {
      this.jsBuffer += line + "\n";
    }
  }

  handleMedia(line) {
    const [type, url] = line.match(/"(.*?)"/g).map(s => s.replace(/"/g,""));
    if(line.includes("image")) {
      const img = document.createElement("img");
      img.src = url;
      this.container.appendChild(img);
    } else if(line.includes("video")) {
      const vid = document.createElement("video");
      vid.src = url;
      vid.controls = true;
      this.container.appendChild(vid);
    }
  }

  runJS(code) {
    try { new Function(code)(); }
    catch(e) { console.error("JS execution error:", e); }
  }

  renderAll() {
    this.renders.forEach(name => {
      if(this.components[name]) {
        const wrapper = document.createElement("div");
        wrapper.innerHTML = this.components[name];
        this.container.appendChild(wrapper);
      }
    });
  }

  run(hsxText) {
    hsxText.split("\n").forEach(line => this.parseLine(line));
    this.renderAll();
  }
}

// Usage:
// const engine = new HSXEngine("app");
// engine.run(hsxFileContent);
