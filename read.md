You are an elite systems programming language designer and compiler engineer with 20+ years of experience building high-performance, safety-critical languages (Rust, Go, SPARK/Ada, Python internals, C/C++, Zig, Odin, and HolyC from TempleOS).
I want you to build the pure core skeleton / foundational framework for a brand new systems programming language called VeldoCra (the official native language for JuraTempestFederationOS).
STRICT REQUIREMENTS
1. Implementation Language & Performance

The entire framework (lexer, parser, AST, type checker, memory manager, codegen/VM) must be written in modern C++23 (or C++20 if needed), keeping it lightweight and zero-bloat.
Use inline x86-64 Assembly for all performance-critical hot paths (lexer scanning, memory allocator fast path, bytecode dispatch, etc.).
INLINE ASM HOT PATHS (MANDATORY)

- Lexer character classification (SIMD accelerated)
- Arena allocator fast path
- VM dispatch loop (computed goto or asm jump table)
- Atomic primitives for concurrency

Must use x86-64 AT&T or Intel syntax inline assembly.
Must compile with GCC/Clang on Linux/others OS.
Goal: Absolute maximum performance — as fast as or faster than hand-written C/Assembly in real workloads, with zero-cost abstractions.

2. Multi-Layer Safety & Protection (The "Great Sage" Safety System)
Combine the absolute best protection mechanisms from 5 languages to achieve absolute safety + absolute performance:

Rust: Full ownership model, borrow checker, lifetimes, zero-cost memory safety.
SPARK/Ada: Formal verification principles, contracts, prove at compile-time: no runtime errors, no null dereference, no overflow, no data races.
Go: Low-pause concurrent garbage collector + channel-style safe concurrency.
Python: Reference counting + cycle detection GC fallback, excellent dynamic safety & runtime diagnostics.
C++: RAII, smart pointers, bounds checking on all arrays/strings.
The result must be: Maximum safety (no UB, no leaks, no crashes) without sacrificing any performance.

GREAT SAGE SAFETY LAYERS

Layer 1: Compile-Time Memory Safety
- Ownership graph model
- Borrow checking pass
- Lifetime inference engine

Layer 2: Formal Verification Layer
- Design-by-contract syntax (preconditions, postconditions, invariants)
- Static overflow checking
- Null safety enforced at type level

Layer 3: Runtime Safety Layer
- Reference counting with cycle detection
- Optional low-latency concurrent GC mode
- Panic / trap handling system

Layer 4: Concurrency Safety Layer
- Data-race-free by design
- Channel-based concurrency primitive
- Lock-free atomic primitives

All safety must be zero-cost when provably safe at compile-time.
No runtime overhead unless required.

3. Syntax Philosophy (Hybrid & Extremely Readable)

Primary readability goal = Python-level cleanliness (because the creator comes from Python background).
Support both styles seamlessly (user can mix in same file):
Python-style: Significant whitespace (indentation with 4 spaces or 1 tab), using : after control keywords (if, for, while, def, class, etc.).
C/Rust/Go-style: Optional curly braces {} for blocks.

Make indentation the strongly recommended style for normal code (cleaner, less noise).
Allow {} freely when user wants explicit control (deep nesting, one-liners, or personal preference).
Parser must handle both styles perfectly and encourage the more readable Python-style indentation where it improves clarity.

4. Scope — Core Skeleton ONLY
This phase is only the bones/framework. Do NOT implement any language-specific keywords, "soul" features, Tensura theme, or high-level syntax yet.
Must include:

Lexer / Tokenizer (supporting both indentation and brace modes)
Parser (recursive descent or Pratt — flexible for both syntax styles)
AST node definitions
Symbol table & scoping
Foundational type system (static + gradual typing support)
Memory management system (ownership tracking + hybrid GC)
Intermediate Representation or simple bytecode
Basic code generator / bytecode VM skeleton
Excellent error reporting & runtime error handling framework
Project structure with CMake + Makefile.

ARCHITECTURE REQUIREMENT

The compiler must follow a clean multi-stage pipeline:

Source (.vel)
→ Lexer
→ Parser
→ AST
→ Semantic Analyzer
→ Ownership & Borrow Checking Pass
→ Type Inference & Gradual Typing Resolution
→ IR (SSA-based Intermediate Representation)
→ Optimization Passes (DCE, Constant Folding, Inlining, Escape Analysis)
→ Codegen Backend (x86-64)
→ Optional Bytecode VM fallback mode

The IR must be Static Single Assignment (SSA) based.
The VM must be register-based, not stack-based.
The system must support both AOT compilation and JIT execution mode (toggleable).

DATA ORIENTED DESIGN REQUIREMENT

AST nodes must be stored in contiguous memory pools.
No heap allocation per node.
Use arena allocator.

Symbol tables must use flat hash tables with open addressing.
No std::map allowed.

Memory layout must be cache-friendly.
All hot-path structures must be POD where possible.
Avoid virtual dispatch in critical paths.

5. Output Format

