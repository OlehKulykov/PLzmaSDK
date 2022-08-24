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


#ifndef __PLZMA_PRIVATE_HPP__
#define __PLZMA_PRIVATE_HPP__ 1

#include <cstddef>

#include "plzma_private.h"

#define LIBPLZMA_CAST_VALUE_TO_PTR(P,T,V) if(P){*P=static_cast<T>(V);}

#define LIBPLZMA_SET_VALUE_TO_PTR(P,V) if(P){*P=V;}

#define LIBPLZMA_RETAIN_IMPL(REF_COUNTER) REF_COUNTER++;

#define LIBPLZMA_RELEASE_IMPL(REF_COUNTER) \
LIBPLZMA_DEBUG_ASSERT(REF_COUNTER > 0) \
if (--REF_COUNTER > 0) { \
    return; \
} \
delete this; \


#define LIBPLZMA_RETAIN_LOCKED_IMPL(REF_COUNTER,LOCK) \
LOCK.lock(); \
REF_COUNTER++; \
LOCK.unlock(); \


#define LIBPLZMA_RELEASE_LOCKED_IMPL(REF_COUNTER,LOCK) \
size_t refCnt; \
LOCK.lock(); \
LIBPLZMA_DEBUG_ASSERT(REF_COUNTER > 0) \
refCnt = --REF_COUNTER; \
LOCK.unlock(); \
if (refCnt > 0) { \
    return; \
} \
delete this; \


#define LIBPLZMA_NON_COPYABLE_NON_MOVABLE(CLASS_NAME) \
CLASS_NAME(CLASS_NAME &&) = delete; \
CLASS_NAME & operator = (CLASS_NAME &&) = delete; \
CLASS_NAME & operator = (const CLASS_NAME &) = delete; \
CLASS_NAME(const CLASS_NAME &) = delete; \


namespace plzma {
    
    /// CFilterCoder, ISequentialOutStream default: 1 << 20
    LIBPLZMA_CPP_API_PRIVATE(plzma_size_t) kStreamReadSize;
    
    /// CFilterCoder, ISequentialOutStream default: 1 << 20
    LIBPLZMA_CPP_API_PRIVATE(plzma_size_t) kStreamWriteSize;
    
    /// CDecoder default: 1 << 20
    LIBPLZMA_CPP_API_PRIVATE(plzma_size_t) kDecoderReadSize;
    
    /// CDecoder default: 1 << 22
    LIBPLZMA_CPP_API_PRIVATE(plzma_size_t) kDecoderWriteSize;
    
    LIBPLZMA_CPP_API_PRIVATE(void) initialize(void);

} // namespace plzma

#endif // !__PLZMA_PRIVATE_HPP__
