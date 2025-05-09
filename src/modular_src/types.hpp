#pragma once

#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define WINDOWS 1
#define LINUX 0
#define APPLE 0
#elif (defined(__linux__))
#define WINDOWS 0
#define LINUX 1
#define APPLE 0
#elif (defined(__APPLE__) || defined(__APPLE_CPP__) || defined(__MACH__) || defined(__DARWIN))
#define WINDOWS 0
#define LINUX 0
#define APPLE 1
#else
#define WINDOWS 0
#define LINUX 0
#define APPLE 0
#endif

#ifdef _MSC_VER
#define MSVC 1
#endif

 // shorter and succinct macros
#if __cplusplus > 202100L
#define CPP 23
#ifdef __VMAWARE_DEBUG__
#pragma message("using post-C++23, set back to C++23 standard")
#endif
#elif __cplusplus == 202100L
#define CPP 23
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++23")
#endif
#elif __cplusplus == 202002L
#define CPP 20
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++20")
#endif
#elif __cplusplus == 201703L
#define CPP 17
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++17")
#endif
#elif __cplusplus == 201402L
#define CPP 14
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++14")
#endif
#elif __cplusplus == 201103L
#define CPP 11
#ifdef __VMAWARE_DEBUG__
#pragma message("using C++11")
#endif
#elif __cplusplus < 201103L
#define CPP 1
#ifdef __VMAWARE_DEBUG__
#pragma message("using pre-C++11")
#endif
#else
#define CPP 0
#ifdef __VMAWARE_DEBUG__
#pragma message("Unknown C++ standard")
#endif
#endif

#if (CPP < 11 && !WINDOWS)
#error "VMAware only supports C++11 or above, set your compiler flag to '-std=c++20' for gcc/clang, or '/std:c++20' for MSVC"
#endif

// unused for now, maybe in the future idk
#if (WINVER == 0x0501) // Windows XP, (0x0701 for Windows 7)
#define WIN_XP 1
#else 
#define WIN_XP 0
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define x86_64 1
#else
#define x86_64 0
#endif

#if defined(__i386__) || defined(_M_IX86)
#define x86_32 1
#else
#define x86_32 0
#endif

#if x86_32 || x86_64
#define x86 1
#else
#define x86 0
#endif

#if (defined(__arm__) || defined(__ARM_LINUX_COMPILER__) || defined(__aarch64__) || defined(_M_ARM64))
#define ARM 1
#else
#define ARM 0
#endif

#if defined(__clang__)
#define GCC 0
#define CLANG 1
#elif defined(__GNUC__)
#define GCC 1
#define CLANG 0
#else
#define GCC 0
#define CLANG 0
#endif

#if !(defined(WINDOWS) || defined(LINUX) || defined(APPLE))
#warning "Unknown OS detected, tests will be severely limited"
#endif

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#if (WINDOWS)
    using brand_score_t = i32;
#else
    using brand_score_t = u8;
#endif

// macro for bypassing unused parameter/variable warnings.
// this isn't really a type, but i'm adding this here
// because of its universal versatility
#define UNUSED(x) ((void)(x))

// MSVC-specific errors
#define SPECTRE 5045
#define ASSIGNMENT_OPERATOR 4626
#define NO_INLINE_FUNC 4514
#define PADDING 4820
#define FS_HANDLE 4733

#if (!WINDOWS)
#define TCHAR char
#endif

#ifdef _UNICODE
#define tregex std::wregex
#else
#define tregex std::regex
#endif

// macro shortcut to disable MSVC warnings
#if (WINDOWS)
#define MSVC_DISABLE_WARNING(...) __pragma(warning(disable : __VA_ARGS__))
#define MSVC_ENABLE_WARNING(...) __pragma(warning(default : __VA_ARGS__))
#else
#define MSVC_DISABLE_WARNING(...)
#define MSVC_ENABLE_WARNING(...)
#endif

#if (CPP >= 17)
#define VMAWARE_CONSTEXPR constexpr
#else
#define VMAWARE_CONSTEXPR
#endif