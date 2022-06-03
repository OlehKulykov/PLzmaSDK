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


#ifndef __PLZMA_PATH_UTILS_HPP__
#define __PLZMA_PATH_UTILS_HPP__ 1

#include <cstddef>

#include "plzma_private.hpp"
#include "plzma_common.hpp"

#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(LIBPLZMA_MSC)

#include <io.h>
#include <direct.h>
#include <windows.h>

#if !defined(S_ISDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#elif defined(LIBPLZMA_POSIX)
#include <dirent.h>
#include <unistd.h>
#else
#error "Error once: file contains unimplemented functionality."
#endif

#if defined(LIBPLZMA_OS_WINDOWS)
#define CLZMA_SEP_WSTR      L"\\"
#define CLZMA_SEP_CSTR      "\\"
#elif defined(LIBPLZMA_POSIX)
#define CLZMA_SEP_WSTR      L"/"
#define CLZMA_SEP_CSTR      "/"
#endif

namespace plzma {
namespace pathUtils {
    
    template<typename T>
    bool pathExists(const T * LIBPLZMA_NONNULL path, bool * LIBPLZMA_NULLABLE isDir) noexcept;
    
    template<>
    inline bool pathExists(const wchar_t * LIBPLZMA_NONNULL path, bool * LIBPLZMA_NULLABLE isDir) noexcept {
#if defined(LIBPLZMA_MSC)
        if (_waccess(path, 0) == 0) {
            if (isDir) {
                struct _stat statbuf;
                if ((_wstat(path, &statbuf) == 0) && (S_ISDIR(statbuf.st_mode))) {
                    *isDir = true;
                } else {
                    *isDir = false;
                }
            }
            return true;
        }
#else
        assert(0);
#endif
        if (isDir) {
            *isDir = false;
        }
        return false;
    }
    
    template<>
    inline bool pathExists(const char * LIBPLZMA_NONNULL path, bool * LIBPLZMA_NULLABLE isDir) noexcept {
#if defined(LIBPLZMA_MSC)
        if (_access(path, 0) == 0) {
            if (isDir) {
                struct _stat statbuf;
                if ((_stat(path, &statbuf) == 0) && (S_ISDIR(statbuf.st_mode))) {
                    *isDir = true;
                } else {
                    *isDir = false;
                }
            }
            return true;
        }
#elif defined(LIBPLZMA_POSIX)
        if (access(path, F_OK) == 0) {
            if (isDir) {
                struct stat statbuf;
                if ((stat(path, &statbuf) == 0) && (S_ISDIR(statbuf.st_mode))) {
                    *isDir = true;
                } else {
                    *isDir = false;
                }
            }
            return true;
        }
#endif
        if (isDir) {
            *isDir = false;
        }
        return false;
    }
    
    template<typename T>
    plzma_path_stat pathStat(const T * LIBPLZMA_NULLABLE path) noexcept;
    
    template<>
    inline plzma_path_stat pathStat(const wchar_t * LIBPLZMA_NULLABLE path) noexcept {
        plzma_path_stat t{0, 0, 0, 0};
#if defined(LIBPLZMA_MSC)
        struct __stat64 statbuf;
        if (path && _wstat64(path, &statbuf) == 0) {
            t.creation = statbuf.st_ctime;
            t.last_access = statbuf.st_atime;
            t.last_modification = statbuf.st_mtime;
            t.size = static_cast<uint64_t>(statbuf.st_size);
        }
#else
        assert(0);
#endif
        return t;
    }
    
    template<>
    inline plzma_path_stat pathStat(const char * LIBPLZMA_NULLABLE path) noexcept {
        plzma_path_stat t{0, 0, 0, 0};
#if defined(LIBPLZMA_MSC)
        struct _stat statbuf;
        if (path && _stat(path, &statbuf) == 0) {
            t.creation = statbuf.st_ctime;
            t.last_access = statbuf.st_atime;
            t.last_modification = statbuf.st_mtime;
        }
#elif defined(LIBPLZMA_POSIX)
        struct stat statbuf;
        if (path && stat(path, &statbuf) == 0) {
            t.creation = statbuf.st_ctime;
            t.last_access = statbuf.st_atime;
            t.last_modification = statbuf.st_mtime;
            t.size = static_cast<uint64_t>(statbuf.st_size);
        }
#endif
        return t;
    }
    
