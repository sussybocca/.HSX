# HSX Programming Language — Standalone Beta (C++)

## What is HSX?

HSX is a brand new programming language designed from the ground up for speed. Instead of verbose syntax and complex boilerplate, HSX uses a clean command-based approach where you write instructions as text commands wrapped in unique symbols.

Every HSX program starts with `HSX:` — that's it. No imports, no main function, no unnecessary ceremony. Just write what you want to happen.

---

## This Build — First Real C++ Standalone Beta

This is **not** a JavaScript frontend prototype or a simulated playground. This is the **first real C++ standalone beta build** of the HSX language.

| Feature | Status |
|---------|--------|
| Language | HSX |
| Implementation | Pure C++17 |
| Compilation | Native binary (.exe) |
| No dependencies | Fully self-contained interpreter |
| Frontend JS version | Exists separately |
| This build | Standalone C++ beta |

Yes, HSX has existed in other forms before — but this is the first time it's compiled and run as a real, native executable built from C++ source. No browser, no Node.js, no tricks. Just a real binary.

---

## Can HSX Run in a Browser?

**Not yet — and that's intentional.**

This C++ standalone is built for desktop execution. A browser-based version would require either:

- A WebAssembly (WASM) compilation target
- A C++ → JavaScript transpiler
- A custom cloud execution platform

As HSX becomes more stable and the feature set locks in, browser support is absolutely on the roadmap. The goal is to eventually allow `.hsx` files to run anywhere — desktop, server, or browser.

---

## Current Features (Beta)

### Core Language
HSX: Required prefix for every line
V_A_R.1 =10.0 Variable assignment (decimals required)
S_T_R.1 ""hello"" String variable
L_S_T.1 [1.0, 2.0, 3.0] List variable
{GT} Get user input
!LP! =5.0 Loop (repeat N times)
@WHL@ V_A_R.1 =10.0 While loop
C>...>C = F>...>F error E>...>E =Print If/Else condition
F_N_C> myFunc >F_N_C Function definition
{R_T_N} V_A_R.1 Return value
/C_O_M/ ""comment"" / Comment

text

### Frontend Commands
+B_A_I_T+ ""Click Me"" Button
+T_X_T+ ""Hello"" Text display
+I_M_G+ ""image.png"" Image
+L_N_K+ ""https://..."" Link
+D_I_V+ Container open
+/D_I_V+ Container close
+F_R_M+ Form open
+/F_R_M+ Form close
+L_S_T+ ""Item1"", ""Item2"" List
+T_B_L+ Table open
+/T_B_L+ Table close

text

### Backend Commands
{I_N_U_T} ""Prompt"" Input field
{F_I_L} ""file.txt"" File read
{A_P_I} ""https://api..."" HTTP GET request
{D_B_C} ""dbname"" Database connection
{S_R_V} =8080.0 TCP server start
{J_S_N} {G_T} JSON parse
{Q_R_Y} ""SELECT * FROM..."" Database query
{S_C_K} Socket connection

text

### Standard Library
MATH.ADD, MATH.SUB, MATH.MUL, MATH.DIV
MATH.SQRT, MATH.POW, MATH.ABS, MATH.MOD
STR.CONCAT, STR.UPPER, STR.LOWER, STR.LEN
STR.SPLIT, STR.REPLACE
LIST.APPEND, LIST.SUM, LIST.LEN, LIST.MAX
LIST.MIN, LIST.SORT, LIST.REVERSE, LIST.GET
CONV.TOSTRING, CONV.TONUMBER

text

---

## How to Run

### Windows

```powershell
./hsx.exe myprogram.hsx
./hsx.exe myprogram.hsx --debug
./hsx.exe myprogram.hsx --optimize
Build from Source
powershell
g++ -std=c++17 main.cpp -o hsx.exe -lws2_32
Requirements:

C++17 compatible compiler (GCC 8+, MSVC 2019+, Clang 7+)

Windows: Winsock2 (included with MinGW/MSVC)

Example Program
text
HSX: *V_A_R.1 =10.0*
HSX: C>*V_A_R.1 =10.0*>C = F>Yes>F error E>No>E =Print
HSX: !LP! =3.0
HSX: +T_X_T+ ""Hello HSX World""
HSX: {I_N_U_T} ""Enter your name:""
HSX: +B_A_I_T+ ""Click Me""
HSX: {F_I_L} ""test.hsx""
HSX: /C_O_M/ ""This is a comment"" /
Output:

text
Yes
Hello HSX World
Hello HSX World
Hello HSX World
Enter your name:isaiah
[Input received: isaiah]
[ BUTTON: Click Me ]
Press ENTER to click...
Button clicked!
[file contents printed]
What "Beta" Means
This is a beta release. Here's what that means:

✅ Core language syntax is stable and functional

✅ Parser, lexer, interpreter, optimizer, debugger all working

✅ Frontend and backend command sets implemented

✅ Standard library with 25+ built-in functions

⚠️ Some edge cases may not be fully handled

⚠️ Error messages may improve in future versions

⚠️ Browser/WebAssembly support not yet available

⚠️ Package ecosystem not yet built

Roadmap
□ WebAssembly compilation for browser support
□ HSX package manager (hpm)
□ VS Code syntax highlighting extension
□ Multi-file project support
□ Async/await for backend operations
□ Full SSL/TLS for HTTP requests
□ Database drivers (SQLite, PostgreSQL)
□ GUI desktop runtime
Repository
This is the official standalone C++ implementation of the HSX programming language. The frontend JS playground version exists separately.

HSX — Commands, not code. Speed by design.
