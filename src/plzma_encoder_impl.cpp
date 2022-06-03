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

#include "plzma_encoder_impl.hpp"
#include "plzma_decoder_impl.hpp"
#include "plzma_in_streams.hpp"
#include "plzma_common.hpp"
#include "plzma_open_callback.hpp"
#include "plzma_c_bindings_private.hpp"

#include <stdint.h>
#include <limits.h>

namespace plzma {
    
    void EncoderImpl::retain() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RETAIN_IMPL(__m_RefCount)
#else
        LIBPLZMA_RETAIN_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    void EncoderImpl::release() {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        LIBPLZMA_RELEASE_IMPL(__m_RefCount)
#else
        LIBPLZMA_RELEASE_LOCKED_IMPL(__m_RefCount, _mutex)
#endif
    }
    
    // IProgress
    STDMETHODIMP EncoderImpl::SetTotal(UInt64 size) {
#if defined(LIBPLZMA_NO_PROGRESS)
        return S_OK;
#else
        return setProgressTotal(size);
#endif
    }
    
    STDMETHODIMP EncoderImpl::SetCompleted(const UInt64 *completeValue) {
#if defined(LIBPLZMA_NO_PROGRESS)
        return S_OK;
#else
        return completeValue ? setProgressCompleted(*completeValue) : S_OK;
#endif
    }
    
    // IUpdateCallback2
    STDMETHODIMP EncoderImpl::GetUpdateItemInfo(UInt32 index, Int32 * newData, Int32 * newProperties, UInt32 * indexInArchive) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result != S_OK) {
                return _result;
            }
            LIBPLZMA_SET_VALUE_TO_PTR(newData, BoolToInt(true))
            LIBPLZMA_SET_VALUE_TO_PTR(newProperties, BoolToInt(true))
            if (indexInArchive) {
                *indexInArchive = (UInt32)(Int32)-1;
            }
            return (_result = setupSource(index));
        } catch (const Exception & exception) {
            _exception = exception.moveToHeapCopy();
            return E_FAIL;
        }
#if defined(LIBPLZMA_HAVE_STD)
        catch (const std::exception & exception) {
            _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
            return E_FAIL;
        }
#endif
        catch (...) {
            _exception = Exception::create(plzma_error_code_not_enough_memory, "Can't setup next item source.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP EncoderImpl::GetProperty(UInt32 index, PROPID propID, PROPVARIANT * value) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result != S_OK) {
                return _result;
            } else if ( (_result = setupSource(index)) != S_OK) {
                return _result;
            }
            NWindows::NCOM::CPropVariant prop;
            switch (propID) {
                case kpidIsAnti: prop = false; break;
                case kpidPath: prop = _source.archivePath.wide(); break;
                case kpidIsDir: prop = false; break;
                case kpidSize: prop = _source.stat.size; break;
                //case kpidAttrib: prop = dirItem.Attrib; break; // 9
                case kpidCTime: prop = UnixTimeToFILETIME(_source.stat.creation); break;
                case kpidATime: prop = UnixTimeToFILETIME(_source.stat.last_access); break;
                case kpidMTime: prop = UnixTimeToFILETIME(_source.stat.last_modification); break;
                
                // Tar
                case kpidSymLink:
                case kpidHardLink:
                    if (_type == plzma_file_type_tar) {
                        prop = L"";
                    }
                    break;
                case kpidPosixAttrib:   // 53, tar
                case kpidUser:          // 25, tar
                case kpidGroup:         // 26, tar
                    break;
                default:
                    break;
            }
            prop.Detach(value);
            return S_OK;
        } catch (const Exception & exception) {
            _exception = exception.moveToHeapCopy();
            return E_FAIL;
        }
#if defined(LIBPLZMA_HAVE_STD)
        catch (const std::exception & exception) {
            _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
            return E_FAIL;
        }
