cmake_minimum_required(VERSION 3.10)
project(stack_overflow_detection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Stack size option (in bytes, default 512KB)
set(STACK_SIZE 524288 CACHE STRING "Stack size limit in bytes")

option(USE_MINGW "Enable MinGW-specific settings" OFF)
option(USE_MSVC "Enable MSVC-specific settings" OFF)

if(USE_MINGW AND USE_MSVC)
    message(FATAL_ERROR "Cannot enable both USE_MINGW and USE_MSVC simultaneously.")
endif()

add_executable(stack_overflow main.cpp)

# Pass STACK_SIZE to C++ code
target_compile_definitions(stack_overflow PRIVATE STACK_SIZE=${STACK_SIZE})

# Debug build settings
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0" CACHE STRING "Debug flags")

if(WIN32)
    if(USE_MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Configuring for MinGW")
        target_compile_options(stack_overflow PRIVATE -Wall -fno-optimize-sibling-calls)
        target_link_options(stack_overflow PRIVATE -Wl,--stack=${STACK_SIZE})
    elseif(USE_MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        message(STATUS "Configuring for MSVC")
        target_compile_options(stack_overflow PRIVATE /EHsc /W4 /Od)
        target_link_options(stack_overflow PRIVATE /STACK:${STACK_SIZE})
    else()
        message(WARNING "Unknown Windows compiler. Applying default settings.")
        target_compile_options(stack_overflow PRIVATE /EHsc /Od)
        target_link_options(stack_overflow PRIVATE /STACK:${STACK_SIZE})
    endif()
else()
    message(STATUS "Configuring for Linux/MacOS")
    target_compile_options(stack_overflow PRIVATE -Wall -fno-optimize-sibling-calls)
    target_compile_definitions(stack_overflow PRIVATE _POSIX_C_SOURCE=200809L)
endif()

# Custom target to run with ulimit -s (Linux/Mac only)
if(NOT WIN32)
    math(EXPR STACK_SIZE_KB "${STACK_SIZE} / 1024")
    add_custom_target(run_with_ulimit
        COMMAND bash -c "ulimit -s ${STACK_SIZE_KB} && ./stack_overflow"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS stack_overflow
        COMMENT "Running stack_overflow with ulimit -s ${STACK_SIZE_KB} KB"
    )
endif()