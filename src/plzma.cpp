//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2022 Oleh Kulykov <olehkulykov@gmail.com>
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


#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_c_bindings_private.hpp"

#include <stdint.h>
#include <limits.h>
#include <time.h>

#if defined(LIBPLZMA_MSC)
#include <malloc.h>
#endif

#if defined(BUILDING_NODE_EXTENSION)
#include <node.h>
#include <uv.h>
#endif

#ifndef RAND_MAX
#define RAND_MAX 0x7fffffff
#endif

#if defined(__clang__)
  #if __has_feature(cxx_rtti)
    #define RTTI_ENABLED 1
  #endif
#elif defined(__GNUG__)
  #if defined(__GXX_RTTI)
    #define RTTI_ENABLED 1
  #endif
#elif defined(_MSC_VER)
  #if defined(_CPPRTTI)
    #define RTTI_ENABLED 1
  #endif
#endif

int32_t plzma_random_in_range(const int32_t low, const int32_t up) {
    static bool notInitalized = true;
    if (notInitalized) {
        notInitalized = false;
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    return (rand() % (up - low + 1)) + low;
}

/// Memory

uint64_t plzma_max_size(void) {
#if defined(SIZE_T_MAX)
    return static_cast<uint64_t>(SIZE_T_MAX);
#elif defined(SIZE_MAX)
    return static_cast<uint64_t>(SIZE_MAX);
#elif defined(ULONG_MAX)
    return static_cast<uint64_t>(ULONG_MAX);
#else
#error "Max size not defined."
#endif
}

void * LIBPLZMA_NULLABLE plzma_malloc(size_t size) {
    return malloc(size);
}

void * LIBPLZMA_NULLABLE plzma_malloc_zero(size_t size) {
    void * mem = plzma_malloc(size);
    if (mem) {
        memset(mem, 0, size);
    }
    return mem;
}

void * LIBPLZMA_NULLABLE plzma_realloc(void * LIBPLZMA_NULLABLE mem, size_t new_size) {
    return realloc(mem, new_size);
}

void plzma_free(void * LIBPLZMA_NULLABLE mem) {
    if (mem) {
        free(mem);
    }
}

#if 0
void * LIBPLZMA_NULLABLE plzma_aligned_malloc(size_t size, size_t alignment) {
#if defined(LIBPLZMA_MSC)
    return _aligned_malloc(size, alignment);
#elif defined(LIBPLZMA_POSIX)
#if defined(__ANDROID__) || defined(__ANDROID_API__)
    // http://code.google.com/p/android/issues/detail?id=35391
    return malloc(size);
#else
    void * ptr = NULL;
    return posix_memalign(&ptr, alignment, size) ? malloc(size) : ptr;
#endif
#else
    return malloc(size);
#endif
}

void * LIBPLZMA_NULLABLE plzma_aligned_realloc(void * LIBPLZMA_NULLABLE mem, size_t new_size, size_t alignment) {
#if defined(LIBPLZMA_MSC)
    return _aligned_realloc(mem, new_size, alignment);
#else
    return realloc(mem, new_size);
#endif
}

void plzma_aligned_free(void * LIBPLZMA_NULLABLE mem) {
    if (mem) {
#if defined(LIBPLZMA_MSC)
        _aligned_free(mem);
#else
        free(mem);
#endif
    }
}
#endif // if 0

/// String

const char * LIBPLZMA_NONNULL plzma_empty_cstring = "";
const wchar_t * LIBPLZMA_NONNULL plzma_empty_wstring = L"";

char * LIBPLZMA_NULLABLE plzma_cstring_copy(const char * LIBPLZMA_NULLABLE str) {
    const size_t len = str ? strlen(str) : 0;
    if (len > 0) {
        char * dst = static_cast<char *>(plzma_malloc(sizeof(char) * (len + 1)));
        if (dst) {
            memcpy(dst, str, sizeof(char) * len);
            dst[len] = 0;
            return dst;
        }
    }
    return nullptr;
}

char * LIBPLZMA_NULLABLE plzma_cstring_append(char * LIBPLZMA_NULLABLE source, const char * LIBPLZMA_NULLABLE str) {
    if (source) {
        const size_t appLen = str ? strlen(str) : 0;
        if (appLen > 0) {
            const size_t srcLen = strlen(source);
            const size_t dstLen = srcLen + appLen;
            char * dst = static_cast<char *>(plzma_realloc(source, sizeof(char) * (dstLen + 1)));
            if (dst) {
                memcpy(dst + srcLen, str, appLen);
                dst[dstLen] = 0;
                return dst;
            }
        }
        return source;
    }
    return plzma_cstring_copy(str);
}

#include "C/CpuArch.h"
#include "C/7zVersion.h"
#include "C/7zCrc.h"
#include "C/Aes.h"
#include "C/XzCrc64.h"

#define LIBPLZMA_STRINGIFY(x) #x
#define LIBPLZMA_TOSTRING(x) LIBPLZMA_STRINGIFY(x)

const char * LIBPLZMA_NONNULL plzma_version(void) {
    static const char * ver = "libplzma "
    LIBPLZMA_TOSTRING(LIBPLZMA_VERSION_MAJOR) "."
    LIBPLZMA_TOSTRING(LIBPLZMA_VERSION_MINOR) "."
    LIBPLZMA_TOSTRING(LIBPLZMA_VERSION_PATCH)
#if defined(LIBPLZMA_VERSION_BUILD)
    " (" LIBPLZMA_TOSTRING(LIBPLZMA_VERSION_BUILD) ")"
#endif
    
#if defined(LIBPLZMA_STATIC)
    " : static"
#elif defined(LIBPLZMA_SHARED)
    " : shared"
#endif
    
#if defined(DEBUG)
    " : debug"
#endif
    
#if defined(COCOAPODS)
    " : CocoaPods"
#endif
    
#if defined(SWIFT_PACKAGE)
    " : Swift Package"
#endif
    
#if defined(LIBPLZMA_HAVE_STD)
    " : std"
#endif

#if defined(RTTI_ENABLED)
    " : rtti"
#elif defined(LIBPLZMA_NO_CPP_RTTI)
    " : no rtti"
#endif
    
#if defined(__TIMESTAMP__)
    " : " __TIMESTAMP__
#else
#if defined(__DATE__)
    " : " __DATE__
#endif
#if defined(__TIME__)
    " : " __TIME__
#endif
#endif

#if defined(_MSC_FULL_VER)
    " : " LIBPLZMA_TOSTRING(_MSC_FULL_VER)
#elif defined(_MSC_VER)
    " : " LIBPLZMA_TOSTRING(_MSC_VER)
#endif
    
#if defined(__VERSION__)
#if defined(__GNUC__) && !defined(__clang__)
    " : gcc " __VERSION__
#else
    " : " __VERSION__
#endif
#endif
    
#if defined(LIBPLZMA_OS_WINDOWS) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Windows"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(TARGET_OS_OSX) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_OSX
    " : macOS"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#endif
#if defined(TARGET_OS_IPHONE) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_IPHONE
    " : iPhone"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#endif
#if defined(TARGET_OS_IOS) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_IOS
    " : iOS"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#endif
#if defined(TARGET_OS_WATCH) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_WATCH
    " : watchOS"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#endif
#if defined(TARGET_OS_TV) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_TV
    " : tvOS"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#endif
#if defined(TARGET_OS_SIMULATOR) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
#if TARGET_OS_SIMULATOR
    " : Simulator"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#endif
#if defined(__APPLE__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Apple"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if (defined(__ANDROID__) || defined(__ANDROID_API__)) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Android"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#if !defined(LIBPLZMA_PLATFORM_MOBILE)
#define LIBPLZMA_PLATFORM_MOBILE 1
#endif
#endif
#if defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__)
#if !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : BSD"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#endif
#if defined(__gnu_linux__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : GNU/Linux"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(__linux__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Linux"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(__ros__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Akaros"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(__native_client__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : NaCL"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(__asmjs__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : AsmJS"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if defined(__Fuchsia__) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : Fuschia"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if (defined(__unix__) || defined(__unix)) && !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : unix"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
#if !defined(LIBPLZMA_VERSION_OS_DETECTED)
    " : unknown"
#define LIBPLZMA_VERSION_OS_DETECTED 1
#endif
    
#if defined(BUILDING_NODE_EXTENSION)
    " : Node.js " NODE_VERSION_STRING
#if defined(V8_MAJOR_VERSION) && defined(V8_MINOR_VERSION) && defined(V8_PATCH_LEVEL)
    ", V8 " LIBPLZMA_TOSTRING(V8_MAJOR_VERSION) "." LIBPLZMA_TOSTRING(V8_MINOR_VERSION) "." LIBPLZMA_TOSTRING(V8_PATCH_LEVEL)
#if defined(V8_BUILD_NUMBER)
    "." LIBPLZMA_TOSTRING(V8_BUILD_NUMBER)
#endif
#endif
#if defined(UV_VERSION_MAJOR) && defined(UV_VERSION_MINOR) && defined(UV_VERSION_PATCH)
    ", uv " LIBPLZMA_TOSTRING(UV_VERSION_MAJOR) "." LIBPLZMA_TOSTRING(UV_VERSION_MINOR) "." LIBPLZMA_TOSTRING(UV_VERSION_PATCH)
#endif
#if defined(NODE_MODULE_VERSION)
    ", module " LIBPLZMA_TOSTRING(NODE_MODULE_VERSION)
#endif
#if defined(NAPI_VERSION)
    ", napi " LIBPLZMA_TOSTRING(NAPI_VERSION)
#endif
#endif
    
#if defined(LIBPLZMA_NO_C_BINDINGS)
    " : no C bindings"
#endif
    
#if defined(LIBPLZMA_NO_CRYPTO)
    " : no crypto"
#endif
    
#if defined(LIBPLZMA_NO_TAR)
    " : no tar"
#endif
    
#if defined(LIBPLZMA_NO_PROGRESS)
    " : no progress"
#endif
    
#if defined(LIBPLZMA_THREAD_UNSAFE)
    " : thread unsafe"
#endif
    
    "\nLZMA SDK " MY_VERSION_COPYRIGHT_DATE "\n";
    return ver;
}

#include "plzma_in_streams.hpp"
#include "plzma_out_streams.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/Windows/PropVariant.h"

namespace plzma {
/**
@brief 12 = 4  Kb min
@brief 13 = 8  Kb
@brief 14 = 16 Kb
@brief 15 = 32 Kb
@brief 16 = 64 Kb
@brief 17 = 128 Kb
@brief 18 = 256 Kb
@brief 19 = 512 Kb
@brief 20 = 1 Mb
@brief 21 = 2 Mb
@brief 22 = 4 Mb
@brief 31 = 1 Gb
*/

#if defined(LIBPLZMA_PLATFORM_MOBILE)
plzma_size_t kStreamReadSize = static_cast<unsigned int>(1) << 16;
plzma_size_t kStreamWriteSize = static_cast<unsigned int>(1) << 16;
plzma_size_t kDecoderReadSize = static_cast<unsigned int>(1) << 16;
plzma_size_t kDecoderWriteSize = static_cast<unsigned int>(1) << 18;
#else
plzma_size_t kStreamReadSize = static_cast<unsigned int>(1) << 20;
plzma_size_t kStreamWriteSize = static_cast<unsigned int>(1) << 20;
plzma_size_t kDecoderReadSize = static_cast<unsigned int>(1) << 20;
plzma_size_t kDecoderWriteSize = static_cast<unsigned int>(1) << 22;
#endif // LIBPLZMA_PLATFORM_MOBILE

    void initialize(void) {
        static bool notInitalized = true;
        if (notInitalized) {
#if defined(LIBPLZMA_USING_REGISTRATORS)
            uint64_t r = plzma_registrator_1();
            r |= plzma_registrator_2();
            r |= plzma_registrator_3();
            r |= plzma_registrator_4();
            r |= plzma_registrator_5();
            r |= plzma_registrator_6();
            r |= plzma_registrator_7();
            r |= plzma_registrator_8();
            r |= plzma_registrator_9();
            r |= plzma_registrator_10();
            r |= plzma_registrator_11();
            r |= plzma_registrator_12();
            r |= plzma_registrator_13();
            r |= plzma_registrator_14();
            r |= plzma_registrator_15();
            r |= plzma_registrator_16();
#if !defined(LIBPLZMA_NO_TAR)
            r |= plzma_registrator_17();
#endif
            if (r > 0) {
                notInitalized = false;
            } else {
                assert(0);
            }
#else
            notInitalized = false;
#endif
            static_assert((static_cast<UInt32>(STREAM_SEEK_SET) == SEEK_SET) &&
                          (static_cast<UInt32>(STREAM_SEEK_SET) == static_cast<UInt32>(SZ_SEEK_SET)), "Seeks mismatch");
            static_assert((static_cast<UInt32>(STREAM_SEEK_CUR) == SEEK_CUR) &&
                          (static_cast<UInt32>(STREAM_SEEK_CUR) == static_cast<UInt32>(SZ_SEEK_CUR)), "Seeks mismatch");
            static_assert((static_cast<UInt32>(STREAM_SEEK_END) == SEEK_END) &&
                          (static_cast<UInt32>(STREAM_SEEK_END) == static_cast<UInt32>(SZ_SEEK_END)), "Seeks mismatch");
            static_assert(sizeof(NWindows::NCOM::CPropVariant) == sizeof(PROPVARIANT), "Props sizes mismatch");
            
            CrcGenerateTable();
            AesGenTables();
            Crc64GenerateTable();
        }
    }
    
} // namespace plzma

/// Settings

plzma_size_t plzma_stream_read_size(void) {
    return plzma::kStreamReadSize;
}

void plzma_set_stream_read_size(const plzma_size_t size) {
    plzma::kStreamReadSize = size;
}

plzma_size_t plzma_stream_write_size(void) {
    return plzma::kStreamWriteSize;
}

void plzma_set_stream_write_size(const plzma_size_t size) {
    plzma::kStreamWriteSize = size;
}

plzma_size_t plzma_decoder_read_size(void) {
    return plzma::kDecoderReadSize;
}

void plzma_set_decoder_read_size(const plzma_size_t size) {
    plzma::kDecoderReadSize = size;
}

plzma_size_t plzma_decoder_write_size(void) {
    return plzma::kDecoderWriteSize;
}

void plzma_set_decoder_write_size(const plzma_size_t size) {
    plzma::kDecoderWriteSize = size;
}

#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

/// Item Array

plzma_item_array plzma_item_array_create(const plzma_size_t capacity) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_item_array)
    auto array = makeShared<ItemArray>(capacity);
    createdCObject.object = static_cast<void *>(array.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_item_array_add(plzma_item_array * LIBPLZMA_NONNULL array,
                          plzma_item * LIBPLZMA_NONNULL item) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(array)
    SharedPtr<Item> itemSPtr(static_cast<Item *>(item->object));
    static_cast<ItemArray *>(array->object)->push(static_cast<SharedPtr<Item> &&>(itemSPtr));
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(array)
}

