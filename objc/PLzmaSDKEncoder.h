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


#import "PLzmaSDKInStream.h"
#import "PLzmaSDKOutStream.h"
#import "PLzmaSDKOutMultiStream.h"

@class PLzmaSDKEncoder;

/// The interface to a progress delegate of the encoder.
@protocol PLzmaSDKEncoderDelegate <NSObject>
@optional

/// Reports the progress of the operation.
/// - Parameter encoder: The encoder.
/// - Parameter path: The currently processing item's path.
/// - Parameter progress: The total progress of the operation.
- (void) onPLzmaSDKEncoder:(nonnull PLzmaSDKEncoder *) encoder
                      path:(nonnull NSString *) path
                  progress:(double) progress;
@end


@interface PLzmaSDKEncoder : NSObject

/// Getter/setter for a 'solid' archive property.
/// - Note: Thread-safe. Must be set before opening.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldCreateSolidArchive;


/// Getter/setter for a compression level.
/// - Parameter level: The level in a range [0; 9].
/// - Returns: The level in a range [0; 9].
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
@property (nonatomic, assign) uint8_t compressionLevel;


/// Should encoder compress the archive header.
/// - Note: Thread-safe. Must be set before opening.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldCompressHeader;


/// Should encoder fully compress the archive header.
/// - Note: Thread-safe. Must be set before opening.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldCompressHeaderFull;


/// Should encoder encrypt the content of the archive items.
///
/// The encryption will take place only if this option enabled, the type supports password protection
/// and the password has been provided.
/// - SeeAlso: `setPassword` method and `FileType` enum.
/// - Note: The password will be required to decode/extract archive items.
/// - Note: Thread-safe. Must be set before opening.
/// - Note: The password will be required to decode/extract archive items.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldEncryptContent;


/// Should encoder encrypt the header with the list of archive items.
/// The encryption will take place only if this option enabled, the type supports password protection
/// and the password has been provided.
/// - SeeAlso: `setPassword` method and `FileType` enum.
/// - Note: The password will be required to open archive and list the items.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldEncryptHeader;


/// Should encoder store the creation time of each item to the header if such available.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldStoreCreationTime;


/// Should encoder store the access time of each item to the header if such available.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldStoreAccessTime;


/// Should encoder store the last modification time of each item to the header if such available.
/// - Note: Enabled by default, the value is `true`.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
@property (nonatomic, assign) BOOL shouldStoreModificationTime;


/// Opens the encoder for compressing.
///
/// During the process, the encoder is self-retained as long the operation is in progress.
/// - Returns: `false` if nothing to compress or encoder aborted or incorrect number of items or number of items greater than supported.
/// otherwise `true`.
/// - Note: The opening progress might be executed in a separate thread.
/// - Note: The opening progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) open;


/// Aborts the opening or compressing process.
/// - Note: The aborted encoder is no longer valid.
/// - Throws: `Exception`.
- (void) abort;


/// Compresses the provided paths and streams.
///
/// During the process, the encoder is self-retained as long the operation is in progress.
/// - Note: The compress progress might be executed in a separate thread.
/// - Note: The compress progress might be aborted via `abort()` method.
/// - Note: Thread-safe.
/// - Throws: `Exception`.
- (BOOL) compress;


/// Adds the physical file or directory path to the encoder.
/// - Parameter path: The file or directory path.
/// - Parameter mode: The mode for opening directory in case if `path` is a directory path.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
- (void) addPath:(nonnull NSString *) path withMode:(const PLzmaSDKOpenDirMode) mode;


/// Adds the physical file or directory path to the encoder.
/// - Parameter path: The uniq. file or directory path. Duplicated value is not allowed.
/// - Parameter mode: The mode for opening directory in case if `path` is a directory path.
/// - Parameter archivePath: The custom path of how the `path` will be presented in archive.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
- (void) addPath:(nonnull NSString *) path
        withMode:(const PLzmaSDKOpenDirMode) mode
  andArchivePath:(nonnull NSString *) archivePath;


/// Adds the file in-sctream to the encoder.
/// - Parameter stream: The input file stream to add. Empty stream is not allowed.
/// - Parameter archivePath: The custom path of how the `path` will be presented in archive. Empty path is not allowed.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
- (void) addStream:(nonnull PLzmaSDKInStream *) stream withArchivePath:(nonnull NSString *) archivePath;


/// Provides the password for archive.
///
/// This password will be used for encrypting header and the content if such options are enabled
/// and selected type supports password protection.
/// - SeeAlso: `setShouldEncryptHeader`, `setShouldEncryptContent` methods and `FileType` enum.
/// - Parameter password: The password. nil or zero length password means no password provided.
/// - Note: Thread-safe. Must be set before opening.
/// - Throws: `Exception`.
- (void) setPassword:(nonnull NSString *) password;


/// Initializes the encoder with output file stream.
/// - Parameter stream: The output file stream to write the archive's file content.
/// - Parameter fileType: The type of the archive.
/// - Parameter method: The compresion method.
/// - Parameter delegate: Optional delegate.
/// - Throws: `Exception` with `.invalidArguments` code in case if provided stream is empty.
- (nonnull instancetype) initWithStream:(nonnull PLzmaSDKOutStream *) stream
                               fileType:(const PLzmaSDKFileType) fileType
                                 method:(const PLzmaSDKMethod) method
                            andDelegate:(nullable id<PLzmaSDKEncoderDelegate>) delegate;

- (nonnull instancetype) init NS_UNAVAILABLE;
+ (nonnull instancetype) new NS_UNAVAILABLE;

@end
