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


#import "PLzmaSDKGlobal.h"

/// The input stream.
///
/// The stream could be initialized with path or memory data.
@interface PLzmaSDKInStream : NSObject


/// Checks the input file stream is opened.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign, readonly) BOOL opened;


/// Erases and removes the content of the stream.
/// - Parameter erase: The type of erasing the content.
/// - Returns: The erasing result.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) erase:(const PLzmaSDKErase) erase;


/// Initializes the input file stream with movable path.
/// - Parameter path: The non-empty input file path.
/// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
- (nonnull instancetype) initWithPath:(nonnull NSString *) path;


/// Initializes the input file stream with the file data.
/// During the creation, the data will copyed.
/// - Parameter dataCopy: The file data.
/// - Throws: `Exception` with `.notEnoughMemory` code in case if can't allocate required size of memory.
/// - Throws: `Exception` with `.invalidArguments` code in case if file data is empty.
- (nonnull instancetype) initWithDataCopy:(nonnull NSData *) dataCopy;


/// Initializes the input file stream with the file data.
/// During the creation and lifetime the data will not be copyed.
/// - Parameter dataNoCopy: The file data.
/// - Throws: `Exception` with `.invalidArguments` code in case if file data is empty.
/// - Throws: `Exception` with `.invalidArguments` code in case if file data size more than int64_max.
- (nonnull instancetype) initWithDataNoCopy:(nonnull NSData *) dataNoCopy;


/// Initializes multi input stream with an array of input streams.
/// The array should not be empty. The order: file.001, file.002, ..., file.XXX
/// - Parameter streams: The non-empty array of input streams. Each stream inside array should also exist.
/// - Note: The content of array will be moved to the newly created stream.
/// - Note: The array should not be empty.
/// - Throws: `Exception` with `.invalidArguments` code in case if streams array is empty or contains empty stream.
- (nonnull instancetype) initWithStreams:(nonnull NSArray<PLzmaSDKInStream *> *) streams;

- (nonnull instancetype) init NS_UNAVAILABLE;
+ (nonnull instancetype) new NS_UNAVAILABLE;

@end