First: Full recommended project directory structure + CMakeLists.txt
Then implement every core file one by one with full, clean, well-commented code
After all files: Build instructions + how to compile and run a minimal test program using both indentation and brace style
Explain key design decisions (especially how hybrid syntax and multi-layer safety are implemented without performance penalty)

6. Performance

"Prioritize Data-Oriented Design (DOD) for AST and Symbol Table structures to minimize cache misses. Use SIMD instructions in the lexer via inline assembly for lightning-fast tokenization.

Begin now. Start with the high-level architecture and project structure, then proceed file by file.

7. SELF-HOSTING REQUIREMENT

The compiler architecture must be designed to allow future self-hosting.

This means:

- Clean separation between frontend, IR, and backend.
- No hard dependency on C++-specific runtime behavior.
- IR must be language-neutral and capable of representing VeldoCra itself.
- All compiler logic must be deterministic and reproducible.

The design must allow rewriting the compiler in VeldoCra later without architectural changes.

8. ERROR HANDLING MODEL

- No exceptions.
- Use Result<T, E> style error propagation.
- Compiler errors must show:
    - File
    - Line
    - Column
    - Context snippet
    - Suggestion

Runtime errors must produce structured diagnostic reports.

9. VM ARCHITECTURE REQUIREMENT

The VM must be register-based.

Must include:

- fixed-size register file
- instruction pointer
- register operands (not stack)

Instruction dispatch must use:

- computed goto OR
- inline assembly jump table

NOT switch-case dispatch.

10. OWNERSHIP MODEL REQUIREMENT

Compiler must maintain ownership graph internally.

Each variable must track:

- owner
- borrow state (immutable / mutable)
- lifetime region

Borrow checker must run as separate compiler pass.

Must detect:

- use after free
- double free
- illegal mutable aliasing

11. PRODUCTION-GRADE REQUIREMENT

This compiler must be designed as a real production-grade system programming language compiler.

Not an academic toy compiler.

Not a tutorial implementation.

Design decisions must prioritize:

- performance
- scalability
- maintainability
- future extensibility

12. SSA IR REQUIREMENT

IR must include:

- Basic blocks
- Control flow graph
- Explicit phi nodes
- Register virtual IDs
- Instruction list stored contiguously

IR must be designed for efficient optimization passes.

Must support:

- constant folding
- dead code elimination
- escape analysis

13. TOKEN REPRESENTATION REQUIREMENT

Tokens must be represented in compact POD structs.

Maximum size per token: 16 bytes.

Must include:

- token type (uint16)
- source offset (uint32)
- length (uint16)
- flags (uint16)

Token stream must be stored in contiguous memory array.

14. ARENA ALLOCATOR SPECIFICATION

The arena allocator must:

- Allocate large contiguous memory blocks (at least 1MB per arena chunk)
- Support fast bump allocation (pointer increment only)
- Never free individual objects
- Support bulk reset
- Support alignment (8, 16, 32 bytes)
- Provide zero fragmentation

Allocator must include inline x86-64 fast path using inline assembly.

AST, IR, and Symbol tables must allocate exclusively through arena allocator.

15. SYMBOL TABLE PERFORMANCE REQUIREMENT

Symbol table must use flat hash table with open addressing.

Must include:

- linear probing OR robin hood hashing
- no pointer chasing

Symbol lookup must be O(1) average time.

16. DIAGNOSTIC SYSTEM REQUIREMENT

Compiler must include diagnostic engine.

Each diagnostic must include:

- severity
- message
- source location
- highlighted source snippet
- optional fix suggestion

Diagnostics must be stored in structured form internally.

17. BUILD TARGET REQUIREMENT

Compiler must support:

velc build file.vel → native binary
velc run file.vel → run via VM
velc check file.vel → static analysis only

Must compile on:

Linux (secondary)
Windows (thirdly)
Future support for JuraTempestFederationOS(Unknown but Primary, but don't create JuraTempestFederationOS, it had been deleted permanently irreversible by me, sound sad right?)

17.1. History of this Programming Language Why was it created (I guess? don't notice, read if you want):
HitlerOS "dies" → VeldoraOS comes back to life
VeldoraOS "dies" → JTFOS comes back to life
JTFOS "dies permanently irreversible" → ???OS is about to come back to life <- Low chance
    **LEGACY & ORIGIN (THE REINCARNATION CYCLE):**

    The Ancestry: This language is the spiritual successor of a lineage of fallen Operating Systems: HitlerOS (The Chaos), VeldoraOS (The Storm), and JuraTempestFederationOS (The Fallen Empire).

    The Mission: With the permanent loss of JTFOS, VeldoCra (.vel) is created as a "Singularity" — a language designed to be so powerful and self-sufficient that it can eventually reconstruct a new, unknown OS (???OS) from its own source code.

    The Philosophy: Build it with the weight of a fallen empire and the ambition of a future god.

**Note**
The file extension for the VeldoCra language will be .vel
"The official file extension for the language is .vel. Ensure the lexer and any future tooling (LSP, compiler CLI) recognize and prioritize this extension."