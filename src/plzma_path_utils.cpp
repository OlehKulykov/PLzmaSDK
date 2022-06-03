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

#include "plzma_path_utils.hpp"

namespace plzma {
namespace pathUtils {

#if defined(LIBPLZMA_MSC)
    bool removeDir(Path & path, const bool skipErrors) {
        path.append(L"*");
        WIN32_FIND_DATAW findData;
        RAIIFindHANDLE handle;
        handle.handle = FindFirstFileW(path.wide(), &findData);
        path.removeLastComponent();
        if (handle.handle == INVALID_HANDLE_VALUE) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) { // function fails because no matching files can be found
                const bool res = removeEmptyDir<wchar_t>(path.wide());
                return (res || skipErrors);
            }
        } else {
            BOOL findNextRes = TRUE;
            do {
                if (!findNextRes) {
                    findNextRes = FindNextFileW(handle.handle, &findData);
                }
                if (findNextRes) {
                    findNextRes = FALSE;
                    if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0 || wcslen(findData.cFileName) == 0) { continue; }
                    path.append(findData.cFileName);
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        const bool res = removeDir(path, skipErrors);
                        path.removeLastComponent();
                        if (res || skipErrors) { continue; }
                        return false;
                    } else {
                        const bool res = removeFile<wchar_t>(path.wide());
                        path.removeLastComponent();
                        if (res || skipErrors) { continue; }
                        return false;
                    }
                } else {
                    FindClose(handle.handle);
                    handle.handle = INVALID_HANDLE_VALUE;
                    const bool res = removeEmptyDir<wchar_t>(path.wide());
                    return (res || skipErrors);
                }
            } while (TRUE);
            return true;
        }
        return skipErrors;
    }
#elif defined(LIBPLZMA_POSIX)
    bool removeDir(Path & path, const bool skipErrors) {
        RAIIDIR dir;
        if ( (dir.dir = opendir(path.utf8())) ) {
            struct dirent d, * dp;
            int readRes;
            do {
                if ( (readRes = readdir_r(dir.dir, &d, &dp)) == 0 && dp) {
                    if (strcmp(d.d_name, ".") == 0 || strcmp(d.d_name, "..") == 0 || strlen(d.d_name) == 0) { continue; }
                    path.append(d.d_name);
                    bool isDir = false, isFile = false, isLink = false;
                    if (d.d_type != DT_UNKNOWN) {
                        isDir = d.d_type == DT_DIR;
                        isFile = d.d_type == DT_REG;
                        isLink = d.d_type == DT_LNK;
                    } else {
                        struct stat statbuf;
                        if (access(path.utf8(), F_OK) == 0 && stat(path.utf8(), &statbuf) == 0) {
                            isDir = S_ISDIR(statbuf.st_mode);
                            isFile = S_ISREG(statbuf.st_mode);
                            isLink = S_ISLNK(statbuf.st_mode);
                        } else {
                            path.removeLastComponent();
                            if (skipErrors) { continue; }
                            return false;
                        }
                    }
                    if (isFile || isLink) {
                        const bool res = removeFile<char>(path.utf8());
                        path.removeLastComponent();
                        if (res || skipErrors) { continue; }
                        return false;
                    } else if (isDir) {
                        const bool res = removeDir(path, skipErrors);
                        path.removeLastComponent();
                        if (res || skipErrors) { continue; }
                        return false;
                    } else {
                        path.removeLastComponent();
                        continue;
                    }
                } else {
                    closedir(dir.dir);
                    dir.dir = nullptr;
                    const bool res = removeEmptyDir<char>(path.utf8());
                    return (res || skipErrors);
                }
            } while (readRes == 0 && dp);
            return true;
        }
        return skipErrors;
    }
#endif

} // namespace pathUtils
} // namespace plzma
