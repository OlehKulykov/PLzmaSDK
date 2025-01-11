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

#include "../libplzma.hpp"

@protocol PLzmaSDKProgressDelegatePrivate <NSObject>
@required

- (void) onCPath:(nullable const char *) cPath progress:(double) progress;

@end

class PLzmaSDKProgressDelegate final : public plzma::ProgressDelegate {
private:
    std::weak_ptr<PLzmaSDKProgressDelegate> _weakSelf;
    void * LIBPLZMA_NULLABLE _delegate = nullptr;
    
public:
    virtual void onProgress(void * LIBPLZMA_NULLABLE context, const plzma::String & path, const double progress);
    
    PLzmaSDKProgressDelegate(void * LIBPLZMA_NULLABLE delegate) noexcept : _delegate(delegate) { }
    virtual ~PLzmaSDKProgressDelegate() noexcept = default;
    
    static std::shared_ptr<PLzmaSDKProgressDelegate> create(void * LIBPLZMA_NULLABLE decoder);
};
