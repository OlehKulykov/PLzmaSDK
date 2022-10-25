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
#include <cwchar>

#include "plzma_out_streams.hpp"
#include "plzma_common.hpp"
#include "plzma_file_utils.hpp"
#include "plzma_c_bindings_private.hpp"

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
                }
                catch (const Exception & exception) {
                    _size = _offset = 0;
                    _opened = false;
                    delete _exception;
                    _exception = exception.moveToHeapCopy();
                    return E_OUTOFMEMORY;
                }
#if defined(LIBPLZMA_HAVE_STD)
                catch (const std::exception & exception) {
                    _size = _offset = 0;
                    _opened = false;
                    delete _exception;
                    _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
                    return E_OUTOFMEMORY;
                }
#endif
                catch (...) {
                    _size = _offset = 0;
                    _opened = false;
                    delete _exception;
                    _exception = Exception::create(plzma_error_code_not_enough_memory, "Can't write to out memory stream.", __FILE__, __LINE__);
                    return E_OUTOFMEMORY;
                }
                _size = dstSize;
            }
            memcpy(static_cast<uint8_t *>(_memory) + _offset, data, static_cast<size_t>(size));
            _offset += size;
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, size)
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
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
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0)
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
            } catch (const Exception & exception) {
                _size = _offset = 0;
                _opened = false;
                delete _exception;
                _exception = exception.moveToHeapCopy();
                return E_OUTOFMEMORY;
            }
#if defined(LIBPLZMA_HAVE_STD)
            catch (const std::exception & exception) {
                _size = _offset = 0;
                _opened = false;
                delete _exception;
                _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
                return E_OUTOFMEMORY;
            }