#endif
        catch (...) {
            _exception = Exception::create(plzma_error_code_not_enough_memory, "Can't provide item property.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP EncoderImpl::GetStream(UInt32 index, ISequentialInStream ** inStream) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result != S_OK) {
                return _result;
            } else if ( (_result = setupSource(index)) != S_OK) {
                return _result;
            }
            
            if (!_source.stream) {
                _source.stream = SharedPtr<InStreamBase>(new InFileStream(static_cast<Path &&>(_source.path)));
            }
            
            InStreamBase * baseStream = _source.stream.get();
            baseStream->AddRef(); // +1
            *inStream = baseStream;
            baseStream->open();
            
#if !defined(LIBPLZMA_NO_PROGRESS)
            _progress->setPath(_source.archivePath);
#endif
            
            return S_OK;
        } catch (const Exception & exception) {
            _exception = exception.moveToHeapCopy();
            return E_FAIL;
        }
#if defined(LIBPLZMA_HAVE_STD)
        catch (const std::exception & exception) {
            _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
            return E_FAIL;
        }
#endif
        catch (...) {
            _exception = Exception::create(plzma_error_code_not_enough_memory, "Can't create input file stream.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP EncoderImpl::SetOperationResult(Int32 operationResult) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result == S_OK) {
                _source.close();
                switch (operationResult) {
                    case NArchive::NUpdate::NOperationResult::kOK:
                        return S_OK;
                    default:
                        _exception = Exception::create(plzma_error_code_internal, "Item compressed with error.", __FILE__, __LINE__);
                        _result = E_FAIL;
                        break;
                }
            }
            return _result;
        } catch (const Exception & exception) {
            _exception = exception.moveToHeapCopy();
            return E_FAIL;
        }
#if defined(LIBPLZMA_HAVE_STD)
        catch (const std::exception & exception) {
            _exception = Exception::create(plzma_error_code_internal, exception.what(), __FILE__, __LINE__);
            return E_FAIL;
        }
#endif
        catch (...) {
            _exception = Exception::create(plzma_error_code_unknown, "Can't process compress operation result.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP EncoderImpl::GetVolumeSize(UInt32 index, UInt64 * size) {
        return S_OK; // unused
    }
    
    STDMETHODIMP EncoderImpl::GetVolumeStream(UInt32 index, ISequentialOutStream ** volumeStream) {
        return S_OK; // unused
    }
    
    // ICryptoGetTextPassword
    STDMETHODIMP EncoderImpl::CryptoGetTextPassword(BSTR * password) {
        if (hasOption(OptionRequirePassword)) {
            return getTextPassword(nullptr, password);
        }
        return S_OK;
    }
    
    // ICryptoGetTextPassword2
    STDMETHODIMP EncoderImpl::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) {
        if (hasOption(OptionRequirePassword)) {
            return getTextPassword(passwordIsDefined, password);
        }
        LIBPLZMA_SET_VALUE_TO_PTR(passwordIsDefined, BoolToInt(false))
        return S_OK;
    }
    
    void EncoderImpl::setPassword(const wchar_t * password) {
#if defined(LIBPLZMA_NO_CRYPTO)
        throw Exception(plzma_error_code_invalid_arguments, LIBPLZMA_NO_CRYPTO_EXCEPTION_WHAT, __FILE__, __LINE__);
#else
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _password.clear(plzma_erase_zero);
        _password.set(password);
#endif
    }
    
    void EncoderImpl::setPassword(const char * password) {
#if defined(LIBPLZMA_NO_CRYPTO)
        throw Exception(plzma_error_code_invalid_arguments, LIBPLZMA_NO_CRYPTO_EXCEPTION_WHAT, __FILE__, __LINE__);
#else
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _password.clear(plzma_erase_zero);
        _password.set(password);
#endif
    }
    
    void EncoderImpl::setProgressDelegate(ProgressDelegate * delegate) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setDelegate(delegate);
