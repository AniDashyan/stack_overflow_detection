#include <stdexcept>
#include <string>
#include <iostream>
#include "kaizen.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/resource.h>
#endif

void print_stack_limit() {
#ifdef _WIN32
    zen::print("Windows: Stack ~512KB (linker-configurable).\n");
#else
    struct rlimit rl;
    zen::print(getrlimit(RLIMIT_STACK, &rl) == 0
        ? "Stack limit: ", rl.rlim_cur, " bytes\n"
        : "Failed to get stack limit\n");
#endif
}

#ifndef _WIN32
void sigsegv_handler(int sig) {
    zen::print("SIGSEGV handler invoked: signal ", sig, "\n");
    throw std::runtime_error("SIGSEGV (possible stack overflow)");
}
#endif

#ifdef _WIN32
LONG WINAPI exception_handler(EXCEPTION_POINTERS* info) {
    zen::print("Exception handler invoked: Code ", info->ExceptionRecord->ExceptionCode, "\n");
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        throw std::runtime_error("Stack overflow");
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void test_fixed(size_t size) {
    char array[8000000];
    if (size > sizeof(array)) throw std::runtime_error("Requested size too large");
    array[0] = 0;
    zen::print("Fixed: Allocated ", size, " bytes.\n");
}

void test_recursion(size_t depth, size_t array_size, size_t& total_usage) {
    if (depth == 0) return;
    if (array_size > 0) {
        char array[10000]; 
        if (array_size > sizeof(array)) throw std::runtime_error("Requested array_size too large");
        array[0] = 0;
        total_usage += array_size;
    }
    total_usage += 32; // Approximate frame overhead
#ifndef _WIN32
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0 && total_usage > rl.rlim_cur) {
        throw std::runtime_error("Stack limit exceeded");
    }
#endif
    zen::print("Recursion: Depth ", depth, ", ", array_size, " bytes, Total: ", total_usage, " bytes.\n");
    test_recursion(depth - 1, array_size, total_usage);
    zen::print("Post-recursion at depth ", depth, "\n"); // Disable TCO
}

int parse_args(int argc, char** argv) {
    zen::cmd_args args(argv, argc);
    if (args.is_present("--fixed")) return 1;
    if (args.is_present("--recursive")) return 0;
    zen::print(zen::color::yellow("Warning: No options provided, using default: --fixed\n"));
    return 1;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(exception_handler);
#else
    // Set alternate signal stack
    stack_t ss;
    ss.ss_sp = malloc(SIGSTKSZ);
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    if (sigaltstack(&ss, nullptr) != 0) {
        zen::print("Failed to set alternate stack\n");
    }
    signal(SIGSEGV, sigsegv_handler);
    // Reduce stack size (optional, prefer ulimit -s 512)
    struct rlimit rl = {524288, 524288}; // 512KB
    if (setrlimit(RLIMIT_STACK, &rl) != 0) {
        zen::print("Failed to set stack limit\n");
    }
#endif

    print_stack_limit();
    size_t sizes[] = {500000, 1000000, 2000000, 4000000, 8000000};
    size_t depths[] = {10000, 50000, 100000};
    size_t rec_sizes[] = {0, 5000, 10000};

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
                    size_t total_usage = 0;
                    test_recursion(depth, rec_size, total_usage);
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