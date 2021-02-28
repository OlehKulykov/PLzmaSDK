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

#include "plzma_out_streams.hpp"
#include "plzma_common.hpp"
#include "plzma_file_utils.hpp"

#include "CPP/Common/MyString.h"

namespace plzma {

    using namespace fileUtils;
    
    void OutStreamBase::retain() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RETAIN_IMPL(__m_RefCount)
#else
        LIBPLZMA_RETAIN_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    void OutStreamBase::release() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RELEASE_IMPL(__m_RefCount)
#else
        LIBPLZMA_RELEASE_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
        
    OutStreamBase::OutStreamBase() : CMyUnknownImp() {
        
    }
    
    /// OutFileStream
    STDMETHODIMP OutFileStream::Write(const void * data, UInt32 size, UInt32 * processedSize) {
        if (_file) {
            const size_t processed = (size > 0) ? fwrite(data, 1, size, _file) : 0;
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, processed)
            return S_OK;
        }
        return S_FALSE;
    }
    
    STDMETHODIMP OutFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        if (_file && fileSeek(_file, offset, seekOrigin) == 0) {
            LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, fileTell(_file))
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0)
        return S_FALSE;
    }
    
    STDMETHODIMP OutFileStream::SetSize(UInt64 newSize) {
        return S_OK;
    }
    
    bool OutFileStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _file != nullptr;
    }
    
    void OutFileStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_file) {
            return;
        }
        FILE * f = _path.openFile("w+b");
        if (f) {
            _file = f;
        } else {
            Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
            exception.setWhat("Can't open out-stream for writing to file in binary mode with path: ", _path.utf8(), nullptr);
            exception.setReason("You don't have write permission or parent directory doesn't exist.", nullptr);
            throw exception;
        }
    }
    
    void OutFileStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_file) {
            fclose(_file);
            _file = nullptr;
        }
    }
    
    bool OutFileStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_file) {
            return false; // opened -> false
        }
        bool isDir = true;
        if (_path.exists(&isDir)) {
            if (!isDir && !fileErase(_path, eraseType)) {
                return false;
            }
            return _path.remove(false);
        }
        return true;
    }
    
    RawHeapMemorySize OutFileStream::copyContent() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _file ? RawHeapMemorySize(RawHeapMemory(), 0) : fileContent(_path);
    }
    
    OutFileStream::OutFileStream(const Path & path) : OutStreamBase(),
        _path(path) {
            if (_path.count() == 0) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate out-stream without path.", __FILE__, __LINE__);
                exception.setReason("The path size is zero.", nullptr);
                throw exception;
            }
    }
    
    OutFileStream::OutFileStream(Path && path) : OutStreamBase(),
        _path(static_cast<Path &&>(path)) {
            if (_path.count() == 0) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate out-stream without path.", __FILE__, __LINE__);
                exception.setReason("The path size is zero.", nullptr);
                throw exception;
            }
    }
    
    OutFileStream::~OutFileStream() noexcept {
        if (_file) {
            fclose(_file);
        }
    }
    
    /// OutMemStream
    STDMETHODIMP OutMemStream::Write(const void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            const uint64_t dstSize = _offset + static_cast<uint64_t>(size);
            if (_size < dstSize) {
                if (dstSize >= plzma_max_size()) {
                    _size = _offset = 0;
                    _opened = false;
                    return E_OUTOFMEMORY;
                }
                try {
                    _memory.resize(static_cast<size_t>(dstSize));
                } catch (...) {
                    _size = _offset = 0;
                    _opened = false;
                    return E_OUTOFMEMORY;
                }
                _size = dstSize;
            }
            memcpy(static_cast<uint8_t *>(_memory) + _offset, data, static_cast<size_t>(size));
            _offset += size;
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, size)
            return S_OK;
        }
        return S_FALSE;
    }
    
    STDMETHODIMP OutMemStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        if (_opened) {
            Int64 finalOffset;
            switch (seekOrigin) {
                case STREAM_SEEK_SET:
                    finalOffset = offset;
                    break;
                case STREAM_SEEK_CUR:
                    finalOffset = _offset;
                    finalOffset += offset;
                    break;
                case STREAM_SEEK_END:
                    finalOffset = _size;
                    finalOffset += offset;
                    break;
                default:
                    finalOffset = -1;
                    break;
            }
            if (finalOffset >= 0 && static_cast<UInt64>(finalOffset) <= _size) {
                _offset = finalOffset;
                LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, _offset)
                return S_OK;
            }
        }
        return S_FALSE;
    }
    
    STDMETHODIMP OutMemStream::SetSize(UInt64 newSize) {
        if (_opened) {
            if (newSize >= plzma_max_size()) {
                _size = _offset = 0;
                _opened = false;
                return E_OUTOFMEMORY;
            }
            try {
                _memory.resize(static_cast<size_t>(newSize));
            } catch (...) {
                _size = _offset = 0;
                _opened = false;
                return E_OUTOFMEMORY;
            }
            _size = static_cast<uint64_t>(newSize);
            return S_OK;
        }
        return S_FALSE;
    }
    
    bool OutMemStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }
    
    void OutMemStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (!_opened) {
            _size = _offset = 0;
            _opened = true;
        }
    }
    
    void OutMemStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _offset = 0;
        _opened = false;
    }
    
    bool OutMemStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return false; // opened -> false
        }
        _memory.clear(eraseType);
        _size = _offset = 0;
        return true;
    }
    
    RawHeapMemorySize OutMemStream::copyContent() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        RawHeapMemorySize content(RawHeapMemory(), 0);
        if (!_opened && _size > 0) {
            const size_t size = static_cast<size_t>(_size);
            content.first.resize(size);
            memcpy(static_cast<void *>(content.first), static_cast<const void *>(_memory), size);
            content.second = size;
        }
        return content;
    }
    
    /// OutTestStream
    STDMETHODIMP OutTestStream::Write(const void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, size)
            return S_OK;
        }
        return S_FALSE;
    }
    
    STDMETHODIMP OutTestStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        return S_OK;
    }
    
    STDMETHODIMP OutTestStream::SetSize(UInt64 newSize) {
        return S_OK;
    }
    
    bool OutTestStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }
    
    void OutTestStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _opened = true;
    }
    
    void OutTestStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _opened = false;
    }
    
    bool OutTestStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        // nothing to erase
        return !_opened; // opened -> false
    }
    
    RawHeapMemorySize OutTestStream::copyContent() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        // nothing to copy
        return RawHeapMemorySize(RawHeapMemory(), 0);
    }

    SharedPtr<OutStream> makeSharedOutStream(const Path & path) {
        return SharedPtr<OutStream>(new OutFileStream(path));
    }
    
    SharedPtr<OutStream> makeSharedOutStream(Path && path) {
        return SharedPtr<OutStream>(new OutFileStream(static_cast<Path &&>(path)));
    }
    
    SharedPtr<OutStream> makeSharedOutStream(void) {
        return SharedPtr<OutStream>(new OutMemStream());
    }

} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_out_stream plzma_out_stream_create_with_path(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_stream, path)
    auto stream = makeSharedOutStream(*static_cast<const Path *>(path->object));
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_out_stream plzma_out_stream_create_with_pathm(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_stream, path)
    auto stream = makeSharedOutStream(static_cast<Path &&>(*static_cast<Path *>(path->object)));
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_out_stream plzma_out_stream_create_memory_stream(void) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_out_stream)
    auto stream = makeSharedOutStream();
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_memory plzma_out_stream_copy_content(plzma_out_stream * LIBPLZMA_NONNULL stream) {
    plzma_memory content;
    content.memory = nullptr;
    content.size = 0;
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, content)
    auto cnt = static_cast<const OutStream *>(stream->object)->copyContent();
    content.memory = cnt.first.take();
    content.size = cnt.second;
    return content;
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(stream, content)
}

bool plzma_out_stream_opened(plzma_out_stream * LIBPLZMA_NULLABLE stream) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, false)
    return static_cast<OutStream *>(stream->object)->opened();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(stream, false)
}

bool plzma_out_stream_erase(plzma_out_stream * LIBPLZMA_NONNULL stream, const plzma_erase erase_type) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, false)
    return static_cast<OutStream *>(stream->object)->erase(erase_type);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(stream, false)
}

void plzma_out_stream_release(plzma_out_stream * LIBPLZMA_NONNULL stream) {
    plzma_object_exception_release(stream);
    SharedPtr<OutStream> streamSPtr;
    streamSPtr.assign(static_cast<OutStream *>(stream->object));
    stream->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
