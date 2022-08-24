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
#include "plzma_path_utils.hpp"

namespace plzma {

    using namespace pathUtils;
    
    /// Iterator
    
    enum PathIteratorFlags : uint16_t {
        PathIteratorFlagIsDir           = 1 << 0,
        PathIteratorFlagSkipPathConcat  = 1 << 1,
        PathIteratorFlagDone            = 1 << 2,
        PathIteratorFlagSkipFindNext    = 1 << 3
    };
    
    Path Path::Iterator::path() const {
        Path res(_path);
        if ( !(_flags & PathIteratorFlagSkipPathConcat) ) {
            res.append(_component);
        }
        return res;
    }
    
    const Path & Path::Iterator::component() const noexcept {
        return _component;
    }
    
    Path Path::Iterator::fullPath() const {
        Path res(_root);
        if ( !(_flags & PathIteratorFlagSkipPathConcat) ) {
            res.append(_component);
        }
        return res;
    }
    
    bool Path::Iterator::isDir() const noexcept {
        return (_flags & PathIteratorFlagIsDir) ? true : false;
    }
    
    void Path::Iterator::clearPaths() noexcept {
        _root.clear(plzma_erase_zero);
        _path.clear(plzma_erase_zero);
        _component.clear(plzma_erase_zero);
    }
    
    Path::Iterator::Iterator(const Path & root, const plzma_open_dir_mode_t mode) :
        _root(root),
        _mode(mode) {
            
    }

#if defined(LIBPLZMA_MSC)
    class PathIteratorMSC final : public Path::Iterator {
    private:
        HANDLE _findHandle = INVALID_HANDLE_VALUE;
        Vector<HANDLE> _stack;
        WIN32_FIND_DATAW _findData;
        plzma_size_t _referenceCounter = 0;
        
        void closeMSC() noexcept {
            _flags |= PathIteratorFlagDone;
            plzma_size_t i = _stack.count();
            if (i > 0) {
                do {
                    FindClose(_stack.at(--i));
                } while (i > 0);
                _stack.clear();
            }
            if (_findHandle != INVALID_HANDLE_VALUE) {
                FindClose(_findHandle);
                _findHandle = INVALID_HANDLE_VALUE;
            }
            clearPaths();
        }
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(PathIteratorMSC)
    
    protected:
        virtual void retain() noexcept override final {
            LIBPLZMA_RETAIN_IMPL(_referenceCounter)
        }
        
        virtual void release() noexcept override final {
            LIBPLZMA_RELEASE_IMPL(_referenceCounter)
        }
        
    public:
        virtual void close() noexcept override final {
            closeMSC();
        }
        