plzma_size_t plzma_item_array_count(const plzma_item_array * LIBPLZMA_NONNULL array) {
    return array->exception ? 0 : static_cast<const ItemArray *>(array->object)->count();
}

plzma_item plzma_item_array_at(plzma_item_array * LIBPLZMA_NONNULL array,
                               const plzma_size_t index) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_item, array)
    auto item = static_cast<const ItemArray *>(array->object)->at(index);
    createdCObject.object = static_cast<void *>(item.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_item_array_sort(plzma_item_array * LIBPLZMA_NONNULL array) {
    if (!array->exception) { static_cast<ItemArray *>(array->object)->sort(); }
}

void plzma_item_array_release(plzma_item_array * LIBPLZMA_NONNULL array) {
    plzma_object_exception_release(array);
    SharedPtr<ItemArray> arraySPtr;
    arraySPtr.assign(static_cast<ItemArray *>(array->object));
    array->object = nullptr;
}

/// Item Out Stream Array

plzma_item_out_stream_array plzma_item_out_stream_array_create(const plzma_size_t capacity) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_item_out_stream_array)
    auto map = makeShared<ItemOutStreamArray>(capacity);
    createdCObject.object = static_cast<void *>(map.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_item_out_stream_array_add(plzma_item_out_stream_array * LIBPLZMA_NONNULL map,
                                     plzma_item * LIBPLZMA_NONNULL item,
                                     plzma_out_stream * LIBPLZMA_NONNULL stream) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(map)
    SharedPtr<Item> itemSPtr(static_cast<Item *>(item->object));
    SharedPtr<OutStream> streamSPtr(static_cast<OutStream *>(stream->object));
    static_cast<ItemOutStreamArray *>(map->object)->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(static_cast<SharedPtr<Item> &&>(itemSPtr),
                                                                                                      static_cast<SharedPtr<OutStream> &&>(streamSPtr)));
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(map)
}

