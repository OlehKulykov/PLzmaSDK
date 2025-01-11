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

#import "PLzmaSDKDecoder.h"
#import "PLzmaSDKDecoder.inl"
#import "PLzmaSDKInStream.inl"
#import "PLzmaSDKOutStream.inl"
#import "PLzmaSDKItem.inl"
#import "PLzmaSDKGlobal.inl"

@implementation PLzmaSDKDecoder

- (void) onCPath:(nullable const char *) cPath progress:(double) progress {
    id<PLzmaSDKDecoderDelegate> delegate = self.delegate;
    NSString * path = delegate ? [NSString stringWithUTF8String:cPath] : nil;
    if (path) {
        [delegate onPLzmaSDKDecoder:self path:path progress:progress];
    }
}

- (PLzmaSDKSize) count {
    PLZMASDKOBJC_TRY
    return _decoder->count();
    PLZMASDKOBJC_CATCH_RETHROW
    return 0;
}

- (nonnull NSArray<PLzmaSDKItem *> *) items {
    PLZMASDKOBJC_TRY
    auto itemsArray = _decoder->items();
    if (itemsArray) {
        const plzma_size_t itemsCount = itemsArray->count();
        NSMutableArray * items = [[NSMutableArray alloc] initWithCapacity:itemsCount];
        for (plzma_size_t itemIndex = 0; itemIndex < itemsCount; itemIndex++) {
            [items addObject:[[PLzmaSDKItem alloc] initWithItemM:&itemsArray->at(itemIndex)]];
        }
        return items;
    }
    PLZMASDKOBJC_CATCH_RETHROW
    return [NSArray array];
}

- (BOOL) open {
    PLZMASDKOBJC_TRY
    return _decoder->open();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) abort {
    PLZMASDKOBJC_TRY
    _decoder->abort();
    PLZMASDKOBJC_CATCH_RETHROW
}

- (nonnull PLzmaSDKItem *) itemAt:(const PLzmaSDKSize) index {
    PLzmaSDKItem * nsItem = nil;
    PLZMASDKOBJC_TRY
    auto item = _decoder->itemAt(index);
    nsItem = [[PLzmaSDKItem alloc] initWithItemM:&item];
    PLZMASDKOBJC_CATCH_RETHROW
    return nsItem;
}

- (BOOL) extractToPath:(nonnull NSString *) path withItemsFullPath:(const BOOL) itemsFullPath {
    PLZMASDKOBJC_TRY
    return _decoder->extract(plzma::Path(path.UTF8String), static_cast<bool>(itemsFullPath));
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (BOOL) extractItems:(nonnull NSArray<PLzmaSDKItem *> *) items
               toPath:(nonnull NSString *) path
    withItemsFullPath:(const BOOL) itemsFullPath {
    PLZMASDKOBJC_TRY
    auto itemsArray = plzma::makeShared<plzma::ItemArray>(static_cast<plzma_size_t>(items.count));
    for (PLzmaSDKItem * nsItem in items) {
        auto item = *nsItem.itemSPtr;
        itemsArray->push(std::move(item));
    }
    return _decoder->extract(itemsArray, plzma::Path(path.UTF8String), static_cast<bool>(itemsFullPath));
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (BOOL) extractItemsToStreams:(nonnull NSDictionary<PLzmaSDKItem *, PLzmaSDKOutStream *> *) items {
    PLZMASDKOBJC_TRY
    auto itemOutStreamArray = plzma::makeShared<plzma::ItemOutStreamArray>(static_cast<plzma_size_t>(items.count));
    plzma::ItemOutStreamArray * arrayPtr = itemOutStreamArray.get();
    [items enumerateKeysAndObjectsUsingBlock:^(PLzmaSDKItem * _Nonnull key, PLzmaSDKOutStream * _Nonnull obj, BOOL * _Nonnull stop) {
        auto item = *key.itemSPtr;
        auto stream = *obj.outStreamSPtr;
        arrayPtr->push(plzma::Pair<plzma::SharedPtr<plzma::Item>, plzma::SharedPtr<plzma::OutStream> >(std::move(item), std::move(stream)));
    }];
    return _decoder->extract(itemOutStreamArray);
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (BOOL) testItems:(nonnull NSArray<PLzmaSDKItem *> *) items {
    PLZMASDKOBJC_TRY
    auto itemsArray = plzma::makeShared<plzma::ItemArray>(static_cast<plzma_size_t>(items.count));
    for (PLzmaSDKItem * nsItem in items) {
        auto item = *nsItem.itemSPtr;
        itemsArray->push(std::move(item));
    }
    return _decoder->test(itemsArray);
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (BOOL) test {
    PLZMASDKOBJC_TRY
    return _decoder->test();
    PLZMASDKOBJC_CATCH_RETHROW
    return NO;
}

- (void) setPassword:(nullable NSString *) password {
    PLZMASDKOBJC_TRY
    _decoder->setPassword(password.UTF8String);
    PLZMASDKOBJC_CATCH_RETHROW
}

- (nonnull instancetype) initWithStream:(nonnull PLzmaSDKInStream *) stream
                               fileType:(const PLzmaSDKFileType) fileType
                            andDelegate:(nullable id<PLzmaSDKDecoderDelegate>) delegate {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        const auto & inStream = *stream.inStreamSPtr;
        _decoder = plzma::makeSharedDecoder(inStream, static_cast<const plzma_file_type>(fileType));
        if ([delegate respondsToSelector:@selector(onPLzmaSDKDecoder:path:progress:)]) {
            _progressDelegate = PLzmaSDKProgressDelegate::create((__bridge void *)self);
            _decoder->setProgressDelegate(_progressDelegate.get());
            self.delegate = delegate;
        }
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (void) dealloc {
    PLZMASDKOBJC_TRY
    _decoder->setProgressDelegate(nullptr);
    _progressDelegate.reset();
    _decoder.clear();
    PLZMASDKOBJC_CATCH_RETHROW
}

@end
