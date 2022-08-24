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
#include <cassert>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_common.hpp"

// See 'trailingBytesForUTF8' array.
#define CLZMA_STRING_MAX_BYTES_PER_WCHAR 5

namespace plzma {

namespace StringConvertUTF {

#include "plzma_convert_utf.hpp"

} // namespace StringConvertUTF
    
    void String::moveFrom(String && str, const plzma_erase eraseType) noexcept {
        _ws.erase(eraseType, sizeof(wchar_t) * _size);
        _cs.erase(eraseType, sizeof(char) * _cslen);
        _ws = static_cast<RawHeapMemory &&>(str._ws);
        _cs = static_cast<RawHeapMemory &&>(str._cs);
        _size = str._size;
        _cslen = str._cslen;
        str._size = str._cslen = 0;
    }
    
    void String::copyFrom(const String & str, const plzma_erase eraseType) {
        if (str._cs && str._cslen > 0) {
            _cs.erase(eraseType, sizeof(char) * _cslen);
            _cs.resize(sizeof(char) * (str._cslen + 1));
            memcpy(_cs, str._cs, sizeof(char) * str._cslen);
            static_cast<char *>(_cs)[str._cslen] = 0;
        } else {
            _cs.clear(eraseType, sizeof(char) * _cslen);
        }
        if (str._ws && str._size > 0) {
            _ws.erase(eraseType, sizeof(wchar_t) * _size);
            _ws.resize(sizeof(wchar_t) * (str._size + 1));
            memcpy(_ws, str._ws, sizeof(wchar_t) * str._size);
            static_cast<wchar_t *>(_ws)[str._size] = 0;
        } else {
            _ws.clear(eraseType, sizeof(wchar_t) * _size);
        }
        _size = str._size;
        _cslen = str._cslen;
    }
    
    void String::copyFrom(const wchar_t * LIBPLZMA_NULLABLE str, const plzma_erase eraseType) {
        const size_t len = str ? wcslen(str) : 0;
        if (len > 0) {
            _ws.erase(eraseType, sizeof(wchar_t) * _size);
            _ws.resize(sizeof(wchar_t) * (len + 1));
            memcpy(_ws, str, sizeof(wchar_t) * len);
            static_cast<wchar_t *>(_ws)[len] = 0;
            _size = static_cast<plzma_size_t>(len);
            _cs.clear(eraseType, sizeof(char) * _cslen);
            _cslen = 0;
        } else {
            clear(eraseType);
        }
    }
    
    void String::copyFrom(const char * LIBPLZMA_NULLABLE str, const plzma_erase eraseType) {
        const auto len = lengthMaxCount(str, static_cast<size_t>(plzma_max_size()));
        if (len.first > 0) {
            _cs.erase(eraseType, sizeof(char) * _cslen);
            _cs.resize(sizeof(char) * (len.first + 1));
            memcpy(_cs, str, sizeof(char) * len.first);
            static_cast<char *>(_cs)[len.first] = 0;
            _ws.clear(eraseType, sizeof(wchar_t) * _size);
            _cslen = static_cast<plzma_size_t>(len.first);
            _size = static_cast<plzma_size_t>(len.second);
        } else {
            clear(eraseType);
        }
    }
    
    plzma_size_t String::count() const noexcept {
        return _size;
    }
    
    void String::syncWide() const {
        const wchar_t * w = wide();
        assert(w);
        (void)w;
    }
    
    void String::syncUtf8() const {
        const char * c = utf8();
        assert(c);
        (void)c;
    }
    
