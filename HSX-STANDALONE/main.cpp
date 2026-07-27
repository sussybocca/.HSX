#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Optimizer.h"
#include "HSXtoJS.h"
#include "Debugger.h"
#include "Core.h"
#include "Tokenizer.h"
#include "Compiler.h"
#include "VM.h"

#pragma comment(lib, "ws2_32.lib")

const std::string HSX_RUNTIME_JS = R"===(
window.__hsx=(function(){const store={};const stack=[];let currentEl=document.body;
function push(el){stack.push(currentEl);currentEl=el;}
function pop(){if(stack.length)currentEl=stack.pop();}
function append(el){currentEl.appendChild(el);return el;}
function make(tag,text){const el=document.createElement(tag);if(text)el.textContent=text;return append(el);}
return{store,setVar(k,v){store[k]=v;},getVar(k){return store[k];},push,pop,
text(t){make('p',t);},heading(n,t){make('h'+n,t);},span(t){make('span',t);},
bold(t){const e=make('b');e.textContent=t;},italic(t){const e=make('i');e.textContent=t;},
underline(t){const e=make('u');e.textContent=t;},code(t){const e=make('code');e.textContent=t;},
pre(t){const e=make('pre');e.textContent=t;},blockquote(t){make('blockquote',t);},
br(){append(document.createElement('br'));},hr(){append(document.createElement('hr'));},
button(label,cb){const b=make('button');b.textContent=label;b.onclick=cb;return b;},
buttongroup(n,label){for(let i=0;i<n;i++)this.button(label+' '+(i+1),()=>{});},
input(prompt,cb){const i=make('input');i.placeholder=prompt;const b=this.button('OK',()=>{cb(i.value);});},
textarea(placeholder,rows){const ta=make('textarea');ta.placeholder=placeholder;ta.rows=rows||5;return ta;},
dropdown(opts){const s=make('select');opts.forEach(o=>{const opt=document.createElement('option');opt.textContent=o;s.appendChild(opt);});return s;},
checkbox(label){const c=document.createElement('input');c.type='checkbox';const l=document.createElement('label');l.textContent=label;currentEl.append(c,l);},
radio(label){const r=document.createElement('input');r.type='radio';const l=document.createElement('label');l.textContent=label;currentEl.append(r,l);},
toggle(label){const t=document.createElement('input');t.type='checkbox';t.style.width='40px';t.style.height='20px';const l=document.createElement('label');l.textContent=label;currentEl.append(t,l);},
slider(min,max,val){const s=make('input');s.type='range';s.min=min;s.max=max;s.value=val;return s;},
datepicker(p){const d=make('input');d.type='date';d.placeholder=p;return d;},
colorpicker(def){const c=make('input');c.type='color';c.value=def||'#000000';return c;},
fileupload(label){const f=make('input');f.type='file';const l=document.createElement('label');l.textContent=label;currentEl.append(l,f);},
progress(min,max){const p=make('progress');p.min=min;p.max=max;p.value=min;return p;},
range(min,max,val){return this.slider(min,max,val);},
image(src,w,h){const img=make('img');img.src=src;if(w)img.width=w;if(h)img.height=h;return img;},
video(src){const v=make('video');v.src=src;v.controls=true;return v;},
audio(src){const a=make('audio');a.src=src;a.controls=true;return a;},
icon(name){const i=make('i');i.textContent=name;return i;},
canvas(w,h){const c=make('canvas');c.width=w;c.height=h;return c;},
svg(content){const d=document.createElement('div');d.innerHTML=content;return append(d.firstChild);},
videoEmbed(url){const i=make('iframe');i.src=url;i.width=560;i.height=315;return i;},
map(address){const d=make('div');d.textContent='[Map: '+address+']';return d;},
iframe(src,w,h){const i=make('iframe');i.src=src;i.width=w;i.height=h;return i;},
tableOpen(){push(make('table'));},
tableRow(){push(document.createElement('tr'));currentEl=stack[stack.length-1].appendChild(currentEl);},
tableCell(t){make('td',t);},
list(items){const ul=make('ul');items.forEach(i=>{const li=document.createElement('li');li.textContent=i;ul.appendChild(li);});return ul;},
orderedList(items){const ol=make('ol');items.forEach(i=>{const li=document.createElement('li');li.textContent=i;ol.appendChild(li);});return ol;},
card(title,body,img){const d=make('div');d.style.border='1px solid #ccc';d.style.padding='10px';if(img){const im=document.createElement('img');im.src=img;d.appendChild(im);}if(title){const h=document.createElement('h3');h.textContent=title;d.appendChild(h);}if(body){const p=document.createElement('p');p.textContent=body;d.appendChild(p);}return d;},
badge(t){const b=make('span',t);b.style.background='#333';b.style.color='#fff';b.style.padding='2px 8px';b.style.borderRadius='10px';return b;},
alert(msg){alert(msg);},
modal(title,content){const d=make('div');d.style.cssText='position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);background:white;padding:20px;border:2px solid black;z-index:1000;';d.innerHTML='<h2>'+title+'</h2><p>'+content+'</p><button onclick="this.parentElement.remove()">Close</button>';return d;},
tooltip(t){const s=make('span',t);s.title=t;s.style.borderBottom='1px dotted';return s;},
accordion(title,content){const d=make('details');d.innerHTML='<summary>'+title+'</summary><p>'+content+'</p>';return d;},
tabsOpen(n){const d=make('div');for(let i=0;i<n;i++){const b=document.createElement('button');b.textContent='Tab '+(i+1);d.appendChild(b);}push(d);},
tab(label){const b=document.createElement('button');b.textContent=label;currentEl.appendChild(b);},
style(css){currentEl._style=css;},className(c){currentEl._class=c;},id(id){currentEl.id=id;},
animate(name,dur){currentEl.style.animation=name+' '+dur+'s';},hover(css){currentEl._hoverStyle=css;},
formOpen(action){push(make('form'));currentEl.action=action;},label(t){make('label',t);},
field(name,type){const i=make('input');i.name=name;i.type=type||'text';return i;},
submit(label){const b=make('input');b.type='submit';b.value=label;return b;},
reset(label){const b=make('input');b.type='reset';b.value=label;return b;},
menuOpen(){push(make('nav'));},menuItem(label,href){const a=make('a',label);a.href=href||'#';return a;},
breadcrumb(path){make('p',path);},
pagination(cur,total){const d=make('div');for(let i=1;i<=total;i++){const b=document.createElement('button');b.textContent=i;if(i===cur)b.disabled=true;d.appendChild(b);}return d;},
dropdownMenu(opts){return this.dropdown(opts);},
tabBar(tabs){const d=make('div');tabs.forEach(t=>{const b=document.createElement('button');b.textContent=t;d.appendChild(b);});return d;},
search(placeholder){const i=make('input');i.type='search';i.placeholder=placeholder;return i;},
spacer(n){const d=make('div');d.style.height=(n||1)+'rem';return d;},
pad(n){currentEl.style.padding=(n||1)+'rem';},margin(n){currentEl.style.margin=(n||1)+'rem';},
center(){currentEl.style.textAlign='center';},hide(){currentEl.style.display='none';},show(){currentEl.style.display='';},
scroll(h){currentEl.style.overflowY='auto';currentEl.style.height=(h||400)+'px';},
sticky(){currentEl.style.position='sticky';currentEl.style.top='0';},
fixed(pos){currentEl.style.position='fixed';currentEl.style[pos||'top']='0';},
onClick(cb){currentEl.onclick=cb;},onChange(cb){currentEl.onchange=cb;},onLoad(cb){window.addEventListener('load',cb);},
loopRender(varName,cb){const arr=store[varName]||[];arr.forEach(item=>{cb(item);});},
bind(varName){},model(varName){},
api(url){fetch(url).then(r=>r.json()).then(d=>{this.print(JSON.stringify(d));});},
fileRead(path){fetch(path).then(r=>r.text()).then(d=>{this.print(d);});},
dbConnect(name){console.log('[DB] Connected: '+name);},query(sql){console.log('[SQL] '+sql);},jsonParse(){},
divOpen(){const d=make('div');if(currentEl._style){d.style.cssText=currentEl._style;currentEl._style=null;}if(currentEl._class){d.className=currentEl._class;currentEl._class=null;}push(d);},
section(id){push(make('section'));currentEl.id=id;},headerOpen(){push(make('header'));},footerOpen(){push(make('footer'));},
navOpen(id){push(make('nav'));currentEl.id=id;},mainOpen(){push(make('main'));},asideOpen(){push(make('aside'));},
gridOpen(cols){const d=make('div');d.style.display='grid';d.style.gridTemplateColumns='repeat('+(cols||3)+', 1fr)';push(d);},
flexOpen(){const d=make('div');d.style.display='flex';push(d);},
print(msg){const pre=make('pre');pre.textContent=msg;pre.style.cssText='background:#f4f4f4;padding:5px;border:1px solid #ddd;margin:5px 0;white-space:pre-wrap;';return pre;}
}})();
)===";

