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

namespace plzma {
    
    RawHeapMemory & RawHeapMemory::operator = (RawHeapMemory && memory) noexcept {
        plzma_free(_memory);
        _memory = memory._memory;
        memory._memory = nullptr;
        return *this;
    }
    
    void * LIBPLZMA_NULLABLE RawHeapMemory::take() noexcept {
        void * memory = _memory;
        _memory = nullptr;
        return memory;
    }

    void RawHeapMemory::resize(const size_t size) {
        if (size > 0) {
            void * memory = _memory ? plzma_realloc(_memory, size) : plzma_malloc(size);
            if (memory) {
                _memory = memory;
            } else {
                throw Exception(plzma_error_code_not_enough_memory, nullptr, __FILE__, __LINE__);
            }
        } else {
            plzma_free(_memory);
            _memory = nullptr;
        }
    }
    
    void RawHeapMemory::erase(const plzma_erase eraseType, const size_t size) noexcept {
        if (_memory && size > 0 && eraseType == plzma_erase_zero) {
            memset(_memory, 0, size);
        }
    }
    
    void RawHeapMemory::clear(const plzma_erase eraseType /* = plzma_erase_none */,
                              const size_t size /* = 0 */) noexcept {
        erase(eraseType, size);
        plzma_free(_memory);
        _memory = nullptr;
    }
    
    RawHeapMemory::RawHeapMemory(const size_t size, const plzma_erase eraseType /* = plzma_erase_none */) {
        if (size > 0) {
            if ( !(_memory = plzma_malloc(size)) ) {
                throw Exception(plzma_error_code_not_enough_memory, nullptr, __FILE__, __LINE__);
            }
            erase(eraseType, size);
        }
    }
    
    RawHeapMemory::RawHeapMemory(RawHeapMemory && memory) noexcept :
        _memory(memory._memory) {
            memory._memory = nullptr;
    }
    
    RawHeapMemory::~RawHeapMemory() noexcept {
        plzma_free(_memory);
    }
    
} // namespace plzma