    const char * LIBPLZMA_NONNULL String::utf8() const {
        using namespace StringConvertUTF;
        
        if (!_cs && _ws) {
            const size_t memSize = CLZMA_STRING_MAX_BYTES_PER_WCHAR * (_size + 1);
            _cs.resize(memSize);
            _cs.erase(plzma_erase_zero, memSize);
            UTF8 * dst = static_cast<UTF8 *>(_cs);
            ConversionResult convRes = sourceIllegal;
            if (sizeof(wchar_t) == sizeof(UTF32)) {
                const UTF32 * src = static_cast<const UTF32 *>(_ws);
                convRes = ConvertUTF32toUTF8(&src, src + _size, &dst, dst + memSize, strictConversion);
                if (convRes != conversionOK) {
                    src = static_cast<const UTF32 *>(_ws);
                    dst = static_cast<UTF8 *>(_cs);
                    convRes = ConvertUTF32toUTF8(&src, src + _size, &dst, dst + memSize, lenientConversion);
                }
            } else if (sizeof(wchar_t) == sizeof(UTF16)) {
                const UTF16 * src = static_cast<const UTF16 *>(_ws);
                convRes = ConvertUTF16toUTF8(&src, src + _size, &dst, dst + memSize, strictConversion);
                if (convRes != conversionOK) {
                    src = static_cast<const UTF16 *>(_ws);
                    dst = static_cast<UTF8 *>(_cs);
                    convRes = ConvertUTF16toUTF8(&src, src + _size, &dst, dst + memSize, lenientConversion);
                }
            }
            if (convRes != conversionOK) {
                throw Exception(plzma_error_code_internal, "Wide character to UTF8 string conversion.", __FILE__, __LINE__);
            }
            const size_t csLen = strlen(static_cast<const char *>(_cs));
            if (memSize > (csLen + 1)) {
                _cs.resize(csLen + 1);
            }
            static_cast<char *>(_cs)[csLen] = 0;
            _cslen = static_cast<plzma_size_t>(csLen);
        }
        return _cs ? _cs : plzma_empty_cstring;
    }
    
    const wchar_t * LIBPLZMA_NONNULL String::wide() const {
        using namespace StringConvertUTF;
        
        if (!_ws && _cs) {
            _ws.resize(sizeof(wchar_t) * (_size + 1));
            const UTF8 * src = static_cast<const UTF8 *>(_cs);
            ConversionResult convRes = sourceIllegal;
            if (sizeof(wchar_t) == sizeof(UTF32)) {
                UTF32 * dst = static_cast<UTF32 *>(_ws);
                convRes = ConvertUTF8toUTF32(&src, src + _cslen, &dst, dst + _size, strictConversion);
                if (convRes != conversionOK) {
                    src = static_cast<const UTF8 *>(_cs);
                    dst = static_cast<UTF32 *>(_ws);
                    convRes = ConvertUTF8toUTF32(&src, src + _cslen, &dst, dst + _size, lenientConversion);
                }
            } else if (sizeof(wchar_t) == sizeof(UTF16)) {
                UTF16 * dst = static_cast<UTF16 *>(_ws);
                convRes = ConvertUTF8toUTF16(&src, src + _cslen, &dst, dst + _size, strictConversion);
                if (convRes != conversionOK) {
                    src = static_cast<const UTF8 *>(_cs);
                    dst = static_cast<UTF16 *>(_ws);
                    convRes = ConvertUTF8toUTF16(&src, src + _cslen, &dst, dst + _size, lenientConversion);
                }
            }
            if (convRes != conversionOK) {
                throw Exception(plzma_error_code_internal, "UTF8 to wide character string conversion.", __FILE__, __LINE__);
            }
            static_cast<wchar_t *>(_ws)[_size] = 0;
        }
        return _ws ? _ws : plzma_empty_wstring;
    }
    
    void String::clear(const plzma_erase eraseType) noexcept {
        _ws.clear(eraseType, sizeof(wchar_t) * _size);
        _cs.clear(eraseType, sizeof(char) * _cslen);
        _size = _cslen = 0;
    }
    
    void String::set(const String & str) {
        copyFrom(str);
    }
    
    void String::set(const wchar_t * LIBPLZMA_NULLABLE str) {
        copyFrom(str);
    }
    
    void String::set(const char * LIBPLZMA_NULLABLE str) {
        copyFrom(str);
    }
    
    void String::append(const wchar_t * LIBPLZMA_NONNULL * LIBPLZMA_NONNULL stringsList,
                        const Pair<size_t, size_t> * LIBPLZMA_NONNULL sizesList,
                        const size_t count,
                        const plzma_erase eraseType) {
        syncWide();
        size_t strSize = 0;
        for (size_t i = 0; i < count; i++) {
            strSize += sizesList[i].second;
        }
        if (strSize > 0) {
            const size_t dstSize = strSize + _size;
            const size_t memSize = sizeof(wchar_t) * (dstSize + 1);
            _ws.resize(memSize);
            for (size_t i = 0, offset = _size; i < count; i++) {
                if (sizesList[i].second > 0) {
                    memcpy(static_cast<wchar_t *>(_ws) + offset, stringsList[i], sizeof(wchar_t) * sizesList[i].second);
                    offset += sizesList[i].second;
                }
            }
            static_cast<wchar_t *>(_ws)[dstSize] = 0;
            _size = static_cast<plzma_size_t>(dstSize);
            _cs.clear(eraseType, sizeof(char) * _cslen);
            _cslen = 0;
        }
    }
    
