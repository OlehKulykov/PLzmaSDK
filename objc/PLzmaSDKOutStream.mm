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


#include <memory>

#import "PLzmaSDKOutStream.h"
#import "PLzmaSDKOutStream.inl"
#import "PLzmaSDKGlobal.inl"

@implementation PLzmaSDKOutStream

- (const plzma::SharedPtr<plzma::OutStream> *) outStreamSPtr {
    return &_outStream;
}

- (BOOL) isMulti {
    return NO;
}

- (BOOL) opened {
    PLZMASDKOBJC_TRY
    return _outStream->opened();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (nonnull NSData *) copyContent {
    PLZMASDKOBJC_TRY
    auto content = _outStream->copyContent();
    if (content.second > 0) {
        return [[NSData alloc] initWithBytesNoCopy:content.first.take() length:content.second deallocator:^(void * bytes, NSUInteger length) {
            (void)length;
            plzma_free(bytes);
        }];
    }
    PLZMASDKOBJC_CATCH_RETHROW
    return [NSData data];
}

- (BOOL) erase:(const PLzmaSDKErase) erase {
    PLZMASDKOBJC_TRY
    return _outStream->erase(static_cast<const plzma_erase>(erase));
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (instancetype) initWithOutStreamM:(plzma::SharedPtr<plzma::OutStream> *) outStream {
    self = [super init];
    if (self) {
        _outStream = std::move(*outStream);
    }
    return self;
}

- (instancetype) initWithOutStreamMTh:(plzma::SharedPtr<plzma::OutStream> *) outStream {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        _outStream = std::move(*outStream);
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (nonnull instancetype) initWithPath:(nonnull NSString *) path {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        _outStream = plzma::makeSharedOutStream(plzma::Path(path.UTF8String));
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (nonnull instancetype) init {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        _outStream = plzma::makeSharedOutStream();
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (void) dealloc {
    PLZMASDKOBJC_TRY
    _outStream.clear();
    PLZMASDKOBJC_CATCH_RETHROW
}

@end