    template<typename T>
    bool pathReadable(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool pathReadable(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _waccess(path, 4) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool pathReadable(const char * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _access(path, 4) == 0;
#elif defined(LIBPLZMA_POSIX)
        return access(path, R_OK) == 0;
#endif
    }
    
    template<typename T>
    bool pathWritable(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool pathWritable(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _waccess(path, 2) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool pathWritable(const char * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _access(path, 2) == 0;
#elif defined(LIBPLZMA_POSIX)
        return access(path, W_OK) == 0;
#endif
    }
    
    template<typename T>
    bool pathReadableAndWritable(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool pathReadableAndWritable(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _waccess(path, 6) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool pathReadableAndWritable(const char * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _access(path, 6) == 0;
#elif defined(LIBPLZMA_POSIX)
        return access(path, R_OK | W_OK) == 0;
#endif
    }
    
    template<typename T>
    bool createSingleDir(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool createSingleDir(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _wmkdir(path) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool createSingleDir(const char * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _mkdir(path) == 0;
#elif defined(LIBPLZMA_POSIX)
        const mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR |
        S_IRGRP |           S_IXGRP |
        S_IROTH |           S_IXOTH ;
        //#define    S_IRWXU        0000700        /* [XSI] RWX mask for owner */
        //#define    S_IRUSR        0000400        /* [XSI] R for owner */
        //#define    S_IWUSR        0000200        /* [XSI] W for owner */
        //#define    S_IXUSR        0000100        /* [XSI] X for owner */
        //        /* Read, write, execute/search by group */
        //#define    S_IRWXG        0000070        /* [XSI] RWX mask for group */
        //#define    S_IRGRP        0000040        /* [XSI] R for group */
        //#define    S_IWGRP        0000020        /* [XSI] W for group */
        //#define    S_IXGRP        0000010        /* [XSI] X for group */
        //        /* Read, write, execute/search by others */
        //#define    S_IRWXO        0000007        /* [XSI] RWX mask for other */
        //#define    S_IROTH        0000004        /* [XSI] R for other */
        //#define    S_IWOTH        0000002        /* [XSI] W for other */
        //#define    S_IXOTH        0000001        /* [XSI] X for other */
        return mkdir(path, mode) == 0;
#endif
    }
    
    template<typename T>
    constexpr T platformSeparator() noexcept {
#if defined(LIBPLZMA_OS_WINDOWS)
        return (sizeof(T) > sizeof(char)) ? L'\\' : '\\';
#elif defined(LIBPLZMA_POSIX)
        return (sizeof(T) > sizeof(char)) ? L'/' : '/';
#endif
    }
    
    template<typename T>
    constexpr T additionalSeparator() noexcept {
#if defined(LIBPLZMA_OS_WINDOWS)
        return (sizeof(T) > sizeof(char)) ? L'/' : '/';
#elif defined(LIBPLZMA_POSIX)
        return (sizeof(T) > sizeof(char)) ? L'\\' : '\\';
#endif
    }
    
    template<typename T>
    constexpr T schemeSeparator() noexcept {
        return (sizeof(T) > sizeof(char)) ? L':' : ':';
    }
    
    template<typename T, const T PS = platformSeparator<T>(), const T AS = additionalSeparator<T>()>
    inline bool createIntermediateDirs(const T * LIBPLZMA_NONNULL path, const size_t len) {
        bool isDir = false;
        if (pathExists<T>(path, &isDir) && isDir) {
            return true;
        }
        RawHeapMemory pathCopy(sizeof(T) * (len + 1), plzma_erase_zero);
        const T * cs1 = path;
        T * s1 = static_cast<T *>(pathCopy);
        size_t l1 = 0;
        while (*cs1) {
            if (PS == *cs1 || AS == *cs1) {
                while (*cs1 && (PS == *cs1 || AS == *cs1)) {
                    *s1++ = PS;
                    cs1++;
                }
                if (l1 > 0) {
                    bool isDir = false;
                    if (pathExists<T>(pathCopy, &isDir)) {
                        if (!isDir) {
                            pathCopy.erase(plzma_erase_zero, sizeof(T) * len);
                            return false;
                        }
                    } else if (!createSingleDir<T>(pathCopy)) {
                        pathCopy.erase(plzma_erase_zero, sizeof(T) * len);
                        return false;
                    }
                    l1 = 0;
                }
            } else {
                l1++;
                *s1++ = *cs1++;
            }
        }
        const bool res = (l1 > 0 && !createSingleDir<T>(pathCopy)) ? false : true;
        pathCopy.erase(plzma_erase_zero, sizeof(T) * len);
        return res;
    }
    
    template<typename T>
    bool removeEmptyDir(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool removeEmptyDir(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _wrmdir(path) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool removeEmptyDir(const char * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _rmdir(path) == 0;
#else
        return rmdir(path) == 0;
#endif
    }
    
    template<typename T>
    bool removeFile(const T * LIBPLZMA_NONNULL path) noexcept;
    
    template<>
    inline bool removeFile(const wchar_t * LIBPLZMA_NONNULL path) noexcept {
#if defined(LIBPLZMA_MSC)
        return _wremove(path) == 0;
#else
        assert(0);
        return false;
#endif
    }
    
    template<>
    inline bool removeFile(const char * LIBPLZMA_NONNULL path) noexcept {
        return remove(path) == 0;
    }
    
    template<typename T>
    inline bool initializeTmpPath(const T * LIBPLZMA_NONNULL parent, const T * LIBPLZMA_NONNULL dir, Path & path) {
        if (pathReadableAndWritable<T>(parent)) {
            path.set(parent);
            path.append(dir);
            return path.exists() ? path.readableAndWritable() : (path.createDir(false) && path.readableAndWritable());
        }
        return false;
    }
    
    template<typename T, const T PS = platformSeparator<T>(), const T AS = additionalSeparator<T>()>
    inline bool requireSeparator(const T * LIBPLZMA_NULLABLE path,
                                 const size_t size,
                                 const T * LIBPLZMA_NONNULL component) noexcept {
        if (size > 0) {
            const T * parent = path + (size - 1);
            return !(PS == *parent || PS == *component || AS == *parent || AS == *component);
        }
        return false;
    }
    
    template<typename T, const T PS = platformSeparator<T>(), const T AS = additionalSeparator<T>()>
    inline bool isScheme(const T * LIBPLZMA_NONNULL str, const T * LIBPLZMA_NONNULL pos) noexcept {
        if (PS == *pos || AS == *pos) {
            pos--; // x://
        }
        if (pos > str && (PS == *pos || AS == *pos)) {
            pos--; // x:/
        }
        if (pos > str && schemeSeparator<T>() == *pos) {
            pos--; // x:
        } else {
            return false;
        }
        return (pos >= str && isalpha(*pos)); // x
    }
    
    template<typename T, const T PS = platformSeparator<T>(), const T AS = additionalSeparator<T>()>
    inline plzma_size_t normalize(T * LIBPLZMA_NULLABLE str) noexcept {
        if (!str) {
            return 0;
        }
        size_t sepsCount = 0, compsCount = 0, skiped = 0;
        T * t = str, * f = str;
        while (*t) {
            if (skiped) { *++t = *++f; } // if (skiped) == if (t != f)
            if (PS == *t || AS == *t) {
                *t = PS;
                if (++sepsCount == 1) {
                    compsCount++;
                    if (t == f) { t++; f++; }
                    continue;
                }
                if (compsCount == 1 && isScheme<T>(str, t)) {
                    if (t == f) { t++; f++; }
                    continue;
                }
                while (*f && (PS == *f || AS == *f)) {
                    f++; skiped++;
                }
                *t = *f;
                sepsCount = 0;
            } else {
                if (t == f) { t++; f++; }
                sepsCount = 0;
            }
        }
        return static_cast<plzma_size_t>(skiped);
    }
    
    template<typename T, const T PS = platformSeparator<T>()>
    inline Pair<const T *, size_t> lastComp(const T * LIBPLZMA_NONNULL path, const size_t length) {
        Pair<const T *, size_t> res;
        const T * p = path + length;
        size_t len = 0;
        do {
            if (*--p == PS) {
                if (isScheme<T>(path, p) || len > 0) {
                    res.first = ++p;
                    res.second = len;
                    return res;
                }
                continue;
            }
            len++;
        } while (p != path);
        res.first = p;
        res.second = len;
        return res;
    }
    
    template<typename T, const T PS = platformSeparator<T>()>
    inline bool removeLastComp(T * LIBPLZMA_NONNULL path, const size_t length) noexcept {
        T * p = path + length;
        size_t len = 0;
        do {
            if (*--p == PS) {
                if (isScheme<T>(path, p)) {
                    return len > 0;
                }
                if (len > 0) {
                    if (p != path) {
                        *p = 0;
                    }
                    return true;
                }
                continue;
            }
            len++;
            *p = 0;
        } while (p != path);
        return len > 0;
    }
    
    template<typename T, const T PS = platformSeparator<T>()>
    plzma_size_t appendRandComp(RawHeapMemory & mem, const size_t len, const size_t compLen = 8) {
        mem.resize(sizeof(T) * (compLen + len + 2));
        plzma_size_t appended = 0;
        T * path = static_cast<T *>(mem);
        if (len > 0) {
            path += len - 1;
            if (*path != PS) {
                *++path = PS;
                appended++;
            }
            path++;
        }
        for (size_t i = 0; i < compLen; i++) {
            *path++ = static_cast<T>(plzma_random_in_range('a', 'z'));
            *path = 0;
            appended++;
            if (i > 2 && !pathExists<T>(mem, nullptr)) {
                return appended;
            }
        }
        static_cast<T *>(mem)[len] = 0;
        if (compLen < 32) {
            return appendRandComp<T>(mem, len, static_cast<size_t>(static_cast<double>(compLen) * 1.5));
        }
        throw Exception(plzma_error_code_internal, "Can't append random component.", __FILE__, __LINE__);
    }
    
    template<typename T, const T PS = platformSeparator<T>()>
    inline bool pathsAreEqual(const T * LIBPLZMA_NONNULL a, const T * LIBPLZMA_NONNULL b,
                              size_t alen, size_t blen) noexcept {
        while (alen && a[alen - 1] == PS) {
            alen--;
        }
        while (blen && b[blen - 1] == PS) {
            blen--;
        }
        if (alen == blen) {
            return alen ? (memcmp(a, b, sizeof(T) * alen) == 0) : true;
        }
        return false;
    }
    
#if defined(LIBPLZMA_MSC)
    struct RAIIFindHANDLE final {
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(RAIIFindHANDLE)
        HANDLE handle = INVALID_HANDLE_VALUE;
        RAIIFindHANDLE() = default;
        ~RAIIFindHANDLE() {
            if (handle != INVALID_HANDLE_VALUE) {
                FindClose(handle);
            }
        }
    };
    
    LIBPLZMA_CPP_API_PRIVATE(bool) removeDir(Path & path, const bool skipErrors);
#elif defined(LIBPLZMA_POSIX)
    struct RAIIDIR final {
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(RAIIDIR)
        DIR * LIBPLZMA_NULLABLE dir = nullptr;
        RAIIDIR() = default;
        ~RAIIDIR() {
            if (dir) {
                closedir(dir);
            }
        }
    };
    
    LIBPLZMA_CPP_API_PRIVATE(bool) removeDir(Path & path, const bool skipErrors);
#endif
    template<typename T>
    bool removePath(const T * LIBPLZMA_NONNULL path, const bool skipErrors) {
        bool isDir = false;
        if (pathExists<T>(path, &isDir)) {
            if (isDir) {
                Path copy(path);
                return removeDir(copy, skipErrors);
            } else {
                const bool res = removeFile<T>(path);
                return (res || skipErrors);
            }
        }
        return true;
    }
    
} // namespace plzma
} // namespace pathUtils

#endif // !__PLZMA_PATH_UTILS_HPP__
