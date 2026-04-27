# Veldanava Standard Library API   
**Status:** All functions implemented and linked to compiler

## 🏛️ Overview

Thư viện chuẩn VeldoCra với naming theo "Soul/Magic Theme" để tạo identity độc đáo. Tất cả functions tự động available, không cần import.

## 📥 I/O - The Echo System

### Output Functions
```cpp
scribe("Hello World")           // Print string
scribe(42)                      // Print number
emit(3.14)                      // Print value (alternative to flow)
reveal(my_object)               // Debug dump (future)
```

### Input Functions (Future)
```cpp
listen_str()                    // Read string
listen_int()                    // Read int
listen_val()                    // Read float
listen_logic()                  // Read bool
```

## 📦 Collections - The Nexus System

### Sequence (Vector/Array)
```cpp
seq = new Sequence()            // Create sequence
seq.push(item)                  // Add item
item = seq.pop()                // Remove last, return it
count = seq.extent()            // Get size
item = seq.peek(index)          // Get item at index
seq.mutate(index, new_val)      // Set item at index
seq.graft(index, item)          // Insert at index
```

### Registry (Map/Dict)
```cpp
reg = new Registry()            // Create registry
reg.bind("key", value)          // Put key-value
value = reg.fetch("key")        // Get value
exists = reg.exists("key")      // Check if key exists
reg.sever("key")                // Remove key
```

### Collective (Set)
```cpp
coll = new Collective()         // Create collective
coll.enroll(item)               // Add item
coll.dismiss(item)              // Remove item
exists = coll.has(item)         // Check membership
coll.merge(other_coll)          // Union
coll.mesh(other_coll)           // Intersect
count = coll.size()             // Get size
coll.clear()                    // Clear all
```

## 🔢 Math - The Calculus Core

### Basic Math
```cpp
result = abs_val(-5)            // 5 (absolute value)
result = root(16)               // 4.0 (square root)
result = exp_val(2, 3)          // 8 (power)
result = floor_val(3.7)         // 3 (floor)
result = ceiling_val(3.2)       // 4 (ceiling)
```

### Trigonometric
```cpp
result = sin_val(PI/2)          // 1.0
result = cos_val(0)             // 1.0
result = tan_val(PI/4)          // 1.0
result = asin_val(1)            // 1.57...
result = acos_val(0)            // 1.57...
```

### Utilities
```cpp
clamped = confine(val, 0, 100) // Clamp value
lerped = bridge(a, b, 0.5)     // Linear interpolation
```

### Random
```cpp
rand_float = chaos_float()            // 0.0 to 1.0
rand_int = chaos_int(1, 10)     // 1 to 10
rand_choice = chaos_pick(array) // Random array element
```

### Constants
```cpp
PI                              // 3.14159... (for general use)
TAU                             // 6.28318... (2*PI, for math enthusiasts)
EULER                           // 2.71828... (Euler's number)
```

## 📝 Strings - The Lexicon Operations

### Info
```cpp
len = lex_span("hello")         // 5 (length)
empty = lex_void("")            // true (is empty)
char = lex_glyph_at("hello", 0) // 'h' (char at index)
```

### Modification
```cpp
upper = lex_elevate("Hello")    // "HELLO"
lower = lex_depress("HELLO")    // "hello"
trimmed = lex_sanitize("  hello  ") // "hello"
replaced = lex_swap("hello world", "world", "universe") // "hello universe"
```

### Search
```cpp
found = lex_find("hello", "ell") // true (contains)
starts = lex_anchors_with("hello", "he") // true (starts with)
ends = lex_trails_with("hello", "lo") // true (ends with)
```

### Conversion
```cpp
num = lex_parse_int("42")       // 42
val = lex_parse_float("3.14")   // 3.14
logic = lex_parse_logic("true") // true
```

### Advanced
```cpp
parts = lex_shatter("a,b,c", ",") // ["a", "b", "c"]
joined = lex_weave(["a", "b", "c"], "-") // "a-b-c"
// format future: lex_reality_weave("Name: {}", name)
```

## 🛠️ Implementation Notes

- **C++ Implementation:** `src/stdlib/stdlib.h` and `src/stdlib/stdlib.cpp`
- **Veldanava Implementation:** `src/veldanc/stdlib.veldan`
- **Linking:** Auto-linked to compiler, available in all VeldoCra code
- **Naming:** Soul/Magic theme for unique identity
- **Types:** Generic via `void*` in C++, native types in Veldanava
- **Error Handling:** Parse functions return defaults on invalid input
- **Memory Operations:**
  - `ee(ptr)` → `free(ptr)` - Manual memory deallocation (function call like C)
  - `ee_inf_layers obj` → `delete obj` - Object destruction (unary operator)
  - `ee_inf_layers[] arr` → `delete[] arr` - Array cleanup (unary operator)
  - `forge(size)` → `malloc(size)` - Memory allocation (function call)
- **Status:** Both versions implemented for cross-compilation support

## 🎯 Usage Examples

```cpp
// Hello World with collections
seq = new Sequence()
seq.push("Hello")
seq.push("World")
scribe(lex_weave(seq, " "))  // "Hello World"

// Math calculations
area = exp(chaos_int(1, 10), 2) * PI
scribe("Area: ")
emit(area)

// String processing
text = "  VeldoCra is awesome!  "
clean = lex_sanitize(text)
upper = lex_elevate(clean)
scribe(upper)  // "VELDOCRA IS AWESOME!"

// Registry usage
config = new Registry()
config.bind("version", "2.0")
config.bind("theme", "soul")
version = config.fetch("version")
scribe("Version: ")
scribe(version)

// Memory management
av buffer = forge(1024)      // Allocate 1KB (raw memory)
ee(buffer)                   // Free raw memory

av buffer2 = clm(2048)       // malloc 2KB
ee(buffer2)                  // Free with free()

// Object management (future with OOP)
av obj = manifest MyClass()  // Create object (calls constructor)
ee_inf_layers obj            // Delete object (calls destructor + free)

// Array management
av arr = forge(100 * 4)      // Allocate array memory
ee(arr)                      // Free array memory

av arr2 = pur(10, 8)         // calloc 10 elements of 8 bytes
ee_inf_layers[] arr2         // Delete array (cleanup + free)
```