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

## Example Output

### Fixed Array Test
```
Windows: Stack ~1MB (linker-configurable).
Running --fixed test:
Trying  500000  bytes... Fixed: Allocated  500000  bytes.
Trying  1000000  bytes... Fixed: Allocated  1000000  bytes.
Trying  2000000  bytes... 
```

### Recursive Test
```
Recursion: Depth  1017 ,  5000  bytes.
Recursion: Depth  1016 ,  5000  bytes.
Recursion: Depth  1015 ,  5000  bytes.
Recursion: Depth  1014 ,  5000  bytes.
Recursion: Depth  1013 ,  5000  bytes.
...
Recursion: Depth  15 ,  5000  bytes.
Recursion: Depth  14 ,  5000  bytes.
Recursion: Depth  13 ,  5000  bytes.
Recursion: Depth  12 ,  5000  bytes.
Recursion: Depth  11 ,  5000  bytes.
Recursion: Depth  10 ,  5000  bytes.
Recursion: Depth  9 ,  5000  bytes.
Recursion: Depth  8 ,  5000  bytes.
Recursion: Depth  7 ,  5000  bytes.
Recursion: Depth  6 ,  5000  bytes.
Recursion: Depth  5 ,  5000  bytes.
Recursion: Depth  4 ,  5000  bytes.
Recursion: Depth  3 ,  5000  bytes.
Recursion: Depth  2 ,  5000  bytes.
Recursion: Depth  1 ,  5000  bytes.
Recursion: Completed.
```

## Explanation

This program demonstrates stack overflow conditions in two different ways:

### Fixed Array Method
The program attempts to allocate arrays of increasing sizes directly on the stack. Stack memory is limited (typically 1MB on Windows and 8MB on Linux by default), so when the array size exceeds this limit, a stack overflow occurs.

The program uses a signal handler (on Unix-like systems) or an exception handler (on Windows) to catch the stack overflow condition and report it instead of crashing.

### Recursive Method
This approach causes stack overflow through deep function call recursion. Each function call adds a new stack frame, which consumes stack space. The program tests two variations:

1. Recursion with no additional array allocation (`rec_size = 0`)
2. Recursion with a 5000-byte array allocated at each level (`rec_size = 5000`)

Interestingly, you may notice that the recursive test with `rec_size = 0` doesn't always trigger a stack overflow, especially at moderate recursion depths. This happens because:

1. **Minimal Stack Usage**: When not allocating additional arrays in each recursive call, each stack frame is relatively small, containing only the function parameters, return address, and minimal housekeeping data.
   
2. **Compiler Behavior**: Even in Debug mode, compilers manage function calls efficiently. While not implementing full Tail Call Optimization (TCO), modern compilers may still optimize how stack frames are managed.

3. **Stack Frame Structure**: The minimal recursive function without local array allocation creates very lightweight stack frames, allowing for deeper recursion before hitting stack limits.

When using `rec_size = 5000`, stack overflow occurs much more quickly as each recursive call adds a substantial amount of data to the stack.