#endif
    }
    
    void EncoderImpl::add(const Path & path, const plzma_open_dir_mode_t openDirMode, const Path & archivePath) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_archive || _opening || _result == E_ABORT) {
            return;
        }
        if (path.count() == 0) {
            throw Exception(plzma_error_code_invalid_arguments, "Can't add empty path.", __FILE__, __LINE__);
        }
        bool isDir = false;
        if (path.exists(&isDir)) {
            for (plzma_size_t i = 0, n = _paths.count(); i < n; i++) {
                const auto & ap = _paths.at(i);
                if (ap.path == path) {
                    Exception exception(plzma_error_code_invalid_arguments, nullptr, __FILE__, __LINE__);
                    exception.setWhat("Can't add duplicated path: ", path.utf8(), nullptr);
                    throw exception;
                }
            }
            AddedPath addedPath;
            addedPath.path = path;
            addedPath.archivePath = archivePath;
            addedPath.openDirMode = openDirMode;
            addedPath.isDir = isDir;
            _paths.push(static_cast<AddedPath &&>(addedPath));
        } else {
            Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
            exception.setWhat("Can't add path: ", path.utf8(), nullptr);
            exception.setReason("The path doesn't exist or is not readable.", nullptr);
            throw exception;
        }
    }
    
    void EncoderImpl::add(const SharedPtr<InStream> & stream, const Path & archivePath) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_archive || _opening || _result == E_ABORT) {
            return;
        }
        if (archivePath.count() == 0) {
            throw Exception(plzma_error_code_invalid_arguments, "Can't add stream without archive path.", __FILE__, __LINE__);
        }
        if (stream) {
            for (plzma_size_t i = 0, n = _streams.count(); i < n; i++) {
                const auto & as = _streams.at(i);
                if (as.archivePath == archivePath) {
                    Exception exception(plzma_error_code_invalid_arguments, nullptr, __FILE__, __LINE__);
                    exception.setWhat("Can't add duplicated stream with archive path: ", archivePath.utf8(), nullptr);
                    throw exception;
                }
            }
            AddedStream addedStream;
            addedStream.stream = stream.cast<InStreamBase>();
            addedStream.archivePath = archivePath;
            plzma_path_stat stat;
            stat.creation = stat.last_access = stat.last_modification = time(nullptr);
            stat.size = 0;
            addedStream.stat = stat;
            _streams.push(static_cast<AddedStream &&>(addedStream));
        } else {
            throw Exception(plzma_error_code_invalid_arguments, "Can't add empty stream.", __FILE__, __LINE__);
        }
    }
    
    uint64_t EncoderImpl::processAddedPaths() {
        uint64_t itemsCount = 0;
        for (plzma_size_t i = 0, n = _paths.count(); i < n; i++) {
            AddedPath addedPath(static_cast<AddedPath &&>(_paths.at(i))); // move -> no longer needed
            Path rootArchivePath = addedPath.archivePath.count() > 0 ? addedPath.archivePath : static_cast<Path &&>(addedPath.path.lastComponent());
            if (addedPath.isDir) {
                auto it = addedPath.path.openDir(addedPath.openDirMode);
                AddedSubDir subDir;
                while (it->next()) {
                    if (!it->isDir()) { // sub-file -> root + iterator path
                        AddedFile item;
                        item.path = static_cast<Path &&>(it->path());
                        item.archivePath = rootArchivePath;
                        item.archivePath.append(item.path);
                        item.stat = it->fullPath().stat();
                        subDir.files.push(static_cast<AddedFile &&>(item));
                        itemsCount++;
                    }
                }
                if (subDir.files.count() > 0) {
                    subDir.path = static_cast<Path &&>(addedPath.path);
                    _subDirs.push(static_cast<AddedSubDir &&>(subDir));
                }
            } else {
                AddedFile item;
                item.path = static_cast<Path &&>(addedPath.path); // full path, move
                item.archivePath = static_cast<Path &&>(rootArchivePath); // move, no longer needed
                item.stat = item.path.stat();
                _files.push(static_cast<AddedFile &&>(item));
                itemsCount++;
            }
        }
        _paths.clear();
        return itemsCount;
    }
    
    HRESULT EncoderImpl::setupSource(UInt32 index) {
        if (_source.itemIndex == index) {
            return S_OK;
        }
        _source.close();
        _source.itemIndex = index;
        
        for (plzma_size_t i = 0, n = _subDirs.count(); i < n; i++) {
            const auto & subDir = _subDirs.at(i);
            const UInt32 count = subDir.files.count();
            if (index < count) {
                const auto & file = subDir.files.at(index);
                _source.archivePath = file.archivePath;
                Path fullPath = subDir.path;
                fullPath.append(file.path);
                _source.path = static_cast<Path &&>(fullPath);
                _source.stat = file.stat;
                return S_OK;
            }
            index -= count;
        }
        
        UInt32 count = _files.count();
        if (index < count) {
            const auto & file = _files.at(index);
            _source.path = file.path;
            _source.archivePath = file.archivePath;
            _source.stat = file.stat;
            return S_OK;
        }
        index -= count;
        
        count = _streams.count();
        if (index < count) {
            auto & stream = _streams.at(index);
            HRESULT res = S_OK;
            _source.stream = stream.stream;
            _source.archivePath = stream.archivePath;
            if (stream.stat.size == 0) {
                stream.stream->open();
                UInt64 pos = 0;
                res = stream.stream->Seek(0, SZ_SEEK_END, &pos);
                stream.stream->close();
                if (res != S_OK) {
                    return res;
                }
                stream.stat.size = pos;
            }
            _source.stat = stream.stat;
            return S_OK;
        }
        
        return E_FAIL;
    }

    void EncoderImpl::applySettings7z(ISetProperties * properties) {
        using namespace NWindows::NCOM;
        
        static const UInt32 settingsCount = 9;
        static const wchar_t * names[settingsCount] = {
            L"0",   // method
            L"s",   // solid
            L"x",   // compression level
            L"hc",  // compress header
            L"he",  // encrypt header
            L"tc",  // write creation time
            L"ta",  // write access time
            L"tm",  // write modification time
            
            L"hcf"  // compress header full, true - add, false - don't add/ignore
        };
        
#if defined(LIBPLZMA_NO_CRYPTO)
        if (_options & OptionEncryptHeader) {
            Exception exception(plzma_error_code_invalid_arguments, "Can't use 'encrypt header' property.", __FILE__, __LINE__);
            exception.setReason("The crypto functionality disabled.", nullptr);
            throw exception;
        }
        if (_options & OptionEncryptContent) {
            Exception exception(plzma_error_code_invalid_arguments, "Can't use 'encrypt content' property.", __FILE__, __LINE__);
            exception.setReason("The crypto functionality disabled.", nullptr);
            throw exception;
        }
#endif
        
        CPropVariant values[settingsCount] = {
            CPropVariant(static_cast<UInt32>(0)),                           // method dummy value
            
            CPropVariant((_options & OptionSolid) ? true : false),          // solid mode ON
            CPropVariant(static_cast<UInt32>(_compressionLevel)),           // compression level = 9 - ultra
            CPropVariant((_options & OptionCompressHeader) ? true : false), // compress header
            CPropVariant((_options & OptionEncryptHeader) ? true : false),  // encrypt header
            CPropVariant((_options & OptionStoreCTime) ? true : false),     // write creation time
            CPropVariant((_options & OptionStoreATime) ? true : false),     // write access time
            CPropVariant((_options & OptionStoreMTime) ? true : false),     // write modification time
            
            CPropVariant(true)                                              // compress header full, true - add, false - don't add/ignore
        };
        
        switch (_method) {
            case plzma_method_LZMA:     values[0] = L"LZMA";  break;
            case plzma_method_LZMA2:    values[0] = L"LZMA2"; break;
            case plzma_method_PPMd:     values[0] = L"PPMD";  break;
            default: break;
        }
        
        const HRESULT res = properties->SetProperties(names,
                                                      values,
                                                      (_options & OptionCompressHeaderFull) ? settingsCount : (settingsCount - 1));
        if (res != S_OK) {
            throw Exception(plzma_error_code_internal, "Can't apply 7z archive properties.", __FILE__, __LINE__);
        }
    }
    
    void EncoderImpl::applySettingsXz(ISetProperties * properties) {
        using namespace NWindows::NCOM;
        
        static const UInt32 settingsCount = 3;
        static const wchar_t * names[settingsCount] = {
            L"0",   // method
            L"s",   // solid
            L"x"    // compression level
        };
        
        CPropVariant values[settingsCount] = {
            CPropVariant(L"LZMA2"),                                         // method
            CPropVariant((_options & OptionSolid) ? true : false),          // solid mode ON
            CPropVariant(static_cast<UInt32>(_compressionLevel)),           // compression level = 9 - ultra
        };
        
        const HRESULT res = properties->SetProperties(names, values, settingsCount);
        if (res != S_OK) {
            throw Exception(plzma_error_code_internal, "Can't apply xz archive properties.", __FILE__, __LINE__);
        }
    }
    
    void EncoderImpl::applySettingsTar(ISetProperties * properties) {
        using namespace NWindows::NCOM;
        
        static const UInt32 settingsCount = 1;
        static const wchar_t * names[settingsCount] = {
            L"cp"   // code page
        };
        
        CPropVariant values[settingsCount] = {
            CPropVariant(static_cast<UInt32>(CP_UTF8))
        };
        
        const HRESULT res = properties->SetProperties(names, values, settingsCount);
        if (res != S_OK) {
            throw Exception(plzma_error_code_internal, "Can't apply tar archive properties.", __FILE__, __LINE__);
        }
    }
    
    void EncoderImpl::applySettings() {
        ISetProperties * setPropertiesRaw = nullptr;
        const HRESULT res = _archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&setPropertiesRaw));
        CMyComPtr<ISetProperties> setProperties;
        setProperties.Attach(setPropertiesRaw);
        if (res != S_OK || !setPropertiesRaw) {
            throw Exception(plzma_error_code_internal, "Can't initialize archive properties.", __FILE__, __LINE__);
        }
        
        switch (_type) {
            case plzma_file_type_7z:
                applySettings7z(setPropertiesRaw);
                break;
            case plzma_file_type_xz:
                applySettingsXz(setPropertiesRaw);
                break;
            case plzma_file_type_tar:
                applySettingsTar(setPropertiesRaw);
                break;
            default:
                break;
        }
    }
        
    bool EncoderImpl::open() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (_archive || _opening || _compressing || _result == E_ABORT) {
            return false;
        }
        
        CMyComPtr<EncoderImpl> selfPtr(this);
        _opening = true;
        
        uint64_t itemsCount = processAddedPaths();

        itemsCount += _streams.count();
        
        if (_result == E_ABORT || itemsCount == 0) {
            return false;
        }
        
        size_t errorNumber = 0;
        if (itemsCount > 1 && _type == plzma_file_type_xz) {
            errorNumber = 1;
        } else if (itemsCount >= UINT32_MAX) {
            errorNumber = 2;
        }
        if (errorNumber) {
            Exception exception(plzma_error_code_invalid_arguments, nullptr, __FILE__, __LINE__);
            char reason[128];
            snprintf(reason, 128, "The number of items: %llu", static_cast<unsigned long long>(itemsCount));
            exception.setReason(reason, nullptr);
            exception.setWhat(errorNumber == 1 ? "The 'xz' type supports only one item." : "The number of items is greater then supported.", nullptr);
            throw exception;
        }
        
        _itemsCount = static_cast<UInt32>(itemsCount);
        _stream->open();
        _archive = OpenCallback::createArchive<IOutArchive>(_type);
        
        applySettings();
        
        _opening = false;
        return true;
    }
    
    bool EncoderImpl::compress() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        if (!_archive || _opening || _compressing || _result == E_ABORT) {
            return false;
        }
        
        CMyComPtr<EncoderImpl> selfPtr(this);
        _source.itemIndex = 1;
        HRESULT result = setupSource(0);
        if (result != S_OK) {
            return false;
        }
        
        _compressing = true;
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setPartsCount(1);
        _progress->startPart();
