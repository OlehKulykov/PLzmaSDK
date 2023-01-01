//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2023 Oleh Kulykov <olehkulykov@gmail.com>
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


#import <Foundation/Foundation.h>

#import "PLzmaSDKProgressDelegate.inl"

void PLzmaSDKProgressDelegate::onProgress(void * LIBPLZMA_NULLABLE context, const plzma::String & path, const double progress) {
    const auto strongSelf = _weakSelf.lock();
    id<PLzmaSDKProgressDelegatePrivate> delegatePrivate = strongSelf ? (__bridge id<PLzmaSDKProgressDelegatePrivate>)strongSelf->_delegate : nil;
    if (delegatePrivate) {
        [delegatePrivate onCPath:path.utf8() progress:progress];
    }
}

std::shared_ptr<PLzmaSDKProgressDelegate> PLzmaSDKProgressDelegate::create(void * LIBPLZMA_NULLABLE decoder) {
    auto delegate = std::make_shared<PLzmaSDKProgressDelegate>(decoder);
    delegate->_weakSelf = delegate;
    return delegate;
}