#endif
            catch (...) {
                _size = _offset = 0;
                _opened = false;
                delete _exception;
                _exception = Exception::create(plzma_error_code_not_enough_memory, "Can't set out memory stream size.", __FILE__, __LINE__);
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
            _offset = 0;
            _opened = true;
        }
    }
    
    void OutMemStream::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _offset = 0;
        _opened = false;
    }
    
    Exception * OutMemStream::takeException() noexcept {
        Exception * exception = _exception;
        _exception = nullptr;
        return exception;
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
    
    OutMemStream::~OutMemStream() noexcept {
        delete _exception;
        _exception = nullptr;
    }
    
    /// OutTestStream
    STDMETHODIMP OutTestStream::Write(const void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, size)
            return S_OK;
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
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

    /// OutMultiStreamBase
    STDMETHODIMP OutMultiStreamBase::Write(const void * data, UInt32 size, UInt32 * processedSize) {
        if (_opened) {
            try {
                if (size == 0) {
                    return S_OK;
                }
                const uint64_t dstSize = _offset + static_cast<uint64_t>(size);
                if (_size < dstSize) {
                    RINOK(SetSize(dstSize))
                }
                const uint8_t * writableData = static_cast<const uint8_t *>(data);
                const uint64_t partSize = _partSize;
                uint64_t written = 0, index, inSize = size;
                do {
                    index = _offset / partSize;
                    if (index >= _parts.count()) {
                        return E_FAIL;
                    }
                    auto stream = _parts.at(static_cast<plzma_size_t>(index));
                    if (!stream->opened()) {
                        stream->open();
                    }
                    uint64_t startOffset = partSize * index;
                    uint64_t offset = _offset - startOffset;
                    RINOK(stream->Seek(offset, STREAM_SEEK_SET, nullptr))
                    uint64_t maxWritableSize = partSize - offset;
                    uint64_t writeSize = (inSize > maxWritableSize) ? maxWritableSize : inSize;
                    UInt32 streamProcessedSize = 0;
                    RINOK(stream->Write(writableData, static_cast<UInt32>(writeSize), &streamProcessedSize))
                    if (streamProcessedSize != writeSize) {
                        return E_FAIL;
                    }
                    writableData += writeSize;
                    written += writeSize;
                    _offset += writeSize;
                    inSize -= writeSize;
                } while (inSize != 0);
                
                LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, written)
                return S_OK;
            } catch (const Exception & exception) {
                clear();
                delete _exception;
                _exception = exception.moveToHeapCopy();
                return E_FAIL;
            }
#if defined(LIBPLZMA_HAVE_STD)
            catch (const std::exception & exception) {
                clear();
                delete _exception;
                _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
                return E_FAIL;
            }
#endif
            catch (...) {
                clear();
                delete _exception;
                _exception = Exception::create(plzma_error_code_internal, "Can't write to out multi stream.", __FILE__, __LINE__);
                return E_FAIL;
            }
        }
        LIBPLZMA_CAST_VALUE_TO_PTR(processedSize, UInt32, 0)
        return S_FALSE;
    }

    STDMETHODIMP OutMultiStreamBase::Seek(Int64 offset, UInt32 seekOrigin, UInt64 * newPosition) {
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
        LIBPLZMA_CAST_VALUE_TO_PTR(newPosition, UInt64, 0)
        return S_FALSE;
    }

    STDMETHODIMP OutMultiStreamBase::SetSize(UInt64 newSize) {
        if (_opened) {
            try {
                uint64_t partsCount = newSize / _partSize;
                if ((partsCount * _partSize) < newSize) {
                    partsCount++;
                }
                checkPartsCount(partsCount);
                plzma_size_t count = _parts.count();
                while (partsCount < count && count > 0) {
                    auto stream = _parts.at(count - 1);
                    stream->close();
                    stream->erase();
                    _parts.pop();
                    count = _parts.count();
                }
                count = _parts.count();
                while (partsCount > count) {
                    addPart();
                    count = _parts.count();
                }
            } catch (const Exception & exception) {
                clear();
                delete _exception;
                _exception = exception.moveToHeapCopy();
                return E_FAIL;
            }
#if defined(LIBPLZMA_HAVE_STD)
            catch (const std::exception & exception) {
                clear();
                delete _exception;
                _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
                return E_FAIL;
            }
#endif
            catch (...) {
                clear();
                delete _exception;
                _exception = Exception::create(plzma_error_code_internal, "Can't set size of the out multi stream.", __FILE__, __LINE__);
                return E_FAIL;
            }
            _size = newSize;
            return S_OK;
        }
        return S_FALSE;
    }

    void OutMultiStreamBase::checkPartsCount(const uint64_t partsCount) const {
        if (partsCount > PLZMA_SIZE_T_MAX) {
            Exception exception(plzma_error_code_internal, "The number of sub-streams is out of range.", __FILE__, __LINE__);
            char reason[128];
            snprintf(reason, 128, "The number: %llu greater than: %llu.", static_cast<unsigned long long>(partsCount), static_cast<unsigned long long>(PLZMA_SIZE_T_MAX));
            exception.setReason(reason, nullptr);
            throw exception;
        }
    }

    void OutMultiStreamBase::checkPartSize(const plzma_size_t partSize) {
        if (partSize == 0) {
            Exception exception(plzma_error_code_invalid_arguments, "Can't use provided part size.", __FILE__, __LINE__);
            exception.setReason("The part size is zero.", nullptr);
            throw exception;
        }
    }

    void OutMultiStreamBase::clear() {
        for (plzma_size_t i = 0, n = _parts.count(); i < n; i++) {
            _parts.at(i)->close();
            _parts.at(i)->erase();
        }
        _parts.clear();
        _size = _offset = 0;
        _opened = false;
    }

    void OutMultiStreamBase::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (!_opened) {
            _offset = 0;
            _opened = true;
        }
    }

    void OutMultiStreamBase::close() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            for (plzma_size_t i = 0, n = _parts.count(); i < n; i++) {
                _parts.at(i)->close();
            }
            _offset = 0;
            _opened = false;
        }
    }

    Exception * OutMultiStreamBase::takeException() noexcept {
        Exception * exception = _exception;
        _exception = nullptr;
        return exception;
    }

    bool OutMultiStreamBase::opened() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _opened;
    }

    bool OutMultiStreamBase::erase(const plzma_erase eraseType) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return false;
        }
        for (plzma_size_t i = 0, n = _parts.count(); i < n; i++) {
            if (!_parts.at(i)->erase(eraseType)) {
                return false;
            }
        }
        _size = _offset = 0;
        _parts.clear();
        return true;
    }

    RawHeapMemorySize OutMultiStreamBase::copyContent() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        RawHeapMemorySize combinedContent = RawHeapMemorySize(RawHeapMemory(), 0);
        if (_opened || _size == 0) {
            return combinedContent;
        }
        if (_size > plzma_max_size()) {
            Exception exception(plzma_error_code_not_enough_memory, "The content size is greater than supported by the current platform.", __FILE__, __LINE__);
            char reason[128];
            snprintf(reason, 128, "The content size: %llu, maximum supported: %llu.", static_cast<unsigned long long>(_size), static_cast<unsigned long long>(plzma_max_size()));
            exception.setReason(reason, nullptr);
            throw exception;
        }
        combinedContent.first.resize(static_cast<size_t>(_size));
        for (plzma_size_t i = 0, n = _parts.count(); i < n; i++) {
            RawHeapMemorySize content = _parts.at(i)->copyContent();
            if (content.second == 0) {
                continue;
            }
            const uint64_t size = combinedContent.second + content.second;
            if (size > _size) {
                throw Exception(plzma_error_code_internal, "The size of the content is greater than calculated.", __FILE__, __LINE__);
            }
            memcpy(static_cast<void *>(static_cast<uint8_t *>(combinedContent.first) + combinedContent.second),
                   static_cast<const void *>(content.first),
                   content.second);
            combinedContent.second = static_cast<size_t>(size); // size < _size && _size < plzma_max_size
        }
        return combinedContent;
    }

    OutStreamArray OutMultiStreamBase::streams() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_opened) {
            return OutStreamArray();
        }
        const plzma_size_t count = _parts.count();
        OutStreamArray outStreams(count);
        for (plzma_size_t i = 0; i < count; i++) {
            outStreams.push(_parts.at(i).cast<OutStream>());
        }
        return outStreams;
    }

    OutMultiStreamBase::OutMultiStreamBase(const plzma_size_t partSize) : OutStreamBase() {
        checkPartSize(partSize);
        _partSize = partSize;
    }

    OutMultiStreamBase::~OutMultiStreamBase() noexcept {
        delete _exception;
        _exception = nullptr;
    }
    
    /// OutMultiFileStream
    SharedPtr<OutStreamBase> OutMultiFileStream::addPart() {
        auto path = _dirPath;
#if defined(LIBPLZMA_MSC)
        const wchar_t * name = _partName.wide(), * ext = _partExtension.wide();
        const size_t nameLen = wcslen(name), extLen = wcslen(ext), len = nameLen + extLen + 8;
        RawHeapMemory buff(sizeof(wchar_t) * len);
        int written = swprintf(static_cast<wchar_t *>(buff), len, L"%ls.%ls.%03lu", name, ext, static_cast<unsigned long>(_parts.count() + 1));
        if (written > 0 && static_cast<size_t>(written) < len) {
            static_cast<wchar_t *>(buff)[written] = 0;
        } else {
            throw Exception(plzma_error_code_internal, "Can't generate sub-stream path.", __FILE__, __LINE__);
        }
        path.append(static_cast<wchar_t *>(buff));
#else
        const char * name = _partName.utf8(), * ext = _partExtension.utf8();
        const size_t nameLen = strlen(name), extLen = strlen(ext), len = nameLen + extLen + 8;
        RawHeapMemory buff(sizeof(char) * len);
        int written = snprintf(static_cast<char *>(buff), len, "%s.%s.%03lu", name, ext, static_cast<unsigned long>(_parts.count() + 1));
        if (written > 0 && static_cast<size_t>(written) < len) {
            static_cast<char *>(buff)[written] = 0;
        } else {
            throw Exception(plzma_error_code_internal, "Can't generate sub-stream path.", __FILE__, __LINE__);
        }
        path.append(static_cast<char *>(buff));
#endif
        const SharedPtr<OutStreamBase> stream(new OutFileStream(static_cast<Path &&>(path)));
        _parts.push(stream);
        return stream;
    }

    void OutMultiFileStream::checkPartsCount(const uint64_t partsCount) const {
        OutMultiStreamBase::checkPartsCount(partsCount);
        switch (_format) {
            case plzma_plzma_multi_stream_part_name_format_name_ext_00x:
                if (partsCount > 999) { // ext.999
                    throw Exception(plzma_error_code_invalid_arguments, "Can't instantiate more than 999 file sub-streams with provided 00x format.", __FILE__, __LINE__);
                }
                break;
            default:
                break;
        }
    }

    void OutMultiFileStream::preparePath(const Path & path) {
        if (path.count() == 0) {
            throw Exception(plzma_error_code_invalid_arguments, "Can't instantiate out-stream with empty directory path.", __FILE__, __LINE__);
        }
        bool isDir = false;
        if (!path.exists(&isDir)) {
            if (path.createDir(true)) {
                return;
            }
            Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
            exception.setWhat("Can't create directory at path: ", path.utf8(), nullptr);
            throw exception;
        }
        if (isDir && path.writable()) {
            return;
        }
        Exception exception(plzma_error_code_invalid_arguments, "Can't instantiate out-stream with provided directory path.", __FILE__, __LINE__);
        if (!isDir) {
            exception.setReason("Provided path is not a directory. Path: ", path.utf8(), nullptr);
        } else {
            exception.setReason("Provided path has no write permissions. Path: ", path.utf8(), nullptr);
        }
        throw exception;
    }

    bool OutMultiFileStream::erase(const plzma_erase eraseType) {
        return OutMultiStreamBase::erase(eraseType) ? _dirPath.remove() : false;
    }

    OutMultiFileStream::OutMultiFileStream(const Path & dirPath,
                                           const String & partName,
                                           const String & partExtension,
                                           const plzma_plzma_multi_stream_part_name_format format,
                                           const plzma_size_t partSize) : OutMultiStreamBase(partSize) {
        preparePath(dirPath);
        _dirPath = dirPath;
        _partName = partName;
        _partExtension = partExtension;
        _format = format;
    }

    OutMultiFileStream::OutMultiFileStream(Path && dirPath,
                                           String && partName,
                                           String && partExtension,
                                           const plzma_plzma_multi_stream_part_name_format format,
                                           const plzma_size_t partSize) : OutMultiStreamBase(partSize) {
        preparePath(dirPath);
        _dirPath = static_cast<Path &&>(dirPath);
        _partName = static_cast<String &&>(partName);
        _partExtension = static_cast<String &&>(partExtension);
        _format = format;
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

    SharedPtr<OutMultiStream> makeSharedOutMultiStream(const Path & dirPath,
                                                       const String & partName,
                                                       const String & partExtension,
                                                       const plzma_plzma_multi_stream_part_name_format format,
                                                       const plzma_size_t partSize) {
        return SharedPtr<OutMultiStream>(new OutMultiFileStream(dirPath, partName, partExtension, format, partSize));
    }

    SharedPtr<OutMultiStream> makeSharedOutMultiStream(Path && dirPath,
                                                       String && partName,
                                                       String && partExtension,
                                                       const plzma_plzma_multi_stream_part_name_format format,
                                                       const plzma_size_t partSize) {
        return SharedPtr<OutMultiStream>(new OutMultiFileStream(static_cast<Path &&>(dirPath),
                                                                static_cast<String &&>(partName),
                                                                static_cast<String &&>(partExtension),
                                                                format,
                                                                partSize));
    }

    SharedPtr<OutMultiStream> makeSharedOutMultiStream(const plzma_size_t partSize) {
        return SharedPtr<OutMultiStream>(new OutMultiMemStream(partSize));
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
    plzma_memory createdCObject;
    createdCObject.memory = nullptr;
    createdCObject.exception = nullptr;
    createdCObject.size = 0;
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(stream, createdCObject)
    auto content = static_cast<const OutStream *>(stream->object)->copyContent();
    createdCObject.memory = content.first.take();
    createdCObject.size = content.second;
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
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

plzma_out_multi_stream plzma_out_multi_stream_create_with_directory_path_utf8_name_ext_format_part_size(const plzma_path * LIBPLZMA_NONNULL dir_path,
                                                                                                        const char * LIBPLZMA_NONNULL part_name,
                                                                                                        const char * LIBPLZMA_NULLABLE part_extension,
                                                                                                        const plzma_plzma_multi_stream_part_name_format format,
                                                                                                        const plzma_size_t part_size) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_multi_stream, dir_path)
    auto dirPath = static_cast<const Path &>(*static_cast<const Path *>(dir_path->object));
    auto stream = makeSharedOutMultiStream(static_cast<Path &&>(dirPath),
                                           String(part_name),
                                           String(part_extension),
                                           format,
                                           part_size);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_out_multi_stream plzma_out_multi_stream_create_with_directory_path_wide_name_ext_format_part_size(const plzma_path * LIBPLZMA_NONNULL dir_path,
                                                                                                        const wchar_t * LIBPLZMA_NONNULL part_name,
                                                                                                        const wchar_t * LIBPLZMA_NULLABLE part_extension,
                                                                                                        const plzma_plzma_multi_stream_part_name_format format,
                                                                                                        const plzma_size_t part_size) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_multi_stream, dir_path)
    auto dirPath = static_cast<const Path &>(*static_cast<const Path *>(dir_path->object));
    auto stream = makeSharedOutMultiStream(static_cast<Path &&>(dirPath),
                                           String(part_name),
                                           String(part_extension),
                                           format,
                                           part_size);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_out_multi_stream plzma_out_multi_stream_create_memory_with_part_size(const plzma_size_t part_size) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_TRY(plzma_out_multi_stream)
    auto stream = makeSharedOutMultiStream(part_size);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_out_stream_array plzma_out_multi_stream_streams(const plzma_out_multi_stream * LIBPLZMA_NONNULL stream) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_stream_array, stream)
    OutStreamArray streams = static_cast<OutMultiStream *>(stream->object)->streams();
    createdCObject.object = static_cast<void *>(new OutStreamArray(static_cast<OutStreamArray &&>(streams)));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_out_multi_stream_release(plzma_out_multi_stream * LIBPLZMA_NONNULL stream) {
    plzma_object_exception_release(stream);
    SharedPtr<OutMultiStream> streamSPtr;
    streamSPtr.assign(static_cast<OutMultiStream *>(stream->object));
    stream->object = nullptr;
}

plzma_size_t plzma_out_stream_array_count(const plzma_out_stream_array * LIBPLZMA_NONNULL array) {
    return array->exception ? 0 : static_cast<OutStreamArray *>(array->object)->count();
}

plzma_out_stream plzma_out_stream_array_at(plzma_out_stream_array * LIBPLZMA_NONNULL array,
                                           const plzma_size_t index) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_out_stream, array)
    auto stream = static_cast<const OutStreamArray *>(array->object)->at(index);
    createdCObject.object = static_cast<void *>(stream.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_out_stream_array_release(plzma_out_stream_array * LIBPLZMA_NONNULL array) {
    plzma_object_exception_release(array);
    delete static_cast<OutStreamArray *>(array->object);
    array->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
