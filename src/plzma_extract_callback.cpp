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

#include "plzma_extract_callback.hpp"
#include "plzma_common.hpp"
#include "plzma_c_bindings_private.hpp"

#include "CPP/Common/Defs.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/7zip/Archive/Common/DummyOutStream.h"

namespace plzma {
    
    using namespace NArchive::NExtract;
    
    STDMETHODIMP ExtractCallback::ReportExtractResult(UInt32 indexType, UInt32 index, Int32 opRes) {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        return _result;
#else
        const FailableLockGuard lock(_mutex);
        RINOK(lock.res())
        return _result;
#endif
    }
    
    STDMETHODIMP ExtractCallback::SetRatioInfo(const UInt64 * inSize, const UInt64 * outSize) {
#if defined(LIBPLZMA_THREAD_UNSAFE)
        return _result;
#else
        const FailableLockGuard lock(_mutex);
        RINOK(lock.res())
        return _result;
#endif
    }
    
    STDMETHODIMP ExtractCallback::SetTotal(UInt64 size) {
#if defined(LIBPLZMA_NO_PROGRESS)
        return S_OK;
#else
        return setProgressTotal(size);
#endif
    }
    
    STDMETHODIMP ExtractCallback::SetCompleted(const UInt64 * completeValue) {
#if defined(LIBPLZMA_NO_PROGRESS)
        return S_OK;
#else
        return completeValue ? setProgressCompleted(*completeValue) : S_OK;
#endif
    }
    
    STDMETHODIMP ExtractCallback::CryptoGetTextPassword(BSTR * password) {
        return getTextPassword(nullptr, password);
    }
    
