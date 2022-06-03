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


#ifndef __PLZMA_EXTRACT_CALLBACK_HPP__
#define __PLZMA_EXTRACT_CALLBACK_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_in_streams.hpp"
#include "plzma_out_streams.hpp"
#include "plzma_mutex.hpp"
#include "plzma_base_callback.hpp"
#include "plzma_progress.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/Common/FileStreams.h"

namespace plzma {
    
    class ExtractCallback final :
        public IArchiveExtractCallback,
        public IArchiveExtractCallbackMessage,
        public ICryptoGetTextPassword,
        public ICryptoGetTextPassword2,
        public ICompressProgressInfo,
        public BaseCallback,
        public CMyUnknownImp {
    private:
        Path _path;
        CMyComPtr<InStreamBase> _stream;
        CMyComPtr<OutStreamBase> _currentOutStream;
        CMyComPtr<IInArchive> _archive;
        SharedPtr<ItemOutStreamArray> _itemsMap;
        SharedPtr<ItemArray> _itemsArray;
        UInt32 _extractingFirstIndex = 0;
        UInt32 _extractingLastIndex = 0;
        Int32 _mode = 0; // The value of the 'NArchive::NExtract::NAskMode' anonymous enum.
        plzma_file_type _type = plzma_file_type_7z;
        bool _itemsFullPath = true;
        bool _solidArchive = false;
        bool _extracting = false;
        
        void getTestStream(const UInt32 index, ISequentialOutStream ** outStream);
        void getExtractStream(const UInt32 index, ISequentialOutStream ** outStream);
        
        void process();
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(ExtractCallback)
        
    public:
        MY_UNKNOWN_IMP4(IArchiveExtractCallbackMessage, ICryptoGetTextPassword, ICryptoGetTextPassword2, ICompressProgressInfo)
        
        INTERFACE_IArchiveExtractCallback(;)
        INTERFACE_IArchiveExtractCallbackMessage(;)
        
        STDMETHOD(SetRatioInfo)(const UInt64 * inSize, const UInt64 * outSize);
        
        // ICryptoGetTextPassword
        STDMETHOD(CryptoGetTextPassword)(BSTR * password);
        
        // ICryptoGetTextPassword2
        STDMETHOD(CryptoGetTextPassword2)(Int32 * passwordIsDefined, BSTR * password);
        
        void process(const Int32 mode, const SharedPtr<ItemArray> & items, const Path & path, const bool itemsFullPath = true);
        void process(const Int32 mode, const Path & path, const bool itemsFullPath = true);
        void process(const Int32 mode, const SharedPtr<ItemOutStreamArray> & items);
        void process(const Int32 mode, const SharedPtr<ItemArray> & items);
        void process(const Int32 mode);
        void abort();
        ExtractCallback(const CMyComPtr<IInArchive> & archive,
#if !defined(LIBPLZMA_NO_CRYPTO)
                        const String & passwd,
#endif
#if !defined(LIBPLZMA_NO_PROGRESS)
                        const SharedPtr<Progress> & progress,
#endif
                        const plzma_file_type type);
        virtual ~ExtractCallback() { }
    };
    
} // namespace plzma

#endif // !__PLZMA_EXTRACT_CALLBACK_HPP__
