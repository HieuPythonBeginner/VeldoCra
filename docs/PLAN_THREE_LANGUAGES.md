# Veldanava Multi-Language Plan

## Purpose
To be defined.

## Immediate Priorities (Near Term)

### 1. Stabilize and Document the IR
- Finalize IR node set in `src/middle/ir/` (instructions, types, values).
- Ensure IR is expressive enough to represent:
  - Web: DOM manipulation, event handlers, async/await
  - Game: ECS systems, components, game ticks, physics queries
  - Logic: fact bases, rule chaining, query execution
- Provide thorough documentation in `docs/IR_REFERENCE.md`.
- Make IR extensible via metadata/annotations (for domain-specific info).

### 2. Define the Frontend → IR Interface
- Specify a clear API that each language frontend must implement to generate IR.
  - Example interface (C++):
    ```cpp
    class IRGenerator {
    public:
        virtual void emitFunction(FuncDecl*) = 0;
        virtual void emitBinaryOp(BinaryOpKind, Value*, Value*) = 0;
        virtual void emitCall(Value* callee, ArrayRef<Value*> args) = 0;
        // ... other emits for control flow, memory, etc.
    };
    ```
- This interface lives in `src/frontend/irgen/IRGenerator.h`.
- Frontends (Veldora, Velgrynd, Velzath) will implement it; Veldanava core remains unchanged.

### 3. Build a Configurable Hybrid Block Parser Framework
- Extract the hybrid block logic (support for `{}` blocks, indented blocks, and mandatory semicolon terminators) into a reusable utility or base class.
- Provide knobs to enable/disable:
  - Braces (`{}`) vs. indent-only blocks
  - Statement terminator (`;` required/optional)
  - Indentation sensitivity (on/off)
- This allows each language frontend to instantiate a parser suited to its syntax while sharing common block-handling logic.
- Place in `src/frontend/parser/HybridBlockHelper.{h,cpp}`.

### 4. Develop Minimal Viable Frontends for Each Language
- Start with a tiny subset (e.g., variable declarations, function definitions, basic expressions, hybrid blocks).
- Use the shared lexer/tokenizer infrastructure (with per-language keyword tables).
- Generate IR via the interface from step 2.
- Validate by compiling simple programs and inspecting the resulting IR (via `veldenava check --dump-ir`).

### 5. Establish End-to-End Testing
- Create test programs for each language in `tests/languages/`.
- Ensure `veldenava check <file>` passes (no parse/semantic errors).
- Add IR dump checks to catch regressions.

## Mid-Term Goals

### 6. Shared Optimizations
- Implement IR-level passes (dead code elimination, constant propagation, inlining, etc.) that benefit all languages.
- Design passes to be independent of source language semantics.

### 7. Backend Enhancements
- Optimize the VM and native codegen for common patterns seen across the three domains (e.g., tight loops for games, async callbacks for web).
- Consider adding domain-specific backend features if needed (e.g., WASM export conventions for Veldora).

### 8. Tooling Integration
- Editor syntax highlighting, LSP, and debugging support can be built upon the shared compiler infrastructure.
- Leverage the compiler’s error reporting and AST for IDE features.

## Long-Term Vision

### 9. Self-Hosting for All Languages
- Eventually, each language’s standard library and toolchain can be written in itself and compiled by Veldanava.
- This validates the compiler’s robustness and encourages ecosystem growth.

### 10. Language Interoperability
- With a common IR, it becomes feasible to define foreign function interfaces (FFI) allowing, for example:
  - Veldora to call Velgrynd-based game engines.
  - Velzath to invoke Veldora UI components for visualization.
  - Velgrynd to use Velzath reasoners for AI decision-making.

## How to Proceed Now
1. **Finish the IR documentation** and share it with the team.
2. **Implement the IRGenerator interface** and a simple test frontend (e.g., a mini “ToyLang”) to validate the contract.
3. **Refactor the current hybrid block parser** into the reusable framework described in step 3.
4. Once the above are stable, bootstrap the three language frontends incrementally.

By following this plan, Veldanava will evolve into a flexible compiler platform rather than a monolith tied to a single syntax, unlocking the power of domain-specific languages while retaining the benefits of a shared, highly optimized backend.