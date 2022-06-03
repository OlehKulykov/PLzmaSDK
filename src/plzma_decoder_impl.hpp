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


#ifndef __PLZMA_DECODER_IMPL_HPP__
#define __PLZMA_DECODER_IMPL_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_in_streams.hpp"
#include "plzma_out_streams.hpp"
#include "plzma_open_callback.hpp"
#include "plzma_extract_callback.hpp"
#include "plzma_common.hpp"
#include "plzma_c_bindings_private.hpp"
#include "plzma_progress.hpp"
#include "plzma_mutex.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/Common/Defs.h"
#include "CPP/7zip/Common/FileStreams.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/Windows/PropVariant.h"

namespace plzma {
    
    class DecoderImpl final : public CMyUnknownImp, public Decoder {
    private:
        friend struct SharedPtr<DecoderImpl>;
        LIBPLZMA_MUTEX(mutable _mutex)
#if !defined(LIBPLZMA_NO_CRYPTO)
        String _password;
#endif
        CMyComPtr<InStreamBase> _stream;
        CMyComPtr<OpenCallback> _openCallback;
        CMyComPtr<ExtractCallback> _extractCallback;
#if !defined(LIBPLZMA_NO_PROGRESS)
        SharedPtr<Progress> _progress;
#endif
        plzma_file_type _type = plzma_file_type_7z;
        bool _opened = false;
        bool _opening = false;
        bool _aborted = false;
        
        virtual void retain() override final;
        virtual void release() override final;
        
        template<typename ... ARGS>
        bool process(ARGS&&... args) {
            LIBPLZMA_UNIQUE_LOCK(lock, _mutex)
            if (!_opened || _extractCallback) {
                return false;
            }
            
            CMyComPtr<DecoderImpl> selfPtr(this);
            
#if defined(LIBPLZMA_NO_PROGRESS)
#  if defined(LIBPLZMA_NO_CRYPTO)
            CMyComPtr<ExtractCallback> extractCallback(new ExtractCallback(_openCallback->archive(), _type));
#  else
            CMyComPtr<ExtractCallback> extractCallback(new ExtractCallback(_openCallback->archive(), _password, _type));
#  endif
#else
            _progress->reset();
#  if defined(LIBPLZMA_NO_CRYPTO)
            CMyComPtr<ExtractCallback> extractCallback(new ExtractCallback(_openCallback->archive(), _progress, _type));
#  else
            CMyComPtr<ExtractCallback> extractCallback(new ExtractCallback(_openCallback->archive(), _password, _progress, _type));
#  endif
#endif
            _extractCallback = extractCallback;
            
            LIBPLZMA_UNIQUE_LOCK_UNLOCK(lock)
            extractCallback->process(static_cast<ARGS &&>(args)...);
            LIBPLZMA_UNIQUE_LOCK_LOCK(lock)
            
            CMyComPtr<ExtractCallback> tmpExtractCallback(static_cast<CMyComPtr<ExtractCallback> &&>(_extractCallback));
            tmpExtractCallback.Release();
            
            if (_aborted) {
                _stream->close();
            }
            
            return true;
        }
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(DecoderImpl)
        
    public:
        MY_ADDREF_RELEASE
        
        virtual void setPassword(const wchar_t * LIBPLZMA_NULLABLE password) override final;
        virtual void setPassword(const char * LIBPLZMA_NULLABLE password) override final;
        virtual void setProgressDelegate(ProgressDelegate * LIBPLZMA_NULLABLE delegate) override final;
        virtual bool open() override final;
        virtual void abort() override final;
        virtual plzma_size_t count() const override final;
        virtual SharedPtr<ItemArray> items() const override final;
        virtual SharedPtr<Item> itemAt(const plzma_size_t index) const override final;
        virtual bool extract(const Path & path, const bool usingItemsFullPath = true) override final;
        virtual bool extract(const SharedPtr<ItemArray> & items,
                             const Path & path,
                             const bool usingItemsFullPath = true) override final;
        virtual bool extract(const SharedPtr<ItemOutStreamArray> & items) override final;
        virtual bool test(const SharedPtr<ItemArray> & items) override final;
        virtual bool test() override final;
        
#if !defined(LIBPLZMA_NO_C_BINDINGS)
        void setUtf8Callback(plzma_progress_delegate_utf8_callback LIBPLZMA_NULLABLE callback);
        void setWideCallback(plzma_progress_delegate_wide_callback LIBPLZMA_NULLABLE callback);
#endif
        
        DecoderImpl(const CMyComPtr<InStreamBase> & stream,
                    const plzma_file_type type,
                    const plzma_context context);
        virtual ~DecoderImpl();
    };
    
} // namespace plzma

#endif // !__PLZMA_DECODER_IMPL_HPP__
