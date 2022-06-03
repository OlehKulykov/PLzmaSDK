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


#ifndef __PLZMA_COMMON_HPP__
#define __PLZMA_COMMON_HPP__ 1

#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"

#include <stdio.h>

#include "CPP/Common/Common.h"
#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyString.h"

namespace plzma {
    
    LIBPLZMA_CPP_API_PRIVATE(uint64_t) PROPVARIANTGetUInt64(const PROPVARIANT & prop) noexcept;

    LIBPLZMA_CPP_API_PRIVATE(bool) PROPVARIANTGetBool(const PROPVARIANT & prop) noexcept;

    LIBPLZMA_CPP_API_PRIVATE(time_t) FILETIMEToUnixTime(const FILETIME & filetime) noexcept;

    LIBPLZMA_CPP_API_PRIVATE(FILETIME) UnixTimeToFILETIME(const time_t t) noexcept;
    
} // namespace plzma

#endif // !__PLZMA_COMMON_HPP__
