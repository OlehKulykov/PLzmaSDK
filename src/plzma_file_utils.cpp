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


#include <cstddef>

#include "plzma_file_utils.hpp"

namespace plzma {
namespace fileUtils {
    
    bool fileErase(const Path & path, const plzma_erase eraseType) {
        if (eraseType == plzma_erase_zero) {
            const size_t buffSize = 256 * 1024;
            uint8_t buff[buffSize];
            memset(buff, 0, buffSize);
            FILE * f = path.openFile("r+b");
            if (!f) {
                Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                exception.setWhat("Can't open file for erasing at path: ", path.utf8(), nullptr);
                throw exception;
            }
            
            int64_t size = 0;
            if (fileSeek(f, 0, SEEK_END) == 0) {
                size = fileTell(f);
                if (fileSeek(f, 0, SEEK_SET) != 0) {
                    size = 0;
                }
            }
            
            if (size > 0) {
                for (uint64_t i = 0, n = static_cast<uint64_t>(size) / buffSize; i < n; i++) {
                    const size_t w = fwrite(buff, 1, buffSize, f);
                    if (w != buffSize) {
                        fclose(f);
                        return false;
                    }
                    size -= w;
                }
                if (size > 0) {
                    const size_t w = fwrite(buff, 1, static_cast<size_t>(size), f);
                    if (w != static_cast<size_t>(size)) {
                        fclose(f);
                        return false;
                    }
                }
            }
            fclose(f);
        }
        return true;
    }
    
    RawHeapMemorySize fileContent(const Path & path) {
        RawHeapMemorySize content(RawHeapMemory(), 0);
        FILE * f = path.openFile("rb");
        if (!f) {
            return content;
        }
        
        int64_t fileSize = 0;
        if (fileSeek(f, 0, SEEK_END) == 0) {
            fileSize = fileTell(f);
            if (fileSeek(f, 0, SEEK_SET) != 0) {
                fileSize = 0;
            }
        }
        
        if (fileSize > 0) {
            if (static_cast<uint64_t>(fileSize) > plzma_max_size()) {
                fclose(f);
                Exception exception(plzma_error_code_not_enough_memory, nullptr, __FILE__, __LINE__);
                exception.setWhat("Can't copy file content at path: ", path.utf8(), nullptr);
                exception.setReason("The file size is more than supported.", nullptr);
                throw exception;
            }
            const size_t size = static_cast<size_t>(fileSize);
            try {
                content.first.resize(size);
            } catch (...) {
                fclose(f);
                throw;
            }
            const size_t partSize = 1024 * 1024;
            size_t offset = 0;
            for (size_t i = 0, n = size / partSize; i < n; i++) {
                const size_t read = fread(static_cast<uint8_t *>(content.first) + offset, 1, partSize, f);
                if (read != partSize) {
                    fclose(f);
                    content.first.clear();
                    return content;
                }
                offset += read;
            }
            if (offset < size) {
                const size_t left = size - offset;
                const size_t read = fread(static_cast<uint8_t *>(content.first) + offset, 1, left, f);
                if (read != left) {
                    fclose(f);
                    content.first.clear();
                    return content;
                }
            }
            content.second = size;
        }
        fclose(f);
        
        return content;
    }
    
} // namespace fileUtils
} // namespace plzma