#endif
        LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
        result = _archive->UpdateItems(_stream, _itemsCount, this);
        LIBPLZMA_UNIQUE_LOCK_LOCK(lock)
        
        _compressing = false;
        _stream->close();
        _source.close();
        
        if (result != S_OK || _result != S_OK) {
            if (result == E_ABORT || _result == E_ABORT) {
                return false; // aborted -> without exception
            } else if (_exception) {
                Exception localException(static_cast<Exception &&>(*_exception));
                delete _exception;
                _exception = nullptr;
                throw localException;
            }
            Exception * exception = _stream->takeException();
            if (exception) {
                Exception localException(static_cast<Exception &&>(*exception));
                delete exception;
                throw localException;
            }
            throw Exception(plzma_error_code_internal, "Unknown compress error.", __FILE__, __LINE__);
        }
        
        return true;
    }
    
    void EncoderImpl::abort() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _result = E_ABORT;
        if (!_compressing) {
            _stream->close();
            _source.close();
        }
    }
    
    bool EncoderImpl::hasOption(const Option option) const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return (_options & option) ? true : false;
    }
    
    void EncoderImpl::setOption(const Option option, const bool set) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        if (set) {
            _options |= option;
        } else {
            _options &= ~option;
        }
    }
    
    bool EncoderImpl::shouldCreateSolidArchive() const { return hasOption(OptionSolid); }
    void EncoderImpl::setShouldCreateSolidArchive(const bool solid) { setOption(OptionSolid, solid); }
    bool EncoderImpl::shouldCompressHeader() const { return hasOption(OptionCompressHeader); }
    void EncoderImpl::setShouldCompressHeader(const bool compress) { setOption(OptionCompressHeader, compress); }
    bool EncoderImpl::shouldCompressHeaderFull() const { return hasOption(OptionCompressHeaderFull); }
    void EncoderImpl::setShouldCompressHeaderFull(const bool compress) { setOption(OptionCompressHeaderFull, compress); }
    bool EncoderImpl::shouldEncryptContent() const { return hasOption(OptionEncryptContent); }
    void EncoderImpl::setShouldEncryptContent(const bool encrypt) { setOption(OptionEncryptContent, encrypt); }
    bool EncoderImpl::shouldEncryptHeader() const { return hasOption(OptionEncryptHeader); }
    void EncoderImpl::setShouldEncryptHeader(const bool encrypt) { setOption(OptionEncryptHeader, encrypt); }
    bool EncoderImpl::shouldStoreCreationTime() const { return hasOption(OptionStoreCTime); }
    void EncoderImpl::setShouldStoreCreationTime(const bool store) { setOption(OptionStoreCTime, store); }
    bool EncoderImpl::shouldStoreAccessTime() const { return hasOption(OptionStoreATime); }
    void EncoderImpl::setShouldStoreAccessTime(const bool store) { setOption(OptionStoreATime, store); }
    bool EncoderImpl::shouldStoreModificationTime() const { return hasOption(OptionStoreMTime); }
    void EncoderImpl::setShouldStoreModificationTime(const bool store) { setOption(OptionStoreMTime, store); }
    
    uint8_t EncoderImpl::compressionLevel() const {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        return _compressionLevel;
    }
    
    void EncoderImpl::setCompressionLevel(const uint8_t level) {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _compressionLevel = level > 9 ? 9 : level;
    }
    
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    void EncoderImpl::setUtf8Callback(plzma_progress_delegate_utf8_callback LIBPLZMA_NULLABLE callback) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setUtf8Callback(callback);
#endif
    }
    
    void EncoderImpl::setWideCallback(plzma_progress_delegate_wide_callback LIBPLZMA_NULLABLE callback) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setWideCallback(callback);
