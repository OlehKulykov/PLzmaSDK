/*
 * By using this Software, you are accepting original [LZMA SDK] and MIT license below:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 - 2024 Oleh Kulykov <olehkulykov@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef __PLZMA_PUBLIC_TESTS_HPP__
#define __PLZMA_PUBLIC_TESTS_HPP__

#if defined(CMAKE_BUILD)
#undef CMAKE_BUILD
#endif

#if defined(LIBPLZMA_BUILD)
#undef LIBPLZMA_BUILD
#endif

#include <iostream>
#include <memory>
#include <cassert>
#include <cstring>

#include <libplzma.hpp>
#include <libplzma.h>

#include "../src/plzma_private.hpp"
#include "../src/plzma_common.hpp"
#include "../src/plzma_in_streams.hpp"
#include "../src/plzma_out_streams.hpp"
#include "../src/plzma_extract_callback.hpp"

#if defined(CMAKE_BUILD)
#undef CMAKE_BUILD
#endif

#if defined(LIBPLZMA_BUILD)
#undef LIBPLZMA_BUILD
#endif

#define PLZMA_TESTS_STRINGIFY(x) #x
#define PLZMA_TESTS_TOSTRING(x) PLZMA_TESTS_STRINGIFY(x)

#define PLZMA_TESTS_ASSERT(V) if(!(V)){ \
std::flush(std::cout) << "\'" << PLZMA_TESTS_TOSTRING(V) << "\' Failed at line: " << __LINE__ << std::endl; \
assert(0); \
return __LINE__; \
} \


#endif // !__PLZMA_PUBLIC_TESTS_HPP__
