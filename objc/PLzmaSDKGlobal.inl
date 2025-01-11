//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2025 Oleh Kulykov <olehkulykov@gmail.com>
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


#include <exception>

#include "../libplzma.hpp"

FOUNDATION_EXPORT NSException * PLzmaSDKPLExceptionToNS(const plzma::Exception & exception);
FOUNDATION_EXPORT NSException * PLzmaSDKSTDExceptionToNS(const std::exception & exception);
FOUNDATION_EXPORT NSException * PLzmaSDKExceptionToNS(void);
FOUNDATION_EXPORT NSException * PLzmaSDKExceptionCreate(const PLzmaSDKErrorCode errorCode, NSString * what, NSString * reason, const char * file, const int line);

#define PLZMASDKOBJC_TRY \
try { \


#define PLZMASDKOBJC_CATCH_RETHROW \
} catch (const plzma::Exception & exception) { \
    [PLzmaSDKPLExceptionToNS(exception) raise]; \
} catch (const std::exception & exception) { \
    [PLzmaSDKSTDExceptionToNS(exception) raise]; \
} catch (...) { \
    [PLzmaSDKExceptionToNS() raise];\
} \