#endif
    }
#endif
    
    EncoderImpl::EncoderImpl(const CMyComPtr<OutStreamBase> & stream,
                             const plzma_file_type type,
                             const plzma_method method,
                             const plzma_context context) : CMyUnknownImp(),
        _stream(stream),
        _type(type),
        _method(method) {
            plzma::initialize();
#if !defined(LIBPLZMA_NO_PROGRESS)
            _progress = makeShared<Progress>(context);
#endif
            // docs
            _options |= (OptionSolid | OptionCompressHeader | OptionCompressHeaderFull);
            _options |= (OptionStoreCTime | OptionStoreMTime | OptionStoreATime);
    }
    
    EncoderImpl::~EncoderImpl() {
        _stream->close();
        _source.close();
    }
    
    SharedPtr<Encoder> makeSharedEncoder(const SharedPtr<OutStream> & stream,
                                         const plzma_file_type type,
                                         const plzma_method method,
                                         const plzma_context context) {
        auto baseStream = stream.cast<OutStreamBase>();
        if (baseStream) {
            return SharedPtr<Encoder>(new EncoderImpl(CMyComPtr<OutStreamBase>(baseStream.get()), type, method, context));
        }
        throw Exception(plzma_error_code_invalid_arguments, "No output stream.", __FILE__, __LINE__);
    }

    SharedPtr<Encoder> makeSharedEncoder(const SharedPtr<OutMultiStream> & stream,
                                         const plzma_file_type type,
                                         const plzma_method method,
                                         const plzma_context context) {
        if (type != plzma_file_type_7z) {
            throw Exception(plzma_error_code_invalid_arguments, "Currently only 7-zip type archives supports multi streams.", __FILE__, __LINE__);
        }
        auto baseStream = stream.cast<OutStreamBase>();
        if (baseStream) {
            return SharedPtr<Encoder>(new EncoderImpl(CMyComPtr<OutStreamBase>(baseStream.get()), type, method, context));
        }
        throw Exception(plzma_error_code_invalid_arguments, "No output stream.", __FILE__, __LINE__);
    }

} // namespace plzma


