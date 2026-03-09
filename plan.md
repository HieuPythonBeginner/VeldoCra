# VeldoCra Master Development Plan

Author: Dr. Bright
Language: VeldoCra (.vel)
Compiler: velc
Implementation Language: C++23
Primary Platform: Linux
Architecture: x86-64

---

# CORE PRINCIPLE

VeldoCra must be built in correct order:

DO NOT build VM first
DO NOT build optimizer first
DO NOT build GC first

Correct order:

1. CLI Driver
2. Lexer
3. Parser
4. AST
5. Basic VM
6. Type System
7. IR (SSA)
8. Ownership System
9. Native Codegen
10. Self-hosting

---

# PHASE 0 — PROJECT FOUNDATION (DAY 1)

Goal: Create minimal compiler executable

Create folder structure:

```
VeldoCra/
│
├── CMakeLists.txt
│
├── docs/
│   └── plan.md
│
├── src/
│   ├── main.cpp
│   ├── driver/
│   │   └── driver.cpp
│   │   └── driver.h
│   │
│   ├── frontend/
│   │   ├── lexer/
│   │   ├── parser/
│   │   └── ast/
│   │
│   ├── middle/
│   │   ├── types/
│   │   ├── ownership/
│   │   └── ir/
│   │
│   └── backend/
│       ├── vm/
│       └── codegen/
│
└── tests/
    └── test.vel
```

---

# PHASE 1 — CLI DRIVER (DAY 1–2)

Goal: velc executable works

Command targets:

```
velc build file.vel
velc run file.vel
velc check file.vel
```

Implement:

src/main.cpp

Responsibilities:

* parse arguments
* load source file
* call compiler pipeline

Output example:

```
VeldoCra Compiler v0.0.1
Building file.vel...
Success.
```

NO compiler logic yet.

Only driver.

---

# PHASE 2 — SOURCE FILE LOADER (DAY 2)

Create:

```
src/frontend/source/
```

Files:

```
source.h
source.cpp
```

Responsibilities:

* load .vel file into memory
* store:

Create struct:

```
struct SourceFile
{
    const char* data;
    size_t size;
    const char* filename;
};
```

---

# PHASE 3 — LEXER (DAY 3–5)

Goal: convert text → tokens

Create:

```
src/frontend/lexer/
```

Files:

```
token.h
lexer.h
lexer.cpp
```

Token types:

```
Identifier
Number
String
Newline
Indent
Dedent
BraceOpen
BraceClose
ParenOpen
ParenClose
Operator
EOF
```

Lexer responsibilities:

Input:

```
x := 10
```

Output:

```
IDENT(x)
OP(:=)
NUMBER(10)
EOF
```

Must support:

• indentation tracking
• brace tracking
• fast scanning

This is FIRST REAL compiler component.

---

# PHASE 4 — AST (DAY 5–7)

Create:

```
src/frontend/ast/
```

Files:

```
ast.h
ast.cpp
```

Example nodes:

```
Node
Expression
Statement
BinaryExpr
LiteralExpr
VariableExpr
```

Use arena allocator.

NO heap allocation per node.

---

# PHASE 5 — PARSER (DAY 7–10)

Create:

```
src/frontend/parser/
```

Files:

```
parser.h
parser.cpp
```

Convert:

tokens → AST

Example:

Input:

```
x := 10
```

Output AST:

```
VariableDecl
 ├── name: x
 └── value: 10
```

Use recursive descent parser.

---

# PHASE 6 — BASIC REGISTER VM (DAY 10–14)

Create:

```
src/backend/vm/
```

Files:

```
vm.h
vm.cpp
instruction.h
```

Instruction example:

```
MOV r1, 10
MOV r2, 20
ADD r3, r1, r2
PRINT r3
```

VM must:

• execute instructions
• manage registers
• run simple programs

---

# PHASE 7 — BASIC CODE GENERATION (DAY 14–18)

Convert AST → VM instructions

Example:

```
x := 10
print(x)
```

becomes:

```
MOV r1, 10
PRINT r1
```

---

# PHASE 8 — TYPE SYSTEM (DAY 18–25)

Create:

```
src/middle/types/
```

Files:

```
type.h
type.cpp
type_checker.cpp
```

Responsibilities:

• type inference
• type checking

Example:

```
x := 10
```

type inferred as:

```
i32
```

---

# PHASE 9 — SSA IR (DAY 25–40)

Create:

```
src/middle/ir/
```

Files:

```
ir.h
ir.cpp
builder.cpp
```

Example:

```
%1 = const 10
%2 = const 20
%3 = add %1, %2
```

This enables optimization.

---

# PHASE 10 — OWNERSHIP SYSTEM (DAY 40–60)

Create:

```
src/middle/ownership/
```

Responsibilities:

Track:

• ownership
• borrow
• lifetimes

Prevent:

• use-after-free
• double free

---

# PHASE 11 — NATIVE CODEGEN (DAY 60–90)

Create:

```
src/backend/codegen/
```

Generate:

x86-64 machine code

Output:

ELF binary (Linux)

---

# PHASE 12 — SELF HOSTING (DAY 90+)

Rewrite compiler in VeldoCra:

```
velc.vel
```

Compile using:

```
velc build velc.vel
```

VeldoCra now self-hosted.

---

# FIRST MILESTONE TARGET

Compiler can run:

```
print("Hello, Tempest")
```

via VM

---

# FINAL MILESTONE TARGET

Compiler can compile itself.

---

# DEVELOPMENT ORDER (STRICT)

Build in this exact order:

1 main.cpp
2 driver
3 source loader
4 lexer
5 token system
6 AST
7 parser
8 VM
9 codegen to VM
10 type checker
11 IR
12 ownership
13 native codegen
14 self hosting

---

# WHAT TO DO RIGHT NOW (NEXT STEP)

START HERE:

Create:

```
src/main.cpp
src/driver/driver.cpp
src/driver/driver.h
```

Make velc executable run.

Nothing else.

---

END OF PLAN
