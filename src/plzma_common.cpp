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

#include "plzma_common.hpp"

#include <stdlib.h>

namespace plzma {
    
    uint64_t PROPVARIANTGetUInt64(const PROPVARIANT & prop) noexcept {
        switch (prop.vt) {
            case VT_UI8: return prop.uhVal.QuadPart;
            
            case VT_HRESULT:
            case VT_UI4:
                return prop.ulVal;
            
            case VT_UINT: return prop.uintVal;
            case VT_I8: return prop.hVal.QuadPart;
            case VT_UI1: return prop.bVal;
            case VT_I4: return prop.lVal;
            case VT_INT: return prop.intVal;
            
            default: return 0;
        }
    }
    
    bool PROPVARIANTGetBool(const PROPVARIANT & prop) noexcept {
        switch (prop.vt) {
            case VT_BOOL: return prop.boolVal != 0;
            default: return PROPVARIANTGetUInt64(prop) != 0;
        }
    }
    
    time_t FILETIMEToUnixTime(const FILETIME & filetime) noexcept {
        uint64_t t = filetime.dwHighDateTime;
        t <<= 32;
        t += filetime.dwLowDateTime;
        t -= 116444736000000000ULL;
        return static_cast<time_t>(t / 10000000);
    }
    
    FILETIME UnixTimeToFILETIME(const time_t t) noexcept {
        uint64_t ll = t;
        ll *= 10000000;
        ll += 116444736000000000ULL;
        
        FILETIME FT;
        FT.dwLowDateTime = static_cast<DWORD>(ll);
        FT.dwHighDateTime = static_cast<DWORD>(ll >> 32);
        return FT;
    }
}