#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_encoder plzma_encoder_create(plzma_out_stream * LIBPLZMA_NONNULL stream,
                                   const plzma_file_type type,
                                   const plzma_method method,
                                   const plzma_context context) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_decoder, stream)
    SharedPtr<OutStream> outStream(static_cast<OutStream *>(stream->object));
    auto baseOutStream = outStream.cast<OutStreamBase>();
    if (!baseOutStream) {
        throw Exception(plzma_error_code_invalid_arguments, "No output stream.", __FILE__, __LINE__);
    }
    SharedPtr<EncoderImpl> encoder(new EncoderImpl(CMyComPtr<OutStreamBase>(baseOutStream.get()), type, method, context));
    createdCObject.object = static_cast<void *>(encoder.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_encoder plzma_encoder_create_with_multi_stream(plzma_out_multi_stream * LIBPLZMA_NONNULL stream,
                                                     const plzma_file_type type,
                                                     const plzma_method method,
                                                     const plzma_context context) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_decoder, stream)
    if (type != plzma_file_type_7z) {
        throw Exception(plzma_error_code_invalid_arguments, "Currently only 7-zip type archives supports multi streams.", __FILE__, __LINE__);
    }
    SharedPtr<OutMultiStream> outStream(static_cast<OutMultiStream *>(stream->object));
    auto baseOutStream = outStream.cast<OutStreamBase>();
    if (!baseOutStream) {
        throw Exception(plzma_error_code_invalid_arguments, "No output stream.", __FILE__, __LINE__);
    }
    SharedPtr<EncoderImpl> encoder(new EncoderImpl(CMyComPtr<OutStreamBase>(baseOutStream.get()), type, method, context));
    createdCObject.object = static_cast<void *>(encoder.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

void plzma_encoder_set_progress_delegate_utf8_callback(plzma_encoder * LIBPLZMA_NONNULL encoder,
                                                       plzma_progress_delegate_utf8_callback LIBPLZMA_NULLABLE callback) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setUtf8Callback(callback);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

void plzma_encoder_set_progress_delegate_wide_callback(plzma_encoder * LIBPLZMA_NONNULL encoder,
                                                       plzma_progress_delegate_wide_callback LIBPLZMA_NULLABLE callback) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setWideCallback(callback);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

void plzma_encoder_set_password_wide_string(plzma_encoder * LIBPLZMA_NONNULL encoder, const wchar_t * LIBPLZMA_NULLABLE password) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setPassword(password);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

void plzma_encoder_set_password_utf8_string(plzma_encoder * LIBPLZMA_NONNULL encoder, const char * LIBPLZMA_NULLABLE password) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setPassword(password);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_create_solid_archive(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldCreateSolidArchive();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_create_solid_archive(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool solid) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldCreateSolidArchive(solid);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

uint8_t plzma_encoder_compression_level(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, 0)
    return static_cast<EncoderImpl *>(encoder->object)->compressionLevel();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, 0)
}

