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


#import "PLzmaSDKGlobal.h"

/// The archive item.
@interface PLzmaSDKItem : NSObject


/// The item's path inside the archive.
/// - Throws: `Exception`.
@property (nonatomic, strong, readonly, nonnull) NSString * path;


/// The item's index inside the archive.
@property (nonatomic, assign, readonly) PLzmaSDKSize index;


/// The size in bytes of the item.
@property (nonatomic, assign) uint64_t size;


/// The packed size in bytes of the item.
@property (nonatomic, assign) uint64_t packSize;


/// The CRC-32 checksum of the item's content.
@property (nonatomic, assign) uint32_t crc32;


/// The creation date of the item. Based on unix timestamp.
@property (nonatomic, strong, nonnull) NSDate * creationDate;


/// The last access date of the item. Based on unix timestamp.
@property (nonatomic, strong, nonnull) NSDate * accessDate;


/// The last modification date of the item. Based on unix timestamp.
@property (nonatomic, strong, nonnull) NSDate * modificationDate;


/// The item is encrypted or not.
@property (nonatomic, assign) BOOL encrypted;


/// The item is directory or file.
@property (nonatomic, assign) BOOL isDir;


/// Initializes the item with path string and optional index inside the archive.
/// - Parameter path: The associated item's path.
/// - Parameter index: Optional index of the item inside the archive.
/// - Throws: `Exception`.
- (nonnull instancetype) initWithPath:(nonnull NSString *) path andIndex:(const PLzmaSDKSize) index;

- (nonnull instancetype) init NS_UNAVAILABLE;
+ (nonnull instancetype) new NS_UNAVAILABLE;

@end
