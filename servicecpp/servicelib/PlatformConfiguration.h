#pragma once

#if defined(linux) || defined(__linux) || defined(__linux__)
#	define LINUX
#elif defined(__APPLE__) || defined(__OSX__)
#	define MAC
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#	define WINDOWS
#else
#	error "unknown OS"
#endif

#if defined(LINUX) || defined(MAC) 
#define POSIX
#endif

#if defined(WINDOWS)
#ifdef _WIN64
#define PLATFORM_X64
#else
#define PLATFORM_X32
#endif

#elif defined(POSIX)
#if defined(LP64) || defined(_LP64)
#define PLATFORM_X64
#else
#define PLATFORM_X32
#endif
#endif
