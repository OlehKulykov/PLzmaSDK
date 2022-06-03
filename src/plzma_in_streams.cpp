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

#include "plzma_in_streams.hpp"
#include "plzma_common.hpp"
#include "plzma_file_utils.hpp"

#include "CPP/Common/MyString.h"

namespace plzma {
    
    using namespace fileUtils;

    void InStreamBase::retain() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RETAIN_IMPL(__m_RefCount)
#else
        LIBPLZMA_RETAIN_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    void InStreamBase::release() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RELEASE_IMPL(__m_RefCount)
#else
        LIBPLZMA_RELEASE_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    InStreamBase::InStreamBase() : CMyUnknownImp() {

    }
    
    /// InFileStream
    STDMETHODIMP InFileStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
        if (_file) {
            const size_t processed = (size > 0) ? fread(data, 1, size, _file) : 0;
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, processed)
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
        return S_FALSE;
    }
    
    STDMETHODIMP InFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        if (_file && fileSeek(_file, offset, seekOrigin) == 0) {
            LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, fileTell(_file))
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0)
        return S_FALSE;
    }
    
    bool InFileStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _file != nullptr;
    }
    
    void InFileStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_file) {
            return;
        }
        FILE * f = _path.openFile("rb");
        if (f) {
            _file = f;
        } else {
            Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
            exception.setWhat("Can't open in-stream for reading from file in binary mode with path: ", _path.utf8(), nullptr);
            exception.setReason("File doesn't exist or is not readable.", nullptr);
            throw exception;
        }
    }
    
    void InFileStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_file) {
            fclose(_file);
            _file = nullptr;
        }
    }
    
    bool InFileStream::erase(const plzma_erase eraseType) {
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
    
    const Path & InFileStream::path() const noexcept {
        return _path;
    }
    
    InFileStream::InFileStream(const Path & path) : InStreamBase(),
        _path(path) {
            if (_path.count() == 0) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without path.", __FILE__, __LINE__);
                exception.setReason("The path is empty.", nullptr);
                throw exception;
            }
    }
    
    InFileStream::InFileStream(Path && path) : InStreamBase(),
        _path(static_cast<Path &&>(path)) {
            if (_path.count() == 0) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without path.", __FILE__, __LINE__);
                exception.setReason("The path is empty.", nullptr);
                throw exception;
            }
    }
    
    InFileStream::~InFileStream() noexcept {
        if (_file) {
            fclose(_file);
        }
    }
    
    /// InMemStream
    STDMETHODIMP InMemStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            const UInt64 available = _size - _offset;
            size_t sizeToRead = 0;
            if (available > 0) {
                sizeToRead = (size <= available) ? size : static_cast<size_t>(available);
                uint8_t * m = static_cast<uint8_t *>(_memory) + _offset;
                memcpy(data, m, sizeToRead);
                _offset += sizeToRead;
            }
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, sizeToRead)
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
        return S_FALSE;
    }
    
    STDMETHODIMP InMemStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
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
            _offset = 0;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0);
        return S_FALSE;
    }
    
    bool InMemStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }
    
    void InMemStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _offset = 0;
        _opened = true;
    }
    
    void InMemStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _offset = 0;
        _opened = false;
    }
    
    bool InMemStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return false;
        }
        if (_memory && _size > 0) {
            switch (eraseType) {
                case plzma_erase_zero:
                    memset(_memory, 0, static_cast<size_t>(_size));
                    break;
                default:
                    break;
            }
        }
        return true;
    }
        
    InMemStream::InMemStream(const void * memory, const size_t size) : InStreamBase() {
        if (memory && size > 0) {
            void * m = plzma_malloc(size);
            if (m) {
                _memory = m;
                _size = static_cast<UInt64>(size);
                memcpy(m, memory, size);
            } else {
                throw Exception(plzma_error_code_not_enough_memory, "Can't allocate memory for in-stream.", __FILE__, __LINE__);
            }
        } else {
            Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without memory.", __FILE__, __LINE__);
            exception.setReason((size == 0) ? "The memory size is zero." : "The memory is null.", nullptr);
            throw exception;
        }
    }
    
    InMemStream::InMemStream(void * memory, const size_t size, plzma_free_callback freeCallback) : InStreamBase() {
        if (memory && freeCallback && size > 0) {
            _memory = memory;
            _freeCallback = freeCallback;
            _size = static_cast<UInt64>(size);
        } else {
            Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without user-provided memory.", __FILE__, __LINE__);
            if (size == 0) {
                exception.setReason("The memory size is zero.", nullptr);
            } else if (!memory) {
                exception.setReason("The memory is null.", nullptr);
            } else {
                exception.setReason("The user-provided free callback is null.", nullptr);
            }
            throw exception;
        }
    }
    
    InMemStream::~InMemStream() noexcept {
        if (_freeCallback) {
            _freeCallback(_memory);
        } else {
            plzma_free(_memory);
        }
    }
    
    /// InMemStream
    
    STDMETHODIMP InCallbackStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        if (_opened) {
            UInt64 newPos = 0;
            if (_seekCallback(_context.context, offset, seekOrigin, &newPos)) {
                LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, newPos)
                return S_OK;
            }
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0);
        return S_FALSE;
    }

    STDMETHODIMP InCallbackStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            UInt32 procSize = 0;
            if (_readCallback(_context.context, data, size, &procSize)) {
                LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, procSize)
                return S_OK;
            }
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
        return S_FALSE;
    }
    
    void InCallbackStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (!_opened && !(_opened = _openCallback(_context.context)) ) {
            throw Exception(plzma_error_code_io, "Can't open in-stream using open callback.", __FILE__, __LINE__);
        }
    }
    
    void InCallbackStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            _opened = false;
            _closeCallback(_context.context);
        }
    }
    
    bool InCallbackStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }

    bool InCallbackStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        // no erase functionality for a stream with user-defined callbacks.
        return !_opened; // opened -> false
    }
    
    InCallbackStream::InCallbackStream(plzma_in_stream_open_callback openCallback,
                                       plzma_in_stream_close_callback closeCallback,
                                       plzma_in_stream_seek_callback seekCallback,
                                       plzma_in_stream_read_callback readCallback,
                                       const plzma_context context) : InStreamBase(),
        _context(context),
        _openCallback(openCallback),
        _closeCallback(closeCallback),
        _seekCallback(seekCallback),
        _readCallback(readCallback) {
            if (!_openCallback || !_closeCallback || !_seekCallback || !_readCallback) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without required callback.", __FILE__, __LINE__);
                if (!_openCallback) { exception.setReason("The open callback is null.", nullptr); }
                else if (!_closeCallback) { exception.setReason("The close callback is null.", nullptr); }
                else if (!_seekCallback) { exception.setReason("The seek callback is null.", nullptr); }
                else if (!_readCallback) { exception.setReason("The read callback is null.", nullptr); }
                throw exception;
            }
    }
    
    InCallbackStream::~InCallbackStream() noexcept {
        if (_opened) {
            _closeCallback(_context.context);
        }
        if (_context.context && _context.deinitializer) {
            _context.deinitializer(_context.context);
        }
    }

    /// InMultiStream

    STDMETHODIMP InMultiStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
        return _opened ? _stream.Seek(offset, seekOrigin, newPosition) : S_FALSE;
    }

    STDMETHODIMP InMultiStream::Read(void * data, UInt32 size, UInt32 * processedSize) {
        return _opened ? _stream.Read(data, size, processedSize) : S_FALSE;
    }

    void InMultiStream::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return;
        }
        for (plzma_size_t i = 0, n = _streams.count(); i < n; i++) {
            auto stream = _streams.at(i);
            stream->open();
            UInt64 size = 0;
            HRESULT res = stream->Seek(0, STREAM_SEEK_END, &size);
            if (res == S_OK) {
                res = stream->Seek(0, STREAM_SEEK_SET, nullptr);
            }
            if (res != S_OK) {
                Exception exception(plzma_error_code_invalid_arguments, "Can't open in-stream.", __FILE__, __LINE__);
                char reason[64] = { 0 };
                snprintf(reason, 64, "Can't seek sub-stream at index %llu.", static_cast<unsigned long long>(i));
                exception.setReason(reason, nullptr);
                throw exception;
            }
            CMultiStream::CSubStreamInfo info;
            info.Size = size;
            info.Stream = CMyComPtr<IInStream>(stream.get());
            _stream.Streams.Add(info);
            _stream.Init();
        }
        _opened = true;
    }

    void InMultiStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        for (plzma_size_t i = 0, n = _streams.count(); i < n; i++) {
            _streams.at(i)->close();
        }
        _stream.Streams.ClearAndFree();
        _opened = false;
    }
    
    bool InMultiStream::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }

    bool InMultiStream::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return false;
        }
        for (plzma_size_t i = 0, n = _streams.count(); i < n; i++) {
            if (!_streams.at(i)->erase(eraseType)) {
                return false;
            }
        }
        return true;
    }
    
    InMultiStream::InMultiStream(InStreamArray && streams) {
        auto inStreams = static_cast<InStreamArray &&>(streams);
        if (inStreams.count() == 0) {
            Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream without sub-streams.", __FILE__, __LINE__);
            exception.setReason("Sub-streams size is zero.", nullptr);
            throw exception;
        }
        for (plzma_size_t i = 0, n = inStreams.count(); i < n; i++) {
            const auto stream = inStreams.at(i);
            if (stream) {
                _streams.push(stream.cast<InStreamBase>());
            } else {
                _streams.clear();
                Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate in-stream.", __FILE__, __LINE__);
                char reason[64] = { 0 };
                snprintf(reason, 64, "Sub-stream at index %llu is null.", static_cast<unsigned long long>(i));
                exception.setReason(reason, nullptr);
                throw exception;
            }
        }
    }

    InMultiStream::~InMultiStream() noexcept {
    
    }
    
    SharedPtr<InStream> makeSharedInStream(const Path & path) {
        return SharedPtr<InStream>(new InFileStream(path));
    }
    
    SharedPtr<InStream> makeSharedInStream(Path && path) {
        return SharedPtr<InStream>(new InFileStream(static_cast<Path &&>(path)));
    }
    
    SharedPtr<InStream> makeSharedInStream(const void * LIBPLZMA_NONNULL memory, const size_t size) {
        return SharedPtr<InStream>(new InMemStream(memory, size));
    }
    
    SharedPtr<InStream> makeSharedInStream(void * LIBPLZMA_NONNULL memory, const size_t size, plzma_free_callback LIBPLZMA_NONNULL freeCallback) {
        return SharedPtr<InStream>(new InMemStream(memory, size, freeCallback));
    }
    
    SharedPtr<InStream> makeSharedInStream(plzma_in_stream_open_callback LIBPLZMA_NONNULL openCallback,
                                           plzma_in_stream_close_callback LIBPLZMA_NONNULL closeCallback,
                                           plzma_in_stream_seek_callback LIBPLZMA_NONNULL seekCallback,
                                           plzma_in_stream_read_callback LIBPLZMA_NONNULL readCallback,
                                           const plzma_context context) {
        return SharedPtr<InStream>(new InCallbackStream(openCallback, closeCallback, seekCallback, readCallback, context));
    }

    SharedPtr<InStream> makeSharedInStream(InStreamArray && streams) {
        return SharedPtr<InStream>(new InMultiStream(static_cast<InStreamArray &&>(streams)));
    }

} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_in_stream plzma_in_stream_create_with_path(const plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_in_stream, path)
    auto stream = makeSharedInStream(*static_cast<const Path *>(path->object));
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_in_stream plzma_in_stream_create_with_pathm(plzma_path * LIBPLZMA_NONNULL path) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_in_stream, path)
    auto stream = makeSharedInStream(static_cast<Path &&>(*static_cast<Path *>(path->object)));
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_in_stream plzma_in_stream_create_with_memory_copy(const void * LIBPLZMA_NONNULL memory,
                                                        const size_t size) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_in_stream)
    auto stream = makeSharedInStream(memory, size);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_in_stream plzma_in_stream_create_with_memory(void * LIBPLZMA_NONNULL memory,
                                                   const size_t size,
                                                   plzma_free_callback LIBPLZMA_NONNULL free_callback) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_in_stream)
    auto stream = makeSharedInStream(memory, size, free_callback);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_in_stream plzma_in_stream_create_with_callbacks(plzma_in_stream_open_callback LIBPLZMA_NONNULL open_callback,
                                                      plzma_in_stream_close_callback LIBPLZMA_NONNULL close_callback,
                                                      plzma_in_stream_seek_callback LIBPLZMA_NONNULL seek_callback,
                                                      plzma_in_stream_read_callback LIBPLZMA_NONNULL read_callback,
                                                      const plzma_context context) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_in_stream)
    auto stream = makeSharedInStream(open_callback, close_callback, seek_callback, read_callback, context);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_in_stream plzma_in_stream_create_with_stream_arraym(plzma_in_stream_array * LIBPLZMA_NONNULL stream_array) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_in_stream, stream_array)
    auto stream = makeSharedInStream(static_cast<InStreamArray &&>(*static_cast<InStreamArray *>(stream_array->object)));
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