        virtual bool next() override final {
            if (_flags & PathIteratorFlagDone) { return false; }
            
            BOOL reading = TRUE;
            BOOL findNextRes = (_flags & PathIteratorFlagSkipFindNext) ? TRUE : FALSE;
            _flags = 0;
            do {
                if (!findNextRes) {
                    findNextRes = FindNextFileW(_findHandle, &_findData);
                }
                if (findNextRes) {
                    findNextRes = FALSE;
                    if (wcscmp(_findData.cFileName, L".") == 0 || wcscmp(_findData.cFileName, L"..") == 0) { continue; }
                    const bool isLink = ((_findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (_findData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)) ? true : false;
                    if (isLink && !(_mode & plzma_open_dir_mode_follow_symlinks)) { continue; }
                    _component.set(_findData.cFileName);
                    if (_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        Path root(_root);
                        root.append(_component);
                        root.append(L"*");
                        RAIIFindHANDLE subHandle;
                        subHandle.handle = FindFirstFileW(root.wide(), &_findData);
                        if (subHandle.handle == INVALID_HANDLE_VALUE) {
                            continue;
                        } else {
                            _stack.push(_findHandle);
                            _findHandle = subHandle.handle;
                            subHandle.handle = INVALID_HANDLE_VALUE;
                            root.removeLastComponent(); // remove '/*'
                            _root = static_cast<Path &&>(root);
                            _path.append(_component);
                            _flags |= (PathIteratorFlagSkipPathConcat | PathIteratorFlagIsDir | PathIteratorFlagSkipFindNext);
                            return true; // report dir
                        }
                    } else {
                        //TODO: filter & skip unsupported attribs if needed
                        //https://docs.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
                        return true; // report file
                    }
                } else if (_stack.count() > 0) {
                    _root.removeLastComponent();
                    _path.removeLastComponent();
                    _findHandle = _stack.at(_stack.count() - 1);
                    _stack.pop();
                    continue;
                } else {
                    reading = FALSE; // no files, exit
                }
            } while (reading);

            _flags |= PathIteratorFlagDone;
            return false;
        }
        
        PathIteratorMSC(const Path & root, const plzma_open_dir_mode_t mode) : Path::Iterator(root, mode) {
            Path path(root);
            if (path.count() > 0) {
                path.append(L"*");
            } else {
                path.set(L"." CLZMA_SEP_WSTR L"*");
            }
            RAIIFindHANDLE subHandle;
            subHandle.handle = FindFirstFileW(path.wide(), &_findData);
            if (subHandle.handle == INVALID_HANDLE_VALUE) {
                if (GetLastError() == ERROR_FILE_NOT_FOUND) { // function fails because no matching files can be found
                    clearPaths();
                    _flags |= PathIteratorFlagDone;
                } else {
                    Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                    exception.setWhat("Can't open and iterate path: ", _root.utf8(), nullptr);
                    exception.setReason("Path not found or not a directory.", nullptr);
                    clearPaths();
                    throw exception;
                }
            } else {
                _findHandle = subHandle.handle;
				subHandle.handle = INVALID_HANDLE_VALUE;
                _flags |= PathIteratorFlagSkipFindNext;
            }
        }
        
        virtual ~PathIteratorMSC() noexcept {
            closeMSC();
        }
    };
#elif defined(LIBPLZMA_POSIX)
    class PathIteratorPosix final : public Path::Iterator {
    private:
        DIR * _dir = nullptr;
        Vector<DIR *> _stack;
        plzma_size_t _referenceCounter = 0;
        
        void closePosix() noexcept {
            _flags |= PathIteratorFlagDone;
            plzma_size_t i = _stack.count();
            if (i > 0) {
                do {
                    closedir(_stack.at(--i));
                } while (i > 0);
                _stack.clear();
            }
            if (_dir) {
                closedir(_dir);
                _dir = nullptr;
            }
            clearPaths();
        }
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(PathIteratorPosix)
        
    protected:
        virtual void retain() noexcept override final {
            LIBPLZMA_RETAIN_IMPL(_referenceCounter)
        }
        
        virtual void release() noexcept override final {
            LIBPLZMA_RELEASE_IMPL(_referenceCounter)
        }
        
    public:
        virtual void close() noexcept override final {
            closePosix();
        }
        
