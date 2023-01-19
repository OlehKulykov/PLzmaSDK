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


#import <Foundation/Foundation.h>

FOUNDATION_EXPORT NSExceptionName const PLzmaSDKGenericExceptionName;


/// NSLocalizedDescriptionKey : exception.what()
FOUNDATION_EXPORT NSErrorUserInfoKey const PLzmaSDKExceptionCodeKey;
FOUNDATION_EXPORT NSErrorUserInfoKey const PLzmaSDKExceptionFileKey;
FOUNDATION_EXPORT NSErrorUserInfoKey const PLzmaSDKExceptionLineKey;


/// Limited to 32 bit unsigned integer.
typedef uint32_t PLzmaSDKSize;


/// Exception error codes.
typedef NS_ENUM(uint8_t, PLzmaSDKErrorCode) {

    /// The error type cannot be determined.
    ///
    /// Might be used as a default value during the initialization
    /// or generic, unknown exception was catched during the execution.
    PLzmaSDKErrorCodeUnknown = 0,
    
    /// Provided function or method arguments are invalid.
    PLzmaSDKErrorCodeInvalidArguments = 1,
    
    /// The required amount of memory can't be alocated or
    /// can't instantiate some object.
    PLzmaSDKErrorCodeNotEnoughMemory = 2,
    
    /// File or directory doesn't exists or there are no permissions to
    /// perform required action.
    PLzmaSDKErrorCodeIO = 3,
    
    /// Any internal errors or exceptions.
    PLzmaSDKErrorCodeInternal = 4
};


/// The type of erasing the content.
typedef NS_ENUM(uint8_t, PLzmaSDKErase) {
    
    /// The content will not be erased, i.e. ignored.
    PLzmaSDKEraseNone = 0,
    
    /// Erase, rewrite the content with zeros.
    PLzmaSDKEraseZero = 1
};


/// The type of the file, stream, data buffer, etc.
typedef NS_ENUM(uint8_t, PLzmaSDKFileType) {
    
    /// 7-zip type.
    ///
    /// This file type supports multiple archive items, password protected items list of the arhive and
    /// password protected content.
    /// - Note: Supports `LZMA`, `LZMA2` and `PPMd` compression methods.
    /// - Link: https://www.7-zip.org/7z.html
    PLzmaSDKFileType7z = 1,
    
    /// XZ type.
    ///
    /// This file type supports only one arhive item without password protection.
    /// - Note: Supports only `LZMA2` compression method which is automatically selected.
    /// - Link: https://www.7-zip.org/7z.html
    PLzmaSDKFileTypeXz = 2,
    
    /// TAR type.
    ///
    /// All archive items are combined and stored as one continuous stream without compression and without password protection.
    /// - Note: For this type, the `Method` is ignored.
    /// - Link: https://en.wikipedia.org/wiki/Tar_(computing)
    PLzmaSDKFileTypeTar = 3
};


/// Compression method.
typedef NS_ENUM(uint8_t, PLzmaSDKMethod) {

    /// Default and general compression method of 7z format.
    /// - Link: https://www.7-zip.org/7z.html
    PLzmaSDKMethodLZMA = 1,
    
    /// Improved version of `LZMA`.
    /// - Link: https://www.7-zip.org/7z.html
    PLzmaSDKMethodLZMA2 = 2,
    
    /// Dmitry Shkarin's `PPMdH` with small changes.
    /// - Link: https://www.7-zip.org/7z.html
    PLzmaSDKMethodPPMd = 3
};


/// The enumeration with bitmask options for opening directory path.
/// Currently uses for defining behavior of directory iteration.
typedef NS_OPTIONS(uint8_t, PLzmaSDKOpenDirMode) {
    
    /// Default behaviour, ignore any options.
    PLzmaSDKOpenDirModeDefault = 0,
    
    /// Follow the symbolic links.
    PLzmaSDKOpenDirModeFollowSymlinks = 1 << 0
};


typedef NS_ENUM(uint8_t, PLzmaSDKMultiStreamPartNameFormat) {

    /// "File"."Extension"."002". The maximum number of parts is 999.
    PLzmaSDKMultiStreamPartNameFormatNameExt00x = 1
};


/// The full version string of the library generated on build time.
///
/// Contains version<major, minor, patch> with optional automatic build number,
/// library type, build date/time, os, compiler, environment, usage, features, etc. and original LZMA SDK version.
FOUNDATION_EXPORT NSString * const PLzmaSDKVersion(void);


/// Get the current size in bytes of the stream's read block per single read request.
FOUNDATION_EXPORT PLzmaSDKSize PLzmaSDKGetStreamReadSize(void);


/// Set the current size in bytes of the stream's read block per single read request.
FOUNDATION_EXPORT void PLzmaSDKSetStreamReadSize(const PLzmaSDKSize size);


/// Get the current size in bytes of the stream's write block per single write request.
FOUNDATION_EXPORT PLzmaSDKSize PLzmaSDKGetStreamWriteSize(void);


/// Set the current size in bytes of the stream's write block per single write request.
FOUNDATION_EXPORT void PLzmaSDKSetStreamWriteSize(const PLzmaSDKSize size);


/// Get the current size in bytes of the decoder's internal buffer for holding decoded data.
FOUNDATION_EXPORT PLzmaSDKSize PLzmaSDKGetDecoderReadSize(void);


/// Set the current size in bytes of the decoder's internal buffer for holding decoded data.
FOUNDATION_EXPORT void PLzmaSDKSetDecoderReadSize(const PLzmaSDKSize size);


/// Get the current size in bytes of the decoder's internal buffer for holding decoded data.
FOUNDATION_EXPORT PLzmaSDKSize PLzmaSDKGetDecoderWriteSize(void);


/// Set the current size in bytes of the decoder's internal buffer for holding decoded data.
FOUNDATION_EXPORT void PLzmaSDKSetDecoderWriteSize(const PLzmaSDKSize size);