bool plzma_in_stream_opened(plzma_in_stream * LIBPLZMA_NONNULL stream) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, false)
    return static_cast<InStream *>(stream->object)->opened();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(stream, false)
}

bool plzma_in_stream_erase(plzma_in_stream * LIBPLZMA_NONNULL stream, const plzma_erase erase_type) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, false)
    return static_cast<InStream *>(stream->object)->erase(erase_type);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(stream, false)
}

void plzma_in_stream_release(plzma_in_stream * LIBPLZMA_NONNULL stream) {
    plzma_object_exception_release(stream);
    SharedPtr<InStream> streamSPtr;
    streamSPtr.assign(static_cast<InStream *>(stream->object));
    stream->object = nullptr;
}

plzma_in_stream_array plzma_in_stream_array_create_with_capacity(const plzma_size_t capacity) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_in_stream_array)
    createdCObject.object = static_cast<void *>(new InStreamArray(capacity));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_size_t plzma_in_stream_array_count(const plzma_in_stream_array * LIBPLZMA_NONNULL array) {
    return array->exception ? 0 : static_cast<const InStreamArray *>(array->object)->count();
}

void plzma_in_stream_array_add(plzma_in_stream_array * LIBPLZMA_NONNULL array,
                               const plzma_in_stream * LIBPLZMA_NONNULL stream) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(array)
    SharedPtr<InStream> streamSPtr(static_cast<InStream *>(stream->object));
    static_cast<InStreamArray *>(array->object)->push(static_cast<SharedPtr<InStream> &&>(streamSPtr));
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(array)
}

void plzma_in_stream_array_release(plzma_in_stream_array * LIBPLZMA_NONNULL array) {
    plzma_object_exception_release(array);
    delete static_cast<InStreamArray *>(array->object);
    array->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
