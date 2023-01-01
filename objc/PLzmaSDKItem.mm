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


#import "PLzmaSDKItem.h"
#import "PLzmaSDKItem.inl"
#import "PLzmaSDKGlobal.inl"

@implementation PLzmaSDKItem

- (const plzma::SharedPtr<plzma::Item> *) itemSPtr {
    return &_item;
}

- (nonnull NSString *) path {
    PLZMASDKOBJC_TRY
    NSString * nsStr = [NSString stringWithUTF8String:_item->path().utf8()];
    if (nsStr) {
        return nsStr;
    }
    PLZMASDKOBJC_CATCH_RETHROW
    return [NSString string];
}

- (PLzmaSDKSize) index {
    return _item->index();
}

- (uint64_t) size {
    return _item->size();
}

- (uint64_t) packSize {
    return _item->packSize();
}

- (uint32_t) crc32 {
    return _item->crc32();
}

- (nonnull NSDate *) creationDate {
    return [NSDate dateWithTimeIntervalSince1970:_item->creationTime()];
}

- (nonnull NSDate *) accessDate {
    return [NSDate dateWithTimeIntervalSince1970:_item->accessTime()];
}

- (nonnull NSDate *) modificationDate {
    return [NSDate dateWithTimeIntervalSince1970:_item->accessTime()];
}

- (BOOL) encrypted {
    return _item->encrypted();
}

- (BOOL) isDir {
    return _item->isDir();
}

- (NSUInteger) hash {
    return _item->index();
}

- (BOOL) isEqual:(id) object {
    PLzmaSDKItem * item = (PLzmaSDKItem *)object;
    return [item isKindOfClass:[PLzmaSDKItem class]] ? (item->_item->index() == _item->index()) : [super isEqual:object];
}

- (nonnull instancetype) initWithPath:(nonnull NSString *) path andIndex:(const PLzmaSDKSize) index {
    self = [super init];
    if (self) {
        PLZMASDKOBJC_TRY
        _item = plzma::makeShared<plzma::Item>(plzma::Path(path.UTF8String), index);
        PLZMASDKOBJC_CATCH_RETHROW
    }
    return self;
}

- (instancetype) initWithItemM:(plzma::SharedPtr<plzma::Item> *) item {
    self = [super init];
    if (self) {
        _item = std::move(*item);
    }
    return self;
}

- (void) dealloc {
    PLZMASDKOBJC_TRY
    _item.clear();
    PLZMASDKOBJC_CATCH_RETHROW
}

#if defined(DEBUG)
- (NSString *) debugDescription {
    return [NSString stringWithFormat:@"%@ [Path: %@ \nIndex: %lu\nSize: %llu\nPack size: %llu\nCRC32: %lu\nCreation date: %@\nAccess date: %@\nModification date: %@\nEncrypted: %@\nIs dir: %@]",
            NSStringFromClass(self.class),
            self.path,
            (unsigned long)self.index,
            (unsigned long long)self.size,
            (unsigned long long)self.packSize,
            (unsigned long)self.crc32,
            self.creationDate,
            self.accessDate,
            self.modificationDate,
            self.encrypted ? @"YES" : @"NO",
            self.isDir ? @"YES" : @"NO"];
}

- (NSString *) description {
    return self.debugDescription;
}
#endif

@end
