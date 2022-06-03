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

#include "plzma_open_callback.hpp"
#include "plzma_common.hpp"

namespace plzma {
    
    STDMETHODIMP OpenCallback::SetTotal(const UInt64 * files, const UInt64 * bytes) {
        return S_OK; // unused
    }
    
    STDMETHODIMP OpenCallback::SetCompleted(const UInt64 * files, const UInt64 * bytes) {
        return S_OK; // unused
    }
    
    STDMETHODIMP OpenCallback::CryptoGetTextPassword(BSTR * password) {
        return getTextPassword(nullptr, password);
    }
    
    STDMETHODIMP OpenCallback::CryptoGetTextPassword2(Int32 * passwordIsDefined, BSTR * password) {
        return getTextPassword(passwordIsDefined, password);
    }
    
    bool OpenCallback::open() {
        LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
        if (_result != S_OK) {
            return false;
        }
        CMyComPtr<OpenCallback> selfPtr(this);
        
        LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
        HRESULT result = _archive->Open(_stream, nullptr, this);
        UInt32 numItems = 0;
        if (result == S_OK) {
            result = _archive->GetNumberOfItems(&numItems);
        }
        LIBPLZMA_UNIQUE_LOCK_LOCK(lock)
        
        if (result == S_OK && _result == S_OK) {
            _itemsCount = numItems;
            return true;
        } else if (result == E_ABORT || _result == E_ABORT) {
            _itemsCount = 0;
            return false; // aborted -> false without exception
        }
        
        if (_exception) {
            Exception localException(static_cast<Exception &&>(*_exception));
            delete _exception;
            _exception = nullptr;
            throw localException;
        }
        
        Exception internalException(plzma_error_code_internal, "Can't open in archive.", __FILE__, __LINE__);
#if defined(LIBPLZMA_NO_CRYPTO)
        internalException.setReason("no crypto", nullptr);
#endif
        throw internalException;
    }
    
    void OpenCallback::abort() {
        LIBPLZMA_LOCKGUARD(lock, _mutex)
        _result = E_ABORT;
        _itemsCount = 0;
    }
    
    CMyComPtr<IInArchive> OpenCallback::archive() const noexcept {
        return _archive;
    }
    
    plzma_size_t OpenCallback::itemsCount() noexcept {
        return _itemsCount;
    }
    
    SharedPtr<Item> OpenCallback::initialItemAt(const plzma_size_t index) {
        if (index < _itemsCount) {
            NWindows::NCOM::CPropVariant path, size;
            SharedPtr<Item> item;
            if (_archive->GetProperty(index, kpidPath, &path) == S_OK &&
                _archive->GetProperty(index, kpidSize, &size) == S_OK &&
                (path.vt == VT_EMPTY || path.vt == VT_BSTR)) {
                    item = makeShared<Item>(static_cast<Path &&>(Path(path.bstrVal)), index);
                    item->setSize(PROPVARIANTGetUInt64(size));
            }
            return item;
        }
        return SharedPtr<Item>();
    }
    
    SharedPtr<Item> OpenCallback::itemAt(const plzma_size_t index) {
        auto item = initialItemAt(index);
        if (item) {
            NWindows::NCOM::CPropVariant prop;
            if (_archive->GetProperty(index, kpidPackSize, &prop) == S_OK) {
                item->setPackSize(PROPVARIANTGetUInt64(prop));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidCTime, &prop) == S_OK && prop.vt == VT_FILETIME) {
                item->setCreationTime(FILETIMEToUnixTime(prop.filetime));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidATime, &prop) == S_OK && prop.vt == VT_FILETIME) {
                item->setAccessTime(FILETIMEToUnixTime(prop.filetime));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidMTime, &prop) == S_OK && prop.vt == VT_FILETIME) {
                item->setModificationTime(FILETIMEToUnixTime(prop.filetime));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidEncrypted, &prop) == S_OK) {
                item->setEncrypted(PROPVARIANTGetBool(prop));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidCRC, &prop) == S_OK) {
                item->setCrc32(static_cast<uint32_t>(PROPVARIANTGetUInt64(prop)));
            }
            
            prop.Clear();
            if (_archive->GetProperty(index, kpidIsDir, &prop) == S_OK) {
                item->setIsDir(PROPVARIANTGetBool(prop));
            }
        }
        return item;
    }
    
    SharedPtr<ItemArray> OpenCallback::allItems() {
        auto items = makeShared<ItemArray>(_itemsCount);
        for (plzma_size_t i = 0; i < _itemsCount; i++) {
            items->push(static_cast<SharedPtr<Item> &&>(itemAt(i)));
        }
        return items;
    }
    
    OpenCallback::OpenCallback(const CMyComPtr<InStreamBase> & stream,
#if !defined(LIBPLZMA_NO_CRYPTO)
                               const String & passwd,
#endif
                               const plzma_file_type type) : CMyUnknownImp(),
        _archive(createArchive<IInArchive>(type)),
        _stream(stream) {
#if !defined(LIBPLZMA_NO_CRYPTO)
            _password = passwd;
#endif
    }
    
} // namespace plzma

