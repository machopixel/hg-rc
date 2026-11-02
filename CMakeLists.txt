cmake_minimum_required(VERSION 3.21)
project(hg_rc C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The game DLLs are 32-bit, and so are their structs and file formats.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "hg-rc builds 32-bit only. Configure with: cmake -A Win32")
endif()

if(MSVC)
    # Static CRT, like the original, so the exe has no runtime DLL to ship.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    # Same floating-point results on every platform (DECISIONS.md, "C runtime replacement"): SSE2 doubles, no x87;
    # /fp:precise without /fp:contract (no fused multiply-add); never /fp:fast.
    add_compile_options(/arch:SSE2 /fp:precise)
else()
    add_compile_options(-Wall -Wextra)
    # The same as MSVC's settings above: SSE2 doubles instead of x87 (32-bit x86), no fused multiply-add, and never
    # -ffast-math.
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|X86|i[3-6]86|AMD64|amd64|x86_64)$")
        add_compile_options(-msse2 -mfpmath=sse)
    endif()
    add_compile_options(-ffp-contract=off)
endif()

# SDL3 is the platform layer (input now, audio and video later). It is built
# from source into the exe, so the game folder gets no extra DLL. The first
# configure downloads it, which needs internet once.
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.30/SDL3-3.2.30.zip
    URL_HASH SHA256=bb56270841f378478eefe20072c8a6ad54fe9508d904fb2550ea4bd206ece0f3
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(SDL3)

add_subdirectory(src/launcher)
add_subdirectory(src/plugins/hgvk)