void plzma_encoder_set_compression_level(plzma_encoder * LIBPLZMA_NONNULL encoder, const uint8_t level) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setCompressionLevel(level);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_compress_header(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldCompressHeader();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_compress_header(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool compress) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldCompressHeader(compress);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_compress_header_full(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldCompressHeaderFull();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_compress_header_full(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool compress) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldCompressHeaderFull(compress);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_encrypt_content(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldEncryptContent();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_encrypt_content(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool encrypt) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldEncryptContent(encrypt);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_encrypt_header(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldEncryptHeader();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_encrypt_header(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool encrypt) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldEncryptHeader(encrypt);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_store_creation_time(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldStoreCreationTime();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_store_creation_time(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool store) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldStoreCreationTime(store);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_store_access_time(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldStoreAccessTime();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_store_access_time(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool store) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldStoreAccessTime(store);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_should_store_modification_time(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->shouldStoreModificationTime();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_set_should_store_modification_time(plzma_encoder * LIBPLZMA_NONNULL encoder, const bool store) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->setShouldStoreModificationTime(store);
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

void plzma_encoder_add_path(plzma_encoder * LIBPLZMA_NONNULL encoder,
                            const plzma_path * LIBPLZMA_NONNULL path,
                            const plzma_open_dir_mode_t open_dir_mode,
                            const plzma_path * LIBPLZMA_NULLABLE archive_path) {
    if (encoder->exception || path->exception || (archive_path && archive_path->exception)) return;
    try {
        if (archive_path) {
            static_cast<EncoderImpl *>(encoder->object)->add(*static_cast<const Path *>(path->object), open_dir_mode,
                                                             *static_cast<const Path *>(archive_path->object));
        } else {
            static_cast<EncoderImpl *>(encoder->object)->add(*static_cast<const Path *>(path->object), open_dir_mode);
        }
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

void plzma_encoder_add_stream(plzma_encoder * LIBPLZMA_NONNULL encoder,
                              const plzma_in_stream * LIBPLZMA_NONNULL stream,
                              const plzma_path * LIBPLZMA_NONNULL archive_path) {
    if (encoder->exception || stream->exception || archive_path->exception) return;
    try {
        SharedPtr<InStream> streamSPtr(static_cast<InStream *>(stream->object));
        static_cast<EncoderImpl *>(encoder->object)->add(streamSPtr,
                                                         *static_cast<const Path *>(archive_path->object));
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_open(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->open();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_abort(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY(encoder)
    static_cast<EncoderImpl *>(encoder->object)->abort();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH(encoder)
}

bool plzma_encoder_compress(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_TRY_RETURN(encoder, false)
    return static_cast<EncoderImpl *>(encoder->object)->compress();
    LIBPLZMA_C_BINDINGS_OBJECT_EXEC_CATCH_RETURN(encoder, false)
}

void plzma_encoder_release(plzma_encoder * LIBPLZMA_NONNULL encoder) {
    plzma_object_exception_release(encoder);
    SharedPtr<EncoderImpl> encoderSPtr;
    encoderSPtr.assign(static_cast<EncoderImpl *>(encoder->object));
    encoder->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
