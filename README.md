# Stack Overflow Detection

## Overview
This project demonstrates stack overflow conditions by deliberately causing them in two different ways: using a large fixed-size array allocation and through recursive function calls. It provides insights into stack memory limitations across different platforms and how these limitations manifest when exceeded.

## Build & Run
Clone the repository:
```bash
git clone https://github.com/yourusername/stack_overflow_detection.git
cd stack_overflow_detection
```

Build the project:
```bash
cmake -S . -B build
cmake --build build --config Debug
```

Run the executable with desired mode:
```bash
# For fixed array stack overflow test
./build/stack_overflow --fixed

# For recursive function stack overflow test
./build/stack_overflow --recursive
```

## Testing Modes

### Fixed Array Test (--fixed)
This mode attempts to allocate arrays of increasing sizes directly on the stack:
- Tests allocation sizes from 500KB to 8MB
- When allocation exceeds stack limit (typically 1MB on Windows, 8MB on Linux), stack overflow occurs
- Exception/signal handlers catch the overflow condition instead of crashing

### Recursive Test (--recursive)
This mode causes stack overflow through deep function call recursion:
- Tests recursion depths of 10,000, 50,000, and 100,000 calls
- For each depth, tests three allocation scenarios:
  - No additional allocation (`rec_size = 0`)
  - 5KB array per recursive call (`rec_size = 5000`)
  - 10KB array per recursive call (`rec_size = 10000`)
- Tracks total stack usage throughout recursion

## Example Output

### Fixed Array Test
```
Windows: Stack ~1MB (linker-configurable).
Running --fixed test:
Trying 500000 bytes... Fixed: Allocated 500000 bytes.
Trying 1000000 bytes... Fixed: Allocated 1000000 bytes.
Trying 2000000 bytes... Error: Stack overflow
Trying 4000000 bytes... Error: Stack overflow
Trying 8000000 bytes... Error: Stack overflow

Test done.
```

### Recursive Test
```
Stack limit: 8388608 bytes
Running --recursive test:
Trying depth 10000, 0 bytes... Recursion: Depth 10000, 0 bytes, Total: 320000 bytes.
Recursion: Depth 9999, 0 bytes, Total: 319968 bytes.
[... output truncated ...]
Recursion: Depth 1, 0 bytes, Total: 32 bytes.
Recursion: Completed.

Trying depth 10000, 5000 bytes... Recursion: Depth 10000, 5000 bytes, Total: 50320000 bytes.
Error: Stack limit exceeded

Trying depth 50000, 0 bytes... Recursion: Depth 50000, 0 bytes, Total: 1600000 bytes.
[... output truncated ...]
Recursion: Completed.

Test done.
```

## Stack Overflow Mechanics

### Stack Memory Limitations
- Stack memory is limited by OS/platform settings (typically 1MB on Windows, 8MB on Linux)
- Each thread has its own stack allocation
- Stack space is used for function parameters, return addresses, and local variables

### Why Recursion Doesn't Always Trigger Stack Overflow
Interestingly, deep recursion doesn't always cause stack overflow, especially with minimal local variables. You may observe the program successfully completing thousands or even tens of thousands of recursive calls without error.

This happens because:

1. **Minimal Stack Frame Size**: When no additional arrays are allocated (`rec_size = 0`), each stack frame may only consume 16-32 bytes (just function parameters, return address, and frame pointers).

2. **Compiler Optimizations**: Modern compilers may implement partial optimizations that reduce stack usage, even when full tail call optimization is prevented.

3. **Stack Growth Direction**: Stack typically grows downward in memory, which allows the OS to detect overflow situations before critical memory is corrupted.

4. **Stack Reserve vs. Commit**: On Windows, stack memory is often reserved but not committed until needed, allowing programs to appear to have larger stack space than actually available.

## Platform-Specific Handling

The program implements platform-specific mechanisms for handling stack overflow:
- **Windows**: Uses `SetUnhandledExceptionFilter` to catch `EXCEPTION_STACK_OVERFLOW`
- **Unix-like Systems**: Uses signal handlers with `sigaltstack` to catch `SIGSEGV` caused by stack overflow