        virtual bool next() override final {
            if (_flags & PathIteratorFlagDone) { return false; }
            _flags = 0;
            
            Path root;
            const char * rootUtf8;
            struct dirent d, * dp;
            int readRes;
            do {
                if ( (readRes = readdir_r(_dir, &d, &dp)) == 0 && dp) {
                    if (strcmp(d.d_name, ".") == 0 || strcmp(d.d_name, "..") == 0) { continue; }
                    bool isDir = false, isFile = false, isLink = false;
                    rootUtf8 = nullptr;
                    if (d.d_type != DT_UNKNOWN) {
                        isDir = d.d_type == DT_DIR;
                        isFile = d.d_type == DT_REG;
                        isLink = d.d_type == DT_LNK;
                    } else {
                        root.set(_root);
                        root.append(d.d_name);
                        rootUtf8 = root.utf8();
                        struct stat statbuf;
                        if (access(rootUtf8, F_OK) == 0 && stat(rootUtf8, &statbuf) == 0) {
                            isDir = S_ISDIR(statbuf.st_mode);
                            isFile = S_ISREG(statbuf.st_mode);
                            isLink = S_ISLNK(statbuf.st_mode);
                        } else { continue; }
                    }
                    if (isLink && !(_mode & plzma_open_dir_mode_follow_symlinks)) { continue; }
                    _component.set(d.d_name);
                    if (isFile) {
                        return true;
                    } else if (isDir || isLink) {
                        if (!rootUtf8) {
                            root.set(_root);
                            root.append(_component);
                            rootUtf8 = root.utf8();
                        }
                        RAIIDIR dir;
                        dir.dir = opendir(rootUtf8);
                        if (dir.dir) {
                            _stack.push(_dir);
                            _dir = dir.dir;
                            dir.dir = nullptr;
                            _root = static_cast<Path &&>(root);
                            rootUtf8 = nullptr;
                            _path.append(_component);
                        } else { continue; }
                        _flags |= (PathIteratorFlagSkipPathConcat | PathIteratorFlagIsDir);
                        return true; // report dir
                    } else { continue; }
                }
                if (readRes == 0 && !dp && _stack.count() > 0) {
                    _root.removeLastComponent();
                    _path.removeLastComponent();
                    closedir(_dir);
                    _dir = _stack.at(_stack.count() - 1);
                    _stack.pop();
                    dp = &d;
                    continue;
                }
            } while (readRes == 0 && dp);
            
            _flags |= PathIteratorFlagDone;
            return false;
        }
        
        PathIteratorPosix(const Path & root, const plzma_open_dir_mode_t mode) : Path::Iterator(root, mode) {
            const char * path = nullptr;
            if (_root.count() > 0) {
                const char * utf8 = _root.utf8();
                bool isDir = false;
                if (pathExists<char>(utf8, &isDir) && isDir) {
                    path = utf8;
                } else {
                    Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                    exception.setWhat("Can't open and iterate path: ", _root.utf8(), nullptr);
                    exception.setReason("Path not found or not a directory.", nullptr);
                    clearPaths();
                    _flags |= PathIteratorFlagDone;
                    throw exception;
                }
            } else {
                path = ".";
            }
            if ( !(_dir = opendir(path)) ) {
                Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                exception.setWhat("Can't open directory: ", _root.utf8(), nullptr);
                exception.setReason("No open directory permissions.", nullptr);
                clearPaths();
                _flags |= PathIteratorFlagDone;
                throw exception;
            }
        }
        
        virtual ~PathIteratorPosix() noexcept {
            closePosix();
        }
    };
#endif
    
    void Path::set(const String & str) {
#if defined(LIBPLZMA_MSC)
        set(str.wide());
#elif defined(LIBPLZMA_POSIX)
        set(str.utf8());
#endif
    }
    
    void Path::set(const wchar_t * LIBPLZMA_NULLABLE str) {
        copyFrom(str, plzma_erase_zero);
        const auto reduced = normalize<wchar_t>(_ws);
        if (reduced > 0) {
            _size -= reduced;
        }
    }
    
    void Path::set(const char * LIBPLZMA_NULLABLE str) {
        copyFrom(str, plzma_erase_zero);
        const auto reduced = normalize<char>(_cs);
        if (reduced > 0) {
            _size -= reduced;
            _cslen -= reduced;
        }
    }
    
    void Path::append(const wchar_t * LIBPLZMA_NULLABLE str) {
        syncWide();
        const size_t len = str ? wcslen(str) : 0;
        if (len > 0) {
            const wchar_t * stringsList[2];
            Pair<size_t, size_t> sizesList[2];
            size_t count = 0;
            if (requireSeparator<wchar_t>(_ws, _size, str)) {
                stringsList[count] = CLZMA_SEP_WSTR;
                sizesList[count].first = sizesList[count].second = 1;
                count++;
            }
            stringsList[count] = str;
            sizesList[count].first = sizesList[count].second = len;
            String::append(stringsList, sizesList, ++count, plzma_erase_zero);
            const auto reduced = normalize<wchar_t>(_ws);
            if (reduced > 0) {
                _size -= reduced;
            }
        }
    }
    
