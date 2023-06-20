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


#include <memory>

#import "PLzmaSDKEncoder.h"
#import "PLzmaSDKEncoder.inl"
#import "PLzmaSDKInStream.inl"
#import "PLzmaSDKOutStream.inl"
#import "PLzmaSDKGlobal.inl"

@implementation PLzmaSDKEncoder

- (void) onCPath:(nullable const char *) cPath progress:(double) progress {
    id<PLzmaSDKEncoderDelegate> delegate = self.delegate;
    NSString * path = delegate ? [NSString stringWithUTF8String:cPath] : nil;
    if (path) {
        [delegate onPLzmaSDKEncoder:self path:path progress:progress];
    }
}

- (BOOL) shouldCreateSolidArchive {
    PLZMASDKOBJC_TRY
    return _encoder->shouldCreateSolidArchive();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldCreateSolidArchive:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldCreateSolidArchive(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (uint8_t) compressionLevel {
    PLZMASDKOBJC_TRY
    return _encoder->compressionLevel();
    PLZMASDKOBJC_CATCH_RETHROW
    return 0;
}

- (void) setCompressionLevel:(uint8_t) val {
    PLZMASDKOBJC_TRY
    _encoder->setCompressionLevel(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldCompressHeader {
    PLZMASDKOBJC_TRY
    return _encoder->shouldCompressHeader();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldCompressHeader:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldCompressHeader(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldCompressHeaderFull {
    PLZMASDKOBJC_TRY
    return _encoder->shouldCompressHeaderFull();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldCompressHeaderFull:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldCompressHeaderFull(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldEncryptContent {
    PLZMASDKOBJC_TRY
    return _encoder->shouldEncryptContent();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldEncryptContent:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldEncryptContent(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldEncryptHeader {
    PLZMASDKOBJC_TRY
    return _encoder->shouldEncryptHeader();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldEncryptHeader:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldEncryptHeader(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldStoreCreationTime {
    PLZMASDKOBJC_TRY
    return _encoder->shouldStoreCreationTime();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldStoreCreationTime:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldStoreCreationTime(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldStoreAccessTime {
    PLZMASDKOBJC_TRY
    return _encoder->shouldStoreAccessTime();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldStoreAccessTime:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldStoreAccessTime(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) shouldStoreModificationTime {
    PLZMASDKOBJC_TRY
    return _encoder->shouldStoreModificationTime();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setShouldStoreModificationTime:(BOOL) val {
    PLZMASDKOBJC_TRY
    _encoder->setShouldStoreModificationTime(val);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) open {
    PLZMASDKOBJC_TRY
    return _encoder->open();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) abort {
    PLZMASDKOBJC_TRY
    _encoder->abort();
    PLZMASDKOBJC_CATCH_RETHROW
}

- (BOOL) compress {
    PLZMASDKOBJC_TRY
    return _encoder->compress();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) addPath:(nonnull NSString *) path withMode:(const PLzmaSDKOpenDirMode) mode {
    PLZMASDKOBJC_TRY
    _encoder->add(plzma::Path(path.UTF8String), static_cast<const plzma_open_dir_mode_t>(mode));
    PLZMASDKOBJC_CATCH_RETHROW
}

- (void) addPath:(nonnull NSString *) path
        withMode:(const PLzmaSDKOpenDirMode) mode
  andArchivePath:(nonnull NSString *) archivePath {
    PLZMASDKOBJC_TRY
    _encoder->add(plzma::Path(path.UTF8String), static_cast<const plzma_open_dir_mode_t>(mode), plzma::Path(archivePath.UTF8String));
    PLZMASDKOBJC_CATCH_RETHROW
}

- (void) addStream:(nonnull PLzmaSDKInStream *) stream withArchivePath:(nonnull NSString *) archivePath {
    PLZMASDKOBJC_TRY
    const auto & inStream = *stream.inStreamSPtr;
    _encoder->add(inStream, plzma::Path(archivePath.UTF8String));
    PLZMASDKOBJC_CATCH_RETHROW
}

- (void) setPassword:(nonnull NSString *) password {
    PLZMASDKOBJC_TRY
    _encoder->setPassword(password.UTF8String);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (nonnull instancetype) initWithStream:(nonnull PLzmaSDKOutStream *) stream
                               fileType:(const PLzmaSDKFileType) fileType
                                 method:(const PLzmaSDKMethod) method
                            andDelegate:(nullable id<PLzmaSDKEncoderDelegate>) delegate {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        const auto & outStream = *stream.outStreamSPtr;
        if (stream.isMulti) {
            _encoder = plzma::makeSharedEncoder(outStream.cast<plzma::OutMultiStream>(), static_cast<const plzma_file_type>(fileType), static_cast<const plzma_method>(method));
        } else {
            _encoder = plzma::makeSharedEncoder(outStream, static_cast<const plzma_file_type>(fileType), static_cast<const plzma_method>(method));
        }
        if ([delegate respondsToSelector:@selector(onPLzmaSDKEncoder:path:progress:)]) {
            _progressDelegate = PLzmaSDKProgressDelegate::create((__bridge void *)self);
            _encoder->setProgressDelegate(_progressDelegate.get());
            self.delegate = delegate;
        }
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (void) dealloc {
    PLZMASDKOBJC_TRY
    _encoder->setProgressDelegate(nullptr);
    _progressDelegate.reset();
    _encoder.clear();
    PLZMASDKOBJC_CATCH_RETHROW
}

@end
