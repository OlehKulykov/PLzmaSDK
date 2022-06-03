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


#ifndef __PLZMA_OPEN_CALLBACK_HPP__
#define __PLZMA_OPEN_CALLBACK_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_base_callback.hpp"
#include "plzma_mutex.hpp"
#include "plzma_in_streams.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/IPassword.h"

namespace plzma {

    class OpenCallback final :
        public IArchiveOpenCallback,
        public ICryptoGetTextPassword,
        public ICryptoGetTextPassword2,
        public BaseCallback,
        public CMyUnknownImp {
    private:
        CMyComPtr<IInArchive> _archive;
        CMyComPtr<InStreamBase> _stream;
        plzma_size_t _itemsCount = 0;
        
        SharedPtr<Item> initialItemAt(const plzma_size_t index);
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(OpenCallback)
        
    public:
        MY_UNKNOWN_IMP3(IArchiveOpenCallback, ICryptoGetTextPassword, ICryptoGetTextPassword2)
        
        // IArchiveOpenCallback
        STDMETHOD(SetTotal)(const UInt64 * files, const UInt64 * bytes);
        STDMETHOD(SetCompleted)(const UInt64 * files, const UInt64 * bytes);
        
        // ICryptoGetTextPassword
        STDMETHOD(CryptoGetTextPassword)(BSTR * password);
        
        // ICryptoGetTextPassword2
        STDMETHOD(CryptoGetTextPassword2)(Int32 * passwordIsDefined, BSTR * password);
        
        CMyComPtr<IInArchive> archive() const noexcept;
        bool open();
        void abort();
        plzma_size_t itemsCount() noexcept;
        SharedPtr<Item> itemAt(const plzma_size_t index);
        SharedPtr<ItemArray> allItems();
        OpenCallback(const CMyComPtr<InStreamBase> & stream,
#if !defined(LIBPLZMA_NO_CRYPTO)
                     const String & passwd,
#endif
                     const plzma_file_type type);
        virtual ~OpenCallback() { }
    };
    
} // namespace plzma

#endif // !__PLZMA_OPEN_CALLBACK_HPP__