    void Path::append(const char * LIBPLZMA_NULLABLE str) {
        syncUtf8();
        const auto len = lengthMaxCount(str, static_cast<size_t>(plzma_max_size()));
        if (len.first > 0) {
            const char * stringsList[2];
            Pair<size_t, size_t> sizesList[2];
            size_t count = 0;
            if (requireSeparator<char>(_cs, _cslen, str)) {
                stringsList[count] = CLZMA_SEP_CSTR;
                sizesList[count].first = sizesList[count].second = 1;
                count++;
            }
            stringsList[count] = str;
            sizesList[count] = len;
            String::append(stringsList, sizesList, ++count, plzma_erase_zero);
            const auto reduced = normalize<char>(_cs);
            if (reduced > 0) {
                _size -= reduced;
                _cslen -= reduced;
            }
        }
    }
    
    void Path::append(const Path & path) {
#if defined(LIBPLZMA_MSC)
        append(path.wide());
#elif defined(LIBPLZMA_POSIX)
        append(path.utf8());
#endif
    }
    
    Path Path::appending(const wchar_t * LIBPLZMA_NULLABLE str) const {
        Path result(*this);
        result.append(str);
        return result;
    }

    Path Path::appending(const char * LIBPLZMA_NULLABLE str) const {
        Path result(*this);
        result.append(str);
        return result;
    }

    Path Path::appending(const Path & path) const {
        Path result(*this);
        result.append(path);
        return result;
    }

    void Path::appendRandomComponent() {
#if defined(LIBPLZMA_MSC)
        syncWide();
        _cs.clear(plzma_erase_zero, sizeof(char) * _cslen);
        _size += appendRandComp<wchar_t>(_ws, _size);
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        _ws.clear(plzma_erase_zero, sizeof(wchar_t) * _size);
        const auto appended = appendRandComp<char>(_cs, _cslen);
        _size += appended;
        _cslen += appended;
#endif
    }
    
    Path Path::appendingRandomComponent() const {
        Path result(*this);
        result.appendRandomComponent();
        return result;
    }

    Path Path::lastComponent() const {
        Path res;
#if defined(LIBPLZMA_MSC)
        syncWide();
        if (_ws && _size > 0) {
            const auto comp = lastComp<wchar_t>(_ws, _size);
            if (comp.second > 0) {
                const wchar_t * stringsList[1] = { comp.first };
                Pair<size_t, size_t> sizesList[1];
                sizesList[0].first = sizesList[0].second = comp.second;
                res.String::append(stringsList, sizesList, 1, plzma_erase_zero);
            }
        }
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        if (_cs && _cslen > 0) {
            const auto comp = lastComp<char>(_cs, _cslen);
            if (comp.second > 0) {
                const char * stringsList[1] = { comp.first };
                Pair<size_t, size_t> sizesList[1] = { lengthMaxLength(comp.first, comp.second) };
                res.String::append(stringsList, sizesList, 1, plzma_erase_zero);
            }
        }
#endif
        return res;
    }
    
    void Path::removeLastComponent() {
#if defined(LIBPLZMA_MSC)
        syncWide();
        if (_ws && _size > 0 && removeLastComp<wchar_t>(_ws, _size)) {
            _size = static_cast<plzma_size_t>(wcslen(_ws));
            _cs.clear(plzma_erase_zero, sizeof(char) * _cslen);
            _cslen = 0;
        }
#elif defined(LIBPLZMA_POSIX)
        syncUtf8();
        if (_cs && _cslen > 0 && removeLastComp<char>(_cs, _cslen)) {
            const auto len = String::lengthMaxCount(_cs, static_cast<size_t>(plzma_max_size()));
            _ws.clear(plzma_erase_zero, sizeof(wchar_t) * _size);
            _cslen = static_cast<plzma_size_t>(len.first);
            _size = static_cast<plzma_size_t>(len.second);
        }
#endif
    }
    
