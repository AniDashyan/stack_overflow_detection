#include <stdexcept>
#include <string>
#include "kaizen.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/resource.h>
#include <iostream>
#endif

void print_stack_limit() {
#ifdef _WIN32
    zen::print("Windows: Stack ~1MB (linker-configurable).\n");
#else
    struct rlimit rl;
    zen::print(getrlimit(RLIMIT_STACK, &rl) == 0
        ? "Stack limit: ", rl.rlim_cur, " bytes\n"
        : "Failed to get stack limit\n");
#endif
}

#ifndef _WIN32
void sigsegv_handler(int) {
    throw std::runtime_error("SIGSEGV (stack overflow)");
}
#endif

#ifdef _WIN32
LONG WINAPI exception_handler(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        throw std::runtime_error("Stack overflow");
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void test_fixed(size_t size) {
    char array[size];
    array[0] = 0;
    zen::print("Fixed: Allocated ", size, " bytes.\n");
}

void test_recursion(size_t depth, size_t array_size) {
    if (depth == 0) return;
    if (array_size > 0) {
        char array[array_size];
        array[0] = 0;
    }
    zen::print("Recursion: Depth ", depth, ", ", array_size, " bytes.\n");
    test_recursion(depth - 1, array_size);
}

int parse_args(int argc, char** argv) {
    zen::cmd_args args(argv, argc);
    if (args.is_present("--fixed"))
        return 1;
    if (args.is_present("--recursive"))
        return 0;
    zen::print(zen::color::yellow("Warning: No options provided, using default: --fixed\n"));
    return 1;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(exception_handler);
#else
    signal(SIGSEGV, sigsegv_handler);
#endif

    print_stack_limit();
    size_t sizes[] = {500000, 1000000, 2000000, 4000000, 8000000}; 
    size_t depths[] = {1000, 10000};
    size_t rec_sizes[] = {0, 5000}; 

    int opt = parse_args(argc, argv);
    zen::print(opt ? "Running --fixed test:\n" : "Running --recursive test:\n");
    std::cout << std::flush;
    if (opt) {
        for (size_t size : sizes) {
            try {
                zen::print("Trying ", size, " bytes... ");
                std::cout << std::flush;
                test_fixed(size);
            } catch (const std::exception& e) {
                zen::print("Error: ", e.what(), "\n");
            }
        }
    } else {
        for (size_t depth : depths) {
            for (size_t rec_size : rec_sizes) {
                try {
                    zen::print("Trying depth ", depth, ", ", rec_size, " bytes... ");
                    std::cout << std::flush;
                    test_recursion(depth, rec_size);
                    zen::print("Recursion: Completed.\n");
                } catch (const std::exception& e) {
                    zen::print("Error: ", e.what(), "\n");
                }
            }
        }
    }

    zen::print("\nTest done.\n");
    return 0;
}