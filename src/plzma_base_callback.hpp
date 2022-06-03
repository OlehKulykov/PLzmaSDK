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


#ifndef __PLZMA_BASE_CALLBACK_HPP__
#define __PLZMA_BASE_CALLBACK_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_mutex.hpp"
#include "plzma_progress.hpp"

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"
#include "CPP/Common/MyCom.h"
#include "CPP/Common/Defs.h"
#include "CPP/7zip/Archive/IArchive.h"

namespace plzma {
    
    class BaseCallback {
    protected:
        LIBPLZMA_MUTEX(mutable _mutex)
#if !defined(LIBPLZMA_NO_PROGRESS)
        SharedPtr<Progress> _progress;
#endif
#if !defined(LIBPLZMA_NO_CRYPTO)
        String _password;
#endif
        Exception * _exception = nullptr;   // execution thread only
        HRESULT _result = S_OK;             // shared between threads
        
        HRESULT getTextPassword(Int32 * passwordIsDefined, BSTR * password) noexcept;
#if !defined(LIBPLZMA_NO_PROGRESS)
        HRESULT setProgressTotal(const uint64_t total) noexcept;
        HRESULT setProgressCompleted(const uint64_t completed) noexcept;
#endif
        
        LIBPLZMA_NON_COPYABLE_NON_MOVABLE(BaseCallback)
        
    public:
        BaseCallback() = default;
        virtual ~BaseCallback();
        
        template<typename T>
        static CMyComPtr<T> createArchive(const plzma_file_type type);
    };
    
} // namespace plzma

#endif // !__PLZMA_BASE_CALLBACK_HPP__
