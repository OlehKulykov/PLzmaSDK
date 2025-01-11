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

#import "PLzmaSDKOutMultiStream.h"
#import "PLzmaSDKOutStream.inl"
#import "PLzmaSDKGlobal.inl"

@implementation PLzmaSDKOutMultiStream

- (BOOL) isMulti {
    return YES;
}

- (nonnull NSArray<PLzmaSDKOutStream *> *) streams {
    PLZMASDKOBJC_TRY
    const auto outStream = _outStream.cast<plzma::OutMultiStream>();
    const auto streams = outStream->streams();
    const plzma_size_t streamsCount = streams.count();
    if (streamsCount > 0) {
        NSMutableArray * nsStreams = [[NSMutableArray alloc] initWithCapacity:streamsCount];
        for (plzma_size_t streamIndex = 0; streamIndex < streamsCount; streamIndex++) {
            auto stream = streams.at(streamIndex);
            [nsStreams addObject:[[PLzmaSDKOutStream alloc] initWithOutStreamM:&stream]];
        }
        return nsStreams;
    }
    PLZMASDKOBJC_CATCH_RETHROW
    return [NSArray array];
}

- (nonnull instancetype) initWithDirectoryPath:(nonnull NSString *) directoryPath
                                      partName:(nonnull NSString *) partName
                                 partExtension:(nullable NSString *) partExtension
                                        format:(const PLzmaSDKMultiStreamPartNameFormat) format
                                   andPartSize:(const PLzmaSDKSize) partSize {
    plzma::SharedPtr<plzma::OutStream> outStream;
    PLZMASDKOBJC_TRY
    const auto multiStream = plzma::makeSharedOutMultiStream(plzma::Path(directoryPath.UTF8String),
                                                             plzma::String(partName.UTF8String),
                                                             plzma::String(partExtension ? partExtension.UTF8String : nullptr),
                                                             static_cast<const plzma_plzma_multi_stream_part_name_format>(format),
                                                             partSize);
    outStream = multiStream.cast<plzma::OutStream>();
    PLZMASDKOBJC_CATCH_RETHROW
    return [super initWithOutStreamMTh:&outStream];
}

- (nonnull instancetype) initWithPartSize:(const PLzmaSDKSize) partSize {
    plzma::SharedPtr<plzma::OutStream> outStream;
    PLZMASDKOBJC_TRY
    const auto multiStream = plzma::makeSharedOutMultiStream(partSize);
    outStream = multiStream.cast<plzma::OutStream>();
    PLZMASDKOBJC_CATCH_RETHROW
    return [super initWithOutStreamMTh:&outStream];
}

@end
