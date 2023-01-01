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


#import "PLzmaSDKInStream.h"
#import "PLzmaSDKItem.h"
#import "PLzmaSDKOutStream.h"

@class PLzmaSDKDecoder;

/// The interface to a progress delegate of the decoder.
@protocol PLzmaSDKDecoderDelegate <NSObject>
@optional

/// Reports the progress of the operation.
/// - Parameter decoder: The decoder.
/// - Parameter path: The currently processing item's path.
/// - Parameter progress: The total progress of the operation.
- (void) onPLzmaSDKDecoder:(nonnull PLzmaSDKDecoder *) decoder
                      path:(nonnull NSString *) path
                  progress:(double) progress;

@end


@interface PLzmaSDKDecoder : NSObject

/// - Returns: Receives the number of items in archive.
/// - Note: The decoder must be opened.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign, readonly) PLzmaSDKSize count;


/// Receives all archive items.
/// - Returns: The array with all archive items.
/// - Note: The decoder must be opened.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, strong, readonly, nonnull) NSArray<PLzmaSDKItem *> * items;


/// Opens the archive.
///
/// During the process, the decoder is self-retained as long the operation is in progress.
/// - Returns: `true` the archive was successfully opened, otherwice `false`.
/// - Note: After successful opening, the input stream will be opened as long as a decoder exists.
/// - Note: The opening progress might be executed in a separate thread.
/// - Note: The opening progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) open;


/// Aborts opening, extracting or testing process.
/// - Note: The aborted decoder is no longer valid.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (void) abort;


/// Receives single archive item at a specific index.
/// - Parameter index: The index of the item inside the arhive. Must be less than the number of items reported by the `count()` method.
/// - Returns: The archive item.
/// - Note: The decoder must be opened.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (nonnull PLzmaSDKItem *) itemAt:(const PLzmaSDKSize) index;


/// Extracts all archive items to a specific path.
/// - Parameter path: The directory path to extract all items.
/// - Parameter itemsFullPath: Exctract item using it's full path or only last path component.
/// - Note: The extracting progress might be executed in a separate thread.
/// - Note: The extracting progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) extractToPath:(nonnull NSString *) path withItemsFullPath:(const BOOL) itemsFullPath;


/// Extracts some archive items to a specific path.
/// - Parameter items: The array of items to extract.
/// - Parameter path: The directory path to extract all items.
/// - Parameter itemsFullPath: Exctract item using it's full path or only the last path component.
/// - Note: The extracting progress might be executed in a separate thread.
/// - Note: The extracting progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) extractItems:(nonnull NSArray<PLzmaSDKItem *> *) items
               toPath:(nonnull NSString *) path
    withItemsFullPath:(const BOOL) itemsFullPath;


/// Extracts each archive item to a separate out-stream.
/// - Parameter items: The array with item/out-stream pairs.
/// - Note: The extracting progress might be executed in a separate thread.
/// - Note: The extracting progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) extractItemsToStreams:(nonnull NSDictionary<PLzmaSDKItem *, PLzmaSDKOutStream *> *) items;


/// Tests specific archive items.
/// - Parameter items: The array with items to test.
/// - Note: The testing progress might be executed in a separate thread.
/// - Note: The testing progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) testItems:(nonnull NSArray<PLzmaSDKItem *> *) items;


/// Tests all archive items.
/// - Note: The testing progress might be executed in a separate thread.
/// - Note: The testing progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) test;


/// Provides the archive password for opening, extracting or testing items.
/// - Parameter items password: The password.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (void) setPassword:(nullable NSString *) password;


/// Initializes the decoder.
/// - Parameter stream: The input stream with archive file content.
///                     After successful opening, the input stream will be opened as long as a decoder exists.
/// - Parameter fileType: The file type of the input stream.
/// - Parameter delegate: Optional delegate.
/// - Throws: `Exception`.
- (nonnull instancetype) initWithStream:(nonnull PLzmaSDKInStream *) stream
                               fileType:(const PLzmaSDKFileType) fileType
                            andDelegate:(nullable id<PLzmaSDKDecoderDelegate>) delegate;

- (nonnull instancetype) init NS_UNAVAILABLE;
+ (nonnull instancetype) new NS_UNAVAILABLE;

@end
