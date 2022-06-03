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

#include "../libplzma.hpp"
#include "plzma_private.hpp"

#include <string.h>

namespace plzma {
    
    const char * LIBPLZMA_NULLABLE Exception::what() const noexcept {
        return _what;
    }
    
    const char * LIBPLZMA_NULLABLE Exception::file() const noexcept {
        return _file;
    }
    
    const char * LIBPLZMA_NULLABLE Exception::reason() const noexcept {
        return _reason;
    }
    
    plzma_error_code Exception::code() const noexcept {
        return _code;
    }
    
    int Exception::line() const noexcept {
        return _line;
    }
    
    void Exception::setWhat(const char * LIBPLZMA_NULLABLE what, ...) noexcept {
        if (what) {
            if (_what) {
                _what[0] = 0;
            }
            va_list list;
            va_start(list, what);
            do {
                _what = plzma_cstring_append(_what, what);
                what = va_arg(list, const char *);
            } while (what);
            va_end(list);
        } else {
            plzma_free(_what);
            _what = nullptr;
        }
    }
    
    void Exception::setReason(const char * LIBPLZMA_NULLABLE reason, ...) noexcept {
        if (reason) {
            if (_reason) {
                _reason[0] = 0;
            }
            va_list list;
            va_start(list, reason);
            do {
                _reason = plzma_cstring_append(_reason, reason);
                reason = va_arg(list, const char *);
            } while (reason);
            va_end(list);
        } else {
            plzma_free(_reason);
            _reason = nullptr;
        }
    }
    
    Exception::Exception(Exception && exception) noexcept:
        _what(exception._what),
        _file(exception._file),
        _reason(exception._reason),
        _code(exception._code),
        _line(exception._line) {
            exception._what = exception._file = exception._reason = nullptr;
    }
    
    Exception::Exception(const plzma_error_code code,
                         const char * LIBPLZMA_NULLABLE what,
                         const char * LIBPLZMA_NULLABLE file,
                         const int line) noexcept :
        _code(code),
        _line(line) {
            _what = plzma_cstring_copy(what);
            if (file) {
                const char * string = file, * lastPart = nullptr;
                while (*string) {
                    if (*string == '/' || *string == '\\') {
                        lastPart = ++string;
                    } else {
                        string++;
                    }
                }
                _file = plzma_cstring_copy(lastPart ? lastPart : file);
            }
    }
    
    Exception::~Exception() noexcept {
        plzma_free(_what);
        plzma_free(_file);
        plzma_free(_reason);
    }
    
    Exception * LIBPLZMA_NULLABLE Exception::moveToHeapCopy() const noexcept {
        try {
            Exception * exception = new Exception();
            exception->_what = _what;
            exception->_file = _file;
            exception->_reason = _reason;
            exception->_code = _code;
            exception->_line = _line;
            _what = _file = _reason = nullptr;
            return exception;
        } catch (...) {
            // no need -> nullable
        }
        return nullptr;
    }
    
    Exception * LIBPLZMA_NULLABLE Exception::create(const plzma_error_code code,
                                                    const char * LIBPLZMA_NULLABLE what,
                                                    const char * LIBPLZMA_NULLABLE file,
                                                    const int line) noexcept {
        try {
            return new Exception(code, what, file, line);
        } catch (...) {
            // no need -> nullable
        }
        return nullptr;
    }

} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

const char * LIBPLZMA_NULLABLE plzma_exception_what_utf8_string(plzma_exception_ptr LIBPLZMA_NONNULL exception) {
    return static_cast<Exception *>(exception)->what();
}

const char * LIBPLZMA_NULLABLE plzma_exception_file_utf8_string(plzma_exception_ptr LIBPLZMA_NONNULL exception) {
    return static_cast<Exception *>(exception)->file();
}

const char * LIBPLZMA_NULLABLE plzma_exception_reason_utf8_string(plzma_exception_ptr LIBPLZMA_NONNULL exception) {
    return static_cast<Exception *>(exception)->reason();
}

plzma_error_code plzma_exception_code(plzma_exception_ptr LIBPLZMA_NONNULL exception) {
    return static_cast<Exception *>(exception)->code();
}

int plzma_exception_line(plzma_exception_ptr LIBPLZMA_NONNULL exception) {
    return static_cast<Exception *>(exception)->line();
}

void plzma_exception_release(plzma_exception_ptr LIBPLZMA_NULLABLE exception) {
    delete static_cast<Exception *>(exception);
}

void plzma_object_exception_release(plzma_object * LIBPLZMA_NONNULL object) {
    delete static_cast<Exception *>(object->exception);
    object->exception = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