    void String::append(const char * LIBPLZMA_NONNULL * LIBPLZMA_NONNULL stringsList,
                        const Pair<size_t, size_t> * LIBPLZMA_NONNULL sizesList,
                        const size_t count,
                        const plzma_erase eraseType) {
        syncUtf8();
        Pair<size_t, size_t> strLen;
        strLen.first = strLen.second = 0;
        for (size_t i = 0; i < count; i++) {
            strLen.first += sizesList[i].first;
            strLen.second += sizesList[i].second;
        }
        if (strLen.second > 0) {
            const size_t dstSize = _size + strLen.second;
            const size_t dstLen = strLen.first + _cslen;
            _cs.resize(sizeof(char) * (dstLen + 1));
            for (size_t i = 0, offset = _cslen; i < count; i++) {
                if (sizesList[i].first > 0) {
                    memcpy(static_cast<char *>(_cs) + offset, stringsList[i], sizeof(char) * sizesList[i].first);
                    offset += sizesList[i].first;
                }
            }
            static_cast<char *>(_cs)[dstLen] = 0;
            _ws.clear(eraseType, sizeof(wchar_t) * _size);
            _size = static_cast<plzma_size_t>(dstSize);
            _cslen = static_cast<plzma_size_t>(dstLen);
        }
    }

    void String::append(const wchar_t * LIBPLZMA_NULLABLE str) {
        if (str) {
            const wchar_t * stringsList[1] = { str };
            Pair<size_t, size_t> sizesList[1];
            sizesList[0].first = sizesList[0].second = wcslen(str);
            append(stringsList, sizesList, 1);
        }
    }

    void String::append(const char * LIBPLZMA_NULLABLE str) {
        if (str) {
            const char * stringsList[1] = { str };
            Pair<size_t, size_t> sizesList[1] = { lengthMaxCount(str, static_cast<size_t>(plzma_max_size())) };
            append(stringsList, sizesList, 1);
        }
    }
    
    String & String::operator = (String && str) noexcept {
        moveFrom(static_cast<String &&>(str));
        return *this;
    }

    String::String(String && str) noexcept {
        moveFrom(static_cast<String &&>(str));
    }
    
    String & String::operator = (const String & str) {
        copyFrom(str);
        return *this;
    }

    String::String(const String & str) {
        copyFrom(str);
    }
    
    String::String(const wchar_t * LIBPLZMA_NULLABLE str) {
        copyFrom(str);
    }
    
    String::String(const char * LIBPLZMA_NULLABLE str) {
        copyFrom(str);
    }
    
    size_t String::utf8TrailingBytes(const uint8_t c) noexcept {
        using namespace StringConvertUTF;
        
        return trailingBytesForUTF8[c];
    }

    Pair<size_t, size_t> String::lengthMaxCount(const char * LIBPLZMA_NULLABLE str, const size_t maxCount) noexcept {
        using namespace StringConvertUTF;
        
        if (str) {
            const UTF8 * utf8 = reinterpret_cast<const UTF8 *>(str);
            size_t length = 0, count = 0;
            while (*utf8 && count < maxCount) {
                int bytes = trailingBytesForUTF8[*utf8] + 1;
                count++;
                do {
                    length++;
                } while (*++utf8 && --bytes > 0);
            }
            return Pair<size_t, size_t>(length, count);
        }
        return Pair<size_t, size_t>(0, 0);
    }
    
    Pair<size_t, size_t> String::lengthMaxLength(const char * LIBPLZMA_NULLABLE str, const size_t maxLength) noexcept {
        using namespace StringConvertUTF;
        
        if (str) {
            const UTF8 * utf8 = reinterpret_cast<const UTF8 *>(str);
            size_t length = 0, count = 0;
            while (*utf8 && length < maxLength) {
                int bytes = trailingBytesForUTF8[*utf8] + 1;
                count++;
                do {
                    length++;
                } while (*++utf8 && --bytes > 0);
            }
            return Pair<size_t, size_t>(length, count);
        }
        return Pair<size_t, size_t>(0, 0);
    }

} // namespace plzma