plzma_size_t plzma_item_out_stream_array_count(const plzma_item_out_stream_array * LIBPLZMA_NONNULL map) {
    return map->exception ? 0 : static_cast<const ItemOutStreamArray *>(map->object)->count();
}

plzma_item_out_stream_array_pair plzma_item_out_stream_array_pair_at(plzma_item_out_stream_array * LIBPLZMA_NONNULL map,
                                                                     const plzma_size_t index) {
    plzma_item_out_stream_array_pair createdCObject;
    createdCObject.item.object = createdCObject.item.exception = nullptr;
    createdCObject.stream.object = createdCObject.stream.exception = nullptr;
    createdCObject.exception = nullptr;
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(map, createdCObject)
    auto pair = static_cast<ItemOutStreamArray *>(map->object)->at(index);
    createdCObject.item.object = static_cast<void *>(pair.first.take());
    createdCObject.stream.object = static_cast<void *>(pair.second.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_item_out_stream_array_sort(plzma_item_out_stream_array * LIBPLZMA_NONNULL map) {
    if (!map->exception) { static_cast<ItemOutStreamArray *>(map->object)->sort(); }
}

void plzma_item_out_stream_array_pair_release(plzma_item_out_stream_array_pair * LIBPLZMA_NONNULL pair) {
    plzma_exception_release(pair->exception);
    pair->exception = nullptr;
    plzma_item_release(&pair->item);
    plzma_out_stream_release(&pair->stream);
}

void plzma_item_out_stream_array_release(plzma_item_out_stream_array * LIBPLZMA_NONNULL map) {
    plzma_object_exception_release(map);
    SharedPtr<ItemOutStreamArray> mapSPtr;
    mapSPtr.assign(static_cast<ItemOutStreamArray *>(map->object));
    map->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS

#if 0
void plzma_print_memory(int line, const void * LIBPLZMA_NULLABLE mem, const size_t len) {
    fprintf(stdout, "PRINT MEMORY AT LINE: %i, LEN: %llu\n", line, (unsigned long long)len);
    if (!mem) {
        fprintf(stdout, "NULL\n");
        fflush(stdout);
        return;
    }
    char buff[256];
    char * s = buff;
    const uint8_t * umem = reinterpret_cast<const uint8_t *>(mem);
    bool hasoutput = false;
    for (size_t i = 0, j = 0; i < len; i++, j++) {
        int sp = sprintf(s, "0x%02x ", umem[i]);
        s += sp;
        hasoutput = true;
        if (j == 10) {
            fprintf(stdout, "%s\n", buff);
            j = 0;
            s = buff;
            hasoutput = false;
        }
    }
    if (hasoutput) {
        fprintf(stdout, "%s\n", buff);
    }
    fflush(stdout);
}
#endif // #if 0
