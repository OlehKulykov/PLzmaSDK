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

@interface PLzmaSDKOutStream : NSObject

/// Checks the out stream is opened.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign, readonly) BOOL opened;


/// Copies the content of the stream to `Data`.
///
/// The stream must be closed.
/// - Returns: The `Data` with stream's content.
/// - Throws: `Exception` with `.notEnoughMemory` code in case if required amount of memory can't be allocated.
/// - Note: Thread-safe.
@property (nonatomic, strong, readonly, nonnull) NSData * copyContent;


/// Erases and removes the content of the stream.
/// - Parameter erase: The type of erasing the content.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) erase:(const PLzmaSDKErase) erase;


/// Initializes the output file stream with movable path.
/// - Parameter path: The non-empty input file path.
/// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
- (nonnull instancetype) initWithPath:(nonnull NSString *) path;


/// Initializes the output file stream object for writing to memory.
/// - Throws: `Exception`.
- (nonnull instancetype) init;

+ (nonnull instancetype) new NS_UNAVAILABLE;

@end
