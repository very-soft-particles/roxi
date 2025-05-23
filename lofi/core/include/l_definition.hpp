// =====================================================================================
//
//       Filename:  l_definition.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-16 2:04:40 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

// OS and compiler defines

#if defined (__clang__)
# define COMPILER_CLANG 1
# if defined (_WIN32)
#  define OS_WINDOWS 1
# elif defined (__gnu_linux__)
#  define OS_LINUX 1
# elif defined (__APPLE__) && defined (__MACH__)
#  define OS_MAC 1
# elif defined (__ANDROID__)
#  define OS_ANDROID 1
# else
#  error missing OS detection
# endif

# if defined (__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif 

#elif defined(_MSC_VER)
# define COMPILER_CL 1
# if defined(_WIN32)
#  define OS_WINDOWS 1
# else 
#  error no OS detection
# endif 

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined(_M_ARM)
#  define ARCH_ARM 1
# else
#  error missing ARCH detection
# endif 

#elif defined(__GNUC__)
# define COMPILER_GCC 1
# if defined (_WIN32)
#  define OS_WINDOWS 1
# elif defined (__gnu_linux__)
#  define OS_LINUX 1
# elif defined (__APPLE__) && defined (__MACH__)
#  define OS_MAC 1
# elif defined (__ANDROID__)
#  define OS_ANDROID 1
# else
#  error missing OS detection
# endif

# if defined (__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif 
#else
# error no context cracking for this compiler
#endif

#if !defined (COMPILER_CL)
# define COMPILER_CL 0
#endif
#if !defined (COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined (COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined (OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined (OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined (OS_MAC)
# define OS_MAC 0
#endif
#if !defined (OS_ANDROID)
# define OS_ANDROID 0
#endif
#if !defined (ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined (ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined (ARCH_ARM)
# define ARCH_ARM 0
#endif
#if !defined (ARCH_ARM64)
# define ARCH_ARM64 0
#endif