void serveBrowserApp(const std::string& appJs, int port) {
    std::string html = 
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
        "<title>HSX App</title><style>body{font-family:sans-serif;margin:20px;}"
        "button{margin:5px;padding:8px 16px;cursor:pointer;}"
        "input{margin:5px;padding:8px;}</style></head><body>"
        "<script>" + HSX_RUNTIME_JS + "</script>"
        "<script>" + appJs + "</script>"
        "</body></html>";

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind to port " << port << "\n";
        return;
    }
    listen(server, 1);
    
    std::cout << "HSX Browser App: http://localhost:" << port << "\n";
    std::cout << "Press Ctrl+C to stop the server.\n";
    
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " + std::to_string(html.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + html;

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;
        
        char buf[4096];
        recv(client, buf, sizeof(buf), 0);
        send(client, response.c_str(), (int)response.size(), 0);
        closesocket(client);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: hsx <file.hsx> [--debug] [--optimize] [--fast] [--browser [port]]\n";
        return 1;
    }

    std::string filename;
    bool debugMode = false;
    bool optimizeMode = false;
    bool fastMode = false;
    bool browserMode = false;
    int browserPort = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debugMode = true;
        else if (arg == "--optimize") optimizeMode = true;
        else if (arg == "--fast") fastMode = true;
        else if (arg == "--browser") {
            browserMode = true;
            if (i + 1 < argc && argv[i+1][0] != '-') {
                browserPort = std::stoi(argv[++i]);
            }
        }
        else filename = arg;
    }

    if (fastMode) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open file " << filename << "\n";
            return 1;
        }
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::string src(fileSize, '\0');
        file.read(&src[0], fileSize);
        file.close();

        auto t1 = std::chrono::high_resolution_clock::now();

        hsx::Arena arena(1024 * 1024);
        hsx::CompiledUnit unit;
        hsx::Compiler compiler(src.c_str(), (uint32_t)fileSize, arena, unit);

        if (!compiler.compile()) {
            std::cerr << "HSX Compilation Error\n";
            return 1;
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        auto compileUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        hsx::VM vm(unit);
        auto t3 = std::chrono::high_resolution_clock::now();
        vm.execute();
        auto t4 = std::chrono::high_resolution_clock::now();
        auto execUs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

        std::cerr << "[HSX Fast] Compiled " << unit.code.size() << " instrs in " 
                  << compileUs << "us | Executed in " << execUs << "us | Total: " 
                  << (compileUs + execUs) << "us\n";
        return 0;
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }

    try {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        std::vector<std::unique_ptr<hsx::ASTNode>> program;
        for (const auto& l : lines) {
            program.push_back(hsx::Parser::parseLine(l));
        }

        if (optimizeMode) {
            hsx::Optimizer::optimize(program);
        }

        if (browserMode) {
            std::string appJs = hsx::HSXtoJS::transpile(program);
            std::cout << "Opening HSX app in browser...\n";
            ShellExecuteA(NULL, "open", 
                ("http://localhost:" + std::to_string(browserPort)).c_str(),
                NULL, NULL, SW_SHOW);
            serveBrowserApp(appJs, browserPort);
        } else if (debugMode) {
            std::unordered_map<std::string, std::vector<std::unique_ptr<hsx::ASTNode>>> funcs;
            std::vector<std::unique_ptr<hsx::ASTNode>> mainProg;
            bool inFunc = false;
            std::string currFunc;
            std::vector<std::unique_ptr<hsx::ASTNode>> currBody;
            for (auto& node : program) {
                if (auto* fd = dynamic_cast<hsx::FunctionDefNode*>(node.get())) {
                    if (inFunc) funcs[currFunc] = std::move(currBody);
                    inFunc = true;
                    currFunc = fd->name;
                    currBody.clear();
                } else if (inFunc) {
                    if (dynamic_cast<hsx::ReturnNode*>(node.get())) {
                        currBody.push_back(std::move(node));
                        funcs[currFunc] = std::move(currBody);
                        inFunc = false;
                    } else {
                        currBody.push_back(std::move(node));
                    }
                } else {
                    mainProg.push_back(std::move(node));
                }
            }
            if (inFunc) funcs[currFunc] = std::move(currBody);
            hsx::Debugger debugger(std::move(mainProg), std::move(funcs));
            debugger.run();
        } else {
            hsx::Runtime::init();
            hsx::Interpreter interpreter;
            interpreter.execute(std::move(program));
            hsx::Runtime::cleanup();
        }

        WSACleanup();
    } catch (const std::exception& e) {
        std::cerr << "HSX Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}