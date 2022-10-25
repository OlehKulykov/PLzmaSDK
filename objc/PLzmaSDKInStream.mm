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


#import "PLzmaSDKInStream.h"
#import "PLzmaSDKInStream.inl"
#import "PLzmaSDKGlobal.inl"

@interface PLzmaSDKInStreamDataNoCopyCtx : NSObject {
@public
    NSData * data;
    int64_t offset;
    int64_t length;
}

+ (nonnull instancetype) new NS_UNAVAILABLE;

@end

@implementation PLzmaSDKInStreamDataNoCopyCtx

@end

static void PLzmaSDKInStreamDataNoCopyCtxDeinit(void * LIBPLZMA_NONNULL context) {
    PLzmaSDKInStreamDataNoCopyCtx * ctx = (PLzmaSDKInStreamDataNoCopyCtx *)CFBridgingRelease(context);
    ctx->data = nil;
}

static bool PLzmaSDKInStreamDataNoCopyOpen(void * LIBPLZMA_NULLABLE context) {
    PLzmaSDKInStreamDataNoCopyCtx * ctx = context ? (__bridge PLzmaSDKInStreamDataNoCopyCtx *)context : nil;
    if (ctx) {
        ctx->offset = 0;
        return true;
    }
    return false;
}

static void PLzmaSDKInStreamDataNoCopyClose(void * LIBPLZMA_NULLABLE context) {
    PLzmaSDKInStreamDataNoCopyCtx * ctx = context ? (__bridge PLzmaSDKInStreamDataNoCopyCtx *)context : nil;
    if (ctx) {
        ctx->offset = 0;
    }
}

static bool PLzmaSDKInStreamDataNoCopySeek(void * LIBPLZMA_NULLABLE context,
                                           int64_t offset,
                                           uint32_t seek_origin,
                                           uint64_t * LIBPLZMA_NONNULL new_position) {
    PLzmaSDKInStreamDataNoCopyCtx * ctx = context ? (__bridge PLzmaSDKInStreamDataNoCopyCtx *)context : nil;
    if (ctx) {
        int64_t finalOffset;
        switch (seek_origin) {
            case SEEK_SET:
                finalOffset = offset;
                break;
            case SEEK_CUR:
                finalOffset = ctx->offset + offset;
                break;
            case SEEK_END:
                finalOffset = ctx->length + offset;
                break;
            default:
                finalOffset = -1;
                break;
        }
        if ((finalOffset >= 0) && (finalOffset <= ctx->length)) {
            ctx->offset = finalOffset;
            *new_position = static_cast<uint64_t>(finalOffset);
            return true;
        }
        ctx->offset = 0;
        *new_position = 0;
    }
    return false;
}

static bool PLzmaSDKInStreamDataNoCopyRead(void * LIBPLZMA_NULLABLE context,
                                           void * LIBPLZMA_NONNULL data,
                                           uint32_t size,
                                           uint32_t * LIBPLZMA_NONNULL processed_size) {
    PLzmaSDKInStreamDataNoCopyCtx * ctx = context ? (__bridge PLzmaSDKInStreamDataNoCopyCtx *)context : nil;
    if (ctx) {
        const int64_t available = ctx->length - ctx->offset;
        int64_t sizeToRead = 0;
        if (available > 0) {
            sizeToRead = (size <= available) ? static_cast<int64_t>(size) : available;
            [ctx->data getBytes:data range:NSMakeRange(ctx->offset, sizeToRead)];
            ctx->offset += sizeToRead;
        }
        *processed_size = static_cast<uint32_t>(sizeToRead);
        return true;
    }
    return false;
}

@implementation PLzmaSDKInStream

- (const plzma::SharedPtr<plzma::InStream> *) inStreamSPtr {
    return &_inStream;
}

- (BOOL) opened {
    PLZMASDKOBJC_TRY
    return _inStream->opened();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (BOOL) erase:(const PLzmaSDKErase) erase {
    PLZMASDKOBJC_TRY
    return _inStream->erase(static_cast<plzma_erase>(erase));
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (nonnull instancetype) initWithPath:(nonnull NSString *) path {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        _inStream = plzma::makeSharedInStream(plzma::Path(path.UTF8String));
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (nonnull instancetype) initWithDataCopy:(nonnull NSData *) dataCopy {
    self = [super init];
    if (self) {
        const NSUInteger length = dataCopy.length;
        const void * bytes = (length > 0) ? dataCopy.bytes : NULL;
        if (bytes) {
            PLZMASDKOBJC_TRY
            _inStream = plzma::makeSharedInStream(bytes, length);
            PLZMASDKOBJC_CATCH_RETHROW
        } else {
            [PLzmaSDKExceptionCreate(PLzmaSDKErrorCodeInvalidArguments, @"Can't instantiate in-stream without memory.", @"The memory is null.", __FILE__, __LINE__) raise];
        }
    }
    return self;
}

- (nonnull instancetype) initWithDataNoCopy:(nonnull NSData *) dataNoCopy {
    self = [super init];
    if (self) {
        const NSUInteger length = dataNoCopy.length;
        if (length > 0) {
            if (length > INT64_MAX) {
                [PLzmaSDKExceptionCreate(PLzmaSDKErrorCodeInvalidArguments, @"Can't instantiate in-stream.", @"The memory is bigger as supported(int64_max).", __FILE__, __LINE__) raise];
            }
            PLzmaSDKInStreamDataNoCopyCtx * ctx = [[PLzmaSDKInStreamDataNoCopyCtx alloc] init];
            ctx->data = dataNoCopy;
            ctx->length = static_cast<int64_t>(length);
            plzma_context context;
            context.context = (void *)CFBridgingRetain(ctx);
            context.deinitializer = PLzmaSDKInStreamDataNoCopyCtxDeinit;
            PLZMASDKOBJC_TRY
            _inStream = plzma::makeSharedInStream(PLzmaSDKInStreamDataNoCopyOpen,
                                                  PLzmaSDKInStreamDataNoCopyClose,
                                                  PLzmaSDKInStreamDataNoCopySeek,
                                                  PLzmaSDKInStreamDataNoCopyRead,
                                                  context);
            PLZMASDKOBJC_CATCH_RETHROW
        } else {
            [PLzmaSDKExceptionCreate(PLzmaSDKErrorCodeInvalidArguments, @"Can't instantiate in-stream without memory.", @"The memory is zero.", __FILE__, __LINE__) raise];
        }
    }
    return self;
}

- (nonnull instancetype) initWithStreams:(nonnull NSArray<PLzmaSDKInStream *> *) streams {
    self = [super init];
    if (self) {
        const NSUInteger count = streams.count;
        if ((count == 0) || (count > PLZMA_SIZE_T_MAX)) {
            [PLzmaSDKExceptionCreate(PLzmaSDKErrorCodeInvalidArguments, @"Invalid number of streams.", @"The number should be positive and less than maximum supported size.", __FILE__, __LINE__) raise];
        }
        PLZMASDKOBJC_TRY
        plzma::InStreamArray inStreams(static_cast<const plzma_size_t>(count));
        for (PLzmaSDKInStream * inStream in streams) {
            const auto & inStreamSPtr = *inStream.inStreamSPtr;
            inStreams.push(inStreamSPtr);
        }
        _inStream = plzma::makeSharedInStream(std::move(inStreams));
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (void) dealloc {
    PLZMASDKOBJC_TRY
    _inStream.clear();
    PLZMASDKOBJC_CATCH_RETHROW
}

@end
