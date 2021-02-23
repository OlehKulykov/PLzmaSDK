//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2021 Oleh Kulykov <olehkulykov@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#ifndef __PLZMA_PRIVATE_H__
#define __PLZMA_PRIVATE_H__ 1

#include "../libplzma.h"

#include <string.h>
#include <assert.h>

// Mark of the libplzma build.
// This definition uses by the patched, original [LZMA SDK] via 'StdAfx.h' files in Win env.
#ifndef LIBPLZMA
#define LIBPLZMA 1
#endif

#if (defined(DEBUG) || defined(_DEBUG)) && !defined(DEBUG)
#define DEBUG 1
#endif

// plzma_c_bindings_private.hpp
#if !defined(LIBPLZMA_HAVE_STD) && defined(SWIFT_PACKAGE)
#define LIBPLZMA_HAVE_STD 1
#endif

#if defined(LIBPLZMA_OS_WINDOWS)

#ifndef _WIN32
#define _WIN32
#endif

#ifndef WIN32
#define WIN32
#endif

#if !defined(LIBPLZMA_MSC) && (defined(_MSC_VER) || defined(_MSC_FULL_VER))
#define LIBPLZMA_MSC 1
#endif

#else // !LIBPLZMA_OS_WINDOWS

#if __has_include(<unistd.h>) || defined(HAVE_UNISTD_H)
#define LIBPLZMA_POSIX 1
#endif

#if !defined(LIBPLZMA_POSIX)
#if defined(_POSIX_VERSION) || defined(_POSIX2_VERSION) || defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define LIBPLZMA_POSIX 1
#endif
#if !defined(LIBPLZMA_POSIX) && (defined(__APPLE__) || defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__gnu_linux__))
#define LIBPLZMA_POSIX 1
#endif
#if !defined(LIBPLZMA_POSIX) && (defined(__ANDROID__) || defined(__ANDROID_API__))
#define LIBPLZMA_POSIX 1
#endif
#endif

#if defined(LIBPLZMA_POSIX)
#include <unistd.h>
#include <pthread.h>
#endif

// Types
typedef void* PVOID;
typedef void* LPVOID;
typedef void* HANDLE;

#if !defined(DWORD_SIZE)
#define DWORD_SIZE 4
typedef uint32_t DWORD;
#endif

#if !defined(BOOL)
#define BOOL int
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#endif // !LIBPLZMA_OS_WINDOWS

#ifndef USE_MIXER_ST
#define USE_MIXER_ST
#endif

#ifndef _7ZIP_ST
#define _7ZIP_ST 1
#endif

#if defined(DEBUG)
#define LIBPLZMA_DEBUG_ASSERT(ASSERT_CONDITION) assert(ASSERT_CONDITION);
#else
#define LIBPLZMA_DEBUG_ASSERT(ASSERT_CONDITION)
#endif


#if !defined(LIBPLZMA_USING_REGISTRATORS)
#define LIBPLZMA_USING_REGISTRATORS 1
LIBPLZMA_C_API(uint64_t) plzma_registrator_1(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_2(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_3(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_4(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_5(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_6(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_7(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_8(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_9(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_10(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_11(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_12(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_13(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_14(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_15(void);
LIBPLZMA_C_API(uint64_t) plzma_registrator_16(void);
#if !defined(LIBPLZMA_NO_TAR)
LIBPLZMA_C_API(uint64_t) plzma_registrator_17(void);
#endif
#endif // !LIBPLZMA_USING_REGISTRATORS

#if 0
LIBPLZMA_C_API(void) plzma_print_memory(int line, const void * LIBPLZMA_NULLABLE mem, const size_t len);
#endif // #if 0

#endif // !__PLZMA_PRIVATE_H__