    Path Path::removingLastComponent() const {
        Path result(*this);
        result.removeLastComponent();
        return result;
    }
    
    bool Path::exists(bool * LIBPLZMA_NULLABLE isDir /* = nullptr */) const {
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? pathExists<wchar_t>(wide(), isDir) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathExists<char>(utf8(), isDir) : false;
#endif
    }
    
    bool Path::readable() const {
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? pathReadable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathReadable<char>(utf8()) : false;
#endif
    }
    
    bool Path::writable() const {
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? pathWritable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathWritable<char>(utf8()) : false;
#endif
    }
    
    bool Path::readableAndWritable() const {
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? pathReadableAndWritable<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? pathReadableAndWritable<char>(utf8()) : false;
#endif
    }
    
    plzma_path_stat Path::stat() const {
#if defined(LIBPLZMA_MSC)
        return pathStat<wchar_t>(wide());
#elif defined(LIBPLZMA_POSIX)
        return pathStat<char>(utf8());
#endif
    }
    
    bool Path::remove(const bool skipErrors) const {
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? removePath<wchar_t>(wide(), skipErrors) : true;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? removePath<char>(utf8(), skipErrors) : true;
#endif
    }
    
    bool Path::createDir(const bool withIntermediates) const {
        if (withIntermediates) {
            if (_size > 0) {
#if defined(LIBPLZMA_MSC)
                const wchar_t * w = wide(); // syncWide
                return createIntermediateDirs<wchar_t>(w, _size);
#elif defined(LIBPLZMA_POSIX)
                const char * c = utf8(); // syncUtf8
                return createIntermediateDirs<char>(c, _cslen);
#endif
            }
            return false;
        }
#if defined(LIBPLZMA_MSC)
        return (_size > 0) ? createSingleDir<wchar_t>(wide()) : false;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? createSingleDir<char>(utf8()) : false;
#endif
    }
    
    FILE * LIBPLZMA_NULLABLE Path::openFile(const char * LIBPLZMA_NONNULL mode) const {
#if defined(LIBPLZMA_MSC)
        if (_size > 0) {
            wchar_t wmode[32] = { 0 }; // more than enough for a max mode: "w+b, ccs=UNICODE"
            for (size_t i = 0, n = strlen(mode); ((i < n) && (i < 31)); i++) {
                wmode[i] = static_cast<wchar_t>(mode[i]);
            }
            return _wfopen(wide(), wmode);
        }
        return nullptr;
#elif defined(LIBPLZMA_POSIX)
        return (_size > 0) ? fopen(utf8(), mode) : nullptr;
#endif
    }
    
    SharedPtr<Path::Iterator> Path::openDir(const plzma_open_dir_mode_t mode /* = 0 */ ) const {
#if defined(LIBPLZMA_MSC)
        return SharedPtr<Path::Iterator>(new PathIteratorMSC(*this, mode));
#elif defined(LIBPLZMA_POSIX)
        return SharedPtr<Path::Iterator>(new PathIteratorPosix(*this, mode));
#endif
    }
    
    bool Path::operator == (const Path & path) const {
#if defined(LIBPLZMA_MSC)
        const wchar_t * a = wide();       // syncWide
        const wchar_t * b = path.wide();  // syncWide
        return pathsAreEqual<wchar_t>(a, b, _size, path._size);
#elif defined(LIBPLZMA_POSIX)
        const char * a = utf8();        // syncUtf8
        const char * b = path.utf8();   // syncUtf8
        return pathsAreEqual<char>(a, b, _cslen, path._cslen);
#endif
    }
    