    STDMETHODIMP ExtractCallback::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) {
        return getTextPassword(passwordIsDefined, password);
    }
    
    void ExtractCallback::getTestStream(const UInt32 index, ISequentialOutStream ** outStream) {
#if !defined(LIBPLZMA_NO_PROGRESS)
        Path itemPath;
        NWindows::NCOM::CPropVariant pathProp;
        if (_archive->GetProperty(index, kpidPath, &pathProp) == S_OK &&
            (pathProp.vt == VT_EMPTY || pathProp.vt == VT_BSTR)) {
                itemPath.set(pathProp.bstrVal);
        }
        
        if (_type != plzma_file_type_xz && itemPath.count() == 0) {
            throw Exception(plzma_error_code_internal, "Can't read item path.", __FILE__, __LINE__);
        }
#endif
        
        OutTestStream * stream = new OutTestStream();
        _currentOutStream = stream;
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setPath(static_cast<Path &&>(itemPath));
#endif
        stream->AddRef(); // for '*outStream'
        *outStream = stream;
    }
    
    void ExtractCallback::getExtractStream(const UInt32 index, ISequentialOutStream ** outStream) {
        if (_itemsMap) {
            const auto * pair = _itemsMap->bsearch<plzma_size_t>(index);
            if (pair && pair->first && pair->first->index() == index) {
                auto base = pair->second.cast<OutStreamBase>();
                OutStreamBase * stream = base.get();
                _currentOutStream = stream;
#if !defined(LIBPLZMA_NO_PROGRESS)
                _progress->setPath(pair->first->path());
#endif
                stream->AddRef(); // for '*outStream'
                *outStream = stream;
                return;
            }
            throw Exception(plzma_error_code_internal, "Can't find maped item/stream.", __FILE__, __LINE__);
        }
        
        Path itemPath;
        Path fullPath(_path);
        
        NWindows::NCOM::CPropVariant prop;
        if (_archive->GetProperty(index, kpidPath, &prop) == S_OK && prop.vt == VT_BSTR) {
            itemPath.set(prop.bstrVal);
        }
        
        if (_type != plzma_file_type_xz) {
            if (itemPath.count() == 0) {
                throw Exception(plzma_error_code_internal, "Can't read item path.", __FILE__, __LINE__);
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidIsDir, &prop) != S_OK) {
                throw Exception(plzma_error_code_internal, "Can't check item is directory.", __FILE__, __LINE__);
            }
            
            if (PROPVARIANTGetBool(prop)) { // directory
                if (_itemsFullPath) {
                    fullPath.append(itemPath);
                    if (!fullPath.createDir(true)) {
                        Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                        exception.setWhat("Can't create output directory at path: ", fullPath.utf8(), nullptr);
                        throw exception;
                    }
                }
                return;
            }
            
            const auto itemName = itemPath.lastComponent();
            if (_itemsFullPath) {
                fullPath.append(itemPath);
                fullPath.removeLastComponent();
                if (!fullPath.createDir(true)) {
                    Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                    exception.setWhat("Can't create output directory at path: ", fullPath.utf8(), nullptr);
                    throw exception;
                }
            }
            fullPath.append(itemName);
        } else {
            bool isDir = true;
            if (fullPath.exists(&isDir) && isDir) {
                Exception exception(plzma_error_code_io, "Can't extract xz item to path.", __FILE__, __LINE__);
                exception.setReason("The target path is directory. Path: ", fullPath.utf8(), nullptr);
                throw exception;
            }
            itemPath = fullPath.lastComponent();
            fullPath.removeLastComponent();
            if (!fullPath.createDir(true)) {
                Exception exception(plzma_error_code_io, nullptr, __FILE__, __LINE__);
                exception.setWhat("Can't create output directory at path: ", fullPath.utf8(), nullptr);
                throw exception;
            }
            fullPath.append(itemPath);
        }
        
        OutFileStream * stream = new OutFileStream(static_cast<Path &&>(fullPath));
        _currentOutStream = stream;
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->setPath(static_cast<Path &&>(itemPath));
#endif
        stream->AddRef(); // for '*outStream'
        *outStream = stream;
    }
    
    STDMETHODIMP ExtractCallback::GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode) {
        *outStream = nullptr;
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result != S_OK) {
                return _result;
            } else if (_solidArchive && (index < _extractingFirstIndex || index > _extractingLastIndex)) {
                return S_OK; // skip index in solid archive
            }
            CMyComPtr<OutStreamBase> currentOutStream(static_cast<CMyComPtr<OutStreamBase> &&>(_currentOutStream));
            if (currentOutStream) {
                currentOutStream->close();
            }
            
            switch (askExtractMode) {
                case NAskMode::kExtract: getExtractStream(index, outStream); break;
                case NAskMode::kTest: getTestStream(index, outStream); break;
                case NAskMode::kSkip: break;
                default: _result = E_INVALIDARG; break;
            }
            currentOutStream.Release();
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
            _exception = Exception::create(plzma_error_code_unknown, "Can't create extract stream.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP ExtractCallback::PrepareOperation(Int32 askExtractMode) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_result == S_OK && _currentOutStream) {
                _currentOutStream->open();
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
            _exception = Exception::create(plzma_error_code_unknown, "Can't prepare extract operation.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    STDMETHODIMP ExtractCallback::SetOperationResult(Int32 operationResult) {
        try {
            LIBPLZMA_LOCKGUARD(lock, _mutex)
            if (_currentOutStream) {
                _currentOutStream->close();
                _currentOutStream.Release();
            }
            if (_result == S_OK) {
                switch (operationResult) {
                    case NOperationResult::kOK:
                        return S_OK;
                    default:
                        _exception = Exception::create(plzma_error_code_internal, "Item extracted with error.", __FILE__, __LINE__);
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
            _exception = Exception::create(plzma_error_code_unknown, "Can't process extract operation result.", __FILE__, __LINE__);
            return E_FAIL;
        }
        return S_OK;
    }
    
    void ExtractCallback::process() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        
        CMyComPtr<ExtractCallback> selfPtr(this);
        NWindows::NCOM::CPropVariant prop;
        if (_archive->GetArchiveProperty(kpidSolid, &prop) != S_OK) {
            throw Exception(plzma_error_code_internal, "Can't read archive solid property.", __FILE__, __LINE__);
        }
        _solidArchive = PROPVARIANTGetBool(prop);
        
        UInt32 itemsCount = 0, itemIndex = 0;
        if (_itemsArray) {
            _itemsArray->sort();
            itemsCount = _itemsArray->count();
        } else if (_itemsMap) {
            _itemsMap->sort();
            itemsCount = _itemsMap->count();
        } else {
            UInt32 numItems = 0;
            if (_archive->GetNumberOfItems(&numItems) != S_OK) {
                throw Exception(plzma_error_code_internal, "Can't get number of archive items.", __FILE__, __LINE__);
            }
            itemsCount = numItems;
        }
        
        const UInt32 maxIndicies = 256;
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->reset();
        UInt32 partsCount = itemsCount / maxIndicies;
        partsCount = MyMax<UInt32>(1, partsCount);
        if (itemsCount > partsCount * maxIndicies) {
            partsCount++;
        }
        _progress->setPartsCount(partsCount);
#endif
        
        do {
            UInt32 indicies[maxIndicies];
            UInt32 fromIndex = 0, toIndex = 0, indicesCount = 0;
            if (_itemsArray) {
                while (indicesCount < maxIndicies && itemIndex < itemsCount) {
                    indicies[indicesCount] = toIndex = _itemsArray->at(itemIndex)->index();
                    if (indicesCount == 0) { fromIndex = toIndex; }
                    itemIndex++; indicesCount++;
                }
            } else if (_itemsMap) {
                while (indicesCount < maxIndicies && itemIndex < itemsCount) {
                    indicies[indicesCount] = toIndex = _itemsMap->at(itemIndex).first->index();
                    if (indicesCount == 0) { fromIndex = toIndex; }
                    itemIndex++; indicesCount++;
                }
            } else {
                fromIndex = itemIndex;
                while (indicesCount < maxIndicies && itemIndex < itemsCount) {
                    indicies[indicesCount++] = toIndex = itemIndex++;         // +1
                }
            }
            _extractingFirstIndex = fromIndex;
            _extractingLastIndex = toIndex;
#if !defined(LIBPLZMA_NO_PROGRESS)
            _progress->startPart();
#endif
            _extracting = true;
            
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            const HRESULT result = (indicesCount > 0) ? _archive->Extract(indicies, indicesCount, _mode, this) : S_OK;
            LIBPLZMA_UNIQUE_LOCK_LOCK(lock)
            
            _extracting = false;
            if (_currentOutStream) {
                _currentOutStream->close();
                _currentOutStream.Release();
            }
            
            if (result != S_OK || _result != S_OK) {
                if (result == E_ABORT || _result == E_ABORT) {
                    return; // aborted -> without exception
                } else if (_exception) {
                    Exception localException(static_cast<Exception &&>(*_exception));
                    delete _exception;
                    _exception = nullptr;
                    throw localException;
                }
                throw Exception(plzma_error_code_internal, "Unknown extract error.", __FILE__, __LINE__);
            }
        } while (itemIndex < itemsCount);
        
#if !defined(LIBPLZMA_NO_PROGRESS)
        _progress->finish();
#endif
    }
    
    void ExtractCallback::process(const Int32 mode, const SharedPtr<ItemArray> & items, const Path & path, const bool itemsFullPath) {
        _path.set(path);
        _mode = mode;
        _itemsArray = items;
        _itemsFullPath = itemsFullPath;
        process();
    }
    
    void ExtractCallback::process(const Int32 mode, const Path & path, const bool itemsFullPath) {
        _path.set(path);
        _mode = mode;
        _itemsFullPath = itemsFullPath;
        process();
    }
    
    void ExtractCallback::process(const Int32 mode, const SharedPtr<ItemOutStreamArray> & items) {
        _mode = mode;
        _itemsMap = items;
        process();
    }
    
    void ExtractCallback::process(const Int32 mode, const SharedPtr<ItemArray> & items) {
        _mode = mode;
        _itemsArray = items;
        process();
    }
    
    void ExtractCallback::process(const Int32 mode) {
        _mode = mode;
        process();
    }
    
    void ExtractCallback::abort() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _result = E_ABORT;
        if (!_extracting && _currentOutStream) {
            _currentOutStream->close();
            _currentOutStream.Release();
        }
    }
    
    ExtractCallback::ExtractCallback(const CMyComPtr<IInArchive> & archive,
#if !defined(LIBPLZMA_NO_CRYPTO)
                                     const String & passwd,
#endif
#if !defined(LIBPLZMA_NO_PROGRESS)
                                     const SharedPtr<Progress> & progress,
#endif
                                     const plzma_file_type type) : CMyUnknownImp(),
        _archive(archive),
        _type(type) {
#if !defined(LIBPLZMA_NO_CRYPTO)
            _password = passwd;
#endif
#if !defined(LIBPLZMA_NO_PROGRESS)
            _progress = progress;
#endif
    }
    
} // namespace plzma