    Path & Path::operator = (Path && path) noexcept {
        moveFrom(static_cast<Path &&>(path), plzma_erase_zero);
        return *this;
    }
    
    Path::Path(Path && path) noexcept : String(static_cast<Path &&>(path)) {
        
    }
    
    Path & Path::operator = (const Path & path) {
        copyFrom(path, plzma_erase_zero);
        return *this;
    }
    
    Path::Path(const Path & path) : String(path) {
        
    }
    
    Path::Path(const wchar_t * LIBPLZMA_NULLABLE path) : String(path) {
        const auto reduced = normalize<wchar_t>(_ws);
        if (reduced > 0) {
            _size -= reduced;
        }
    }
    
    Path::Path(const char * LIBPLZMA_NULLABLE path) : String(path) {
        const auto reduced = normalize<char>(_cs);
        if (reduced > 0) {
            _size -= reduced;
            _cslen -= reduced;
        }
    }
    
    Path::~Path() noexcept {
        _ws.erase(plzma_erase_zero, sizeof(wchar_t) * _size);
        _cs.erase(plzma_erase_zero, sizeof(char) * _cslen);
    }
    
#if !defined(PATH_MAX)
#define PATH_MAX 1024
#endif
    
    Path Path::tmpPath() {
        Path path;
#if defined(__APPLE__) && defined(_CS_DARWIN_USER_TEMP_DIR)
        char buff[PATH_MAX];
        const size_t res = confstr(_CS_DARWIN_USER_TEMP_DIR, buff, PATH_MAX);
        if (res > 0 && res < PATH_MAX && initializeTmpPath<char>(buff, "libplzma", path)) {
            return path;
        }
#endif
#if defined(LIBPLZMA_MSC)
        static const wchar_t * const wevs[4] = { L"TMPDIR", L"TEMPDIR", L"TEMP", L"TMP" };
        for (size_t i = 0; i < 4; i++) {
            const wchar_t * p = _wgetenv(wevs[i]);
            if (p && initializeTmpPath<wchar_t>(p, L"libplzma", path)) {
                return path;
            }
        }
#endif
        static const char * const cevs[4] = { "TMPDIR", "TEMPDIR", "TEMP", "TMP" };
        for (size_t i = 0; i < 4; i++) {
            char * p = getenv(cevs[i]);
            if (p && initializeTmpPath<char>(p, "libplzma", path)) {
                return path;
            }
        }
#if !defined(LIBPLZMA_OS_WINDOWS)
        if (initializeTmpPath<char>("/tmp", "libplzma", path)) {
            return path;
        }
#endif
        return Path();
    }
    
} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_path plzma_path_create_with_wide_string(const wchar_t * LIBPLZMA_NULLABLE path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    createdCObject.object = static_cast<void *>(new Path(path));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_create_with_utf8_string(const char * LIBPLZMA_NULLABLE path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    createdCObject.object = static_cast<void *>(new Path(path));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_create_with_tmp_dir(void) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_path)
    auto tmp = Path::tmpPath();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(tmp)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_size_t plzma_path_count(const plzma_path * LIBPLZMA_NONNULL path) {
    return path->exception ? 0 : static_cast<const Path *>(path->object)->count();
}

void plzma_path_set_wide_string(plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE str) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->set(str);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

void plzma_path_set_utf8_string(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE str) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->set(str);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

void plzma_path_append_wide_component(plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->append(component);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_wide_component(const plzma_path * LIBPLZMA_NONNULL path, const wchar_t * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appending(component);
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_append_utf8_component(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->append(component);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_utf8_component(const plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NULLABLE component) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appending(component);
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_append_random_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->appendRandomComponent();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_appending_random_component(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->appendingRandomComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

const wchar_t * LIBPLZMA_NULLABLE plzma_path_wide_string(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->wide();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

const char * LIBPLZMA_NULLABLE plzma_path_utf8_string(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->utf8();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

bool plzma_path_exists(plzma_path * LIBPLZMA_NONNULL path, bool * LIBPLZMA_NULLABLE isDir) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->exists(isDir);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_readable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->readable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_writable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->writable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

bool plzma_path_readable_and_writable(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->readableAndWritable();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

plzma_path_stat plzma_path_get_stat(plzma_path * LIBPLZMA_NONNULL path) {
    plzma_path_stat emptyStat{0, 0, 0, 0};
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, emptyStat)
    return static_cast<Path *>(path->object)->stat();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, emptyStat)
}

void plzma_path_clear(plzma_path * LIBPLZMA_NONNULL path, const plzma_erase erase_type) {
    plzma_object_exception_release(path);
    static_cast<Path *>(path->object)->clear(erase_type);
}

bool plzma_path_remove(plzma_path * LIBPLZMA_NONNULL path, const bool skip_errors) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->remove(skip_errors);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

plzma_path plzma_path_last_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    auto comp = static_cast<Path *>(path->object)->lastComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(comp)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_remove_last_component(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(path)
    static_cast<Path *>(path->object)->removeLastComponent();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(path)
}

plzma_path plzma_path_removing_last_component(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, path)
    Path result = static_cast<const Path *>(path->object)->removingLastComponent();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(result)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

bool plzma_path_create_dir(plzma_path * LIBPLZMA_NONNULL path, const bool with_intermediates) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, false)
    return static_cast<Path *>(path->object)->createDir(with_intermediates);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, false)
}

FILE * LIBPLZMA_NULLABLE plzma_path_open_file(plzma_path * LIBPLZMA_NONNULL path, const char * LIBPLZMA_NONNULL mode) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(path, nullptr)
    return static_cast<Path *>(path->object)->openFile(mode);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(path, nullptr)
}

plzma_path_iterator plzma_path_open_dir(plzma_path * LIBPLZMA_NONNULL path, const plzma_open_dir_mode_t mode) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path_iterator, path)
    auto it = static_cast<Path *>(path->object)->openDir(mode);
    createdCObject.object = static_cast<void *>(it.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_path_release(plzma_path * LIBPLZMA_NULLABLE path) {
    plzma_object_exception_release(path);
    delete static_cast<Path *>(path->object);
    path->object = nullptr;
}

plzma_path plzma_path_iterator_component(const plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, iterator)
    auto comp = static_cast<const Path::Iterator *>(iterator->object)->component();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(comp)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_iterator_path(const plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, iterator)
    auto path = static_cast<const Path::Iterator *>(iterator->object)->path();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(path)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_path_iterator_full_path(const plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, iterator)
    auto fullPath = static_cast<const Path::Iterator *>(iterator->object)->fullPath();
    createdCObject.object = static_cast<void *>(new Path(static_cast<Path &&>(fullPath)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

bool plzma_path_iterator_is_dir(const plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    return iterator->exception ? false : static_cast<const Path::Iterator *>(iterator->object)->isDir();
}

bool plzma_path_iterator_next(plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(iterator, false)
    return static_cast<Path::Iterator *>(iterator->object)->next();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(iterator, false)
}

void plzma_path_iterator_close(plzma_path_iterator * LIBPLZMA_NONNULL iterator) {
    if (!iterator->exception) {
        static_cast<Path::Iterator *>(iterator->object)->close();
    }
}

void plzma_path_iterator_release(plzma_path_iterator * LIBPLZMA_NULLABLE iterator) {
    plzma_object_exception_release(iterator);
    SharedPtr<Path::Iterator> iteratorSPtr;
    iteratorSPtr.assign(static_cast<Path::Iterator *>(iterator->object));
    iterator->object = nullptr;
}

#endif // # !LIBPLZMA_NO_C_BINDINGS
