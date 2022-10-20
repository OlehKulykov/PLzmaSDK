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


import Foundation
#if SWIFT_PACKAGE
import libplzma
#endif

/// Limited to 32 bit unsigned integer.
public typealias Size = plzma_size_t

public protocol Enum {
    
    associatedtype EType: RawRepresentable
    
    var type: EType { get }
}

extension Enum where Self: RawRepresentable, Self.RawValue: BinaryInteger, EType.RawValue: BinaryInteger {
   
    public var type: EType {
        return EType.init(rawValue: EType.RawValue(self.rawValue))!
    }
}

/// Exception error codes.
public enum ErrorCode: UInt8, Enum {
    
    public typealias EType = plzma_error_code
    
    /// The error type cannot be determined.
    ///
    /// Might be used as a default value during the initialization
    /// or generic, unknown exception was catched during the execution.
    case unknown = 0
    
    /// Provided function or method arguments are invalid.
    case invalidArguments = 1
    
    /// The required amount of memory can't be alocated or
    /// can't instantiate some object.
    case notEnoughMemory = 2
    
    /// File or directory doesn't exists or there are no permissions to
    /// perform required action.
    case io = 3
    
    /// Any internal errors or exceptions.
    case `internal` = 4
}

extension plzma_error_code: Enum {
    
    public typealias EType = ErrorCode
}

/// The type of erasing the content.
public enum Erase: UInt8, Enum {
    
    public typealias EType = plzma_erase
    
    /// The content will not be erased, i.e. ignored.
    case none = 0
    
    /// Erase, rewrite the content with zeros.
    case zero = 1
}

extension plzma_erase: Enum {
    
    public typealias EType = Erase
}

/// The type of the file, stream, data buffer, etc.
public enum FileType: UInt8, Enum {
    
    public typealias EType = plzma_file_type
    
    /// 7-zip type.
    ///
    /// This file type supports multiple archive items, password protected items list of the arhive and
    /// password protected content.
    /// - Note: Supports `LZMA`, `LZMA2` and `PPMd` compression methods.
    /// - Link: https://www.7-zip.org/7z.html 
    case sevenZ = 1 // 7z
    
    /// XZ type.
    ///
    /// This file type supports only one arhive item without password protection.
    /// - Note: Supports only `LZMA2` compression method which is automatically selected.
    /// - Link: https://www.7-zip.org/7z.html
    case xz = 2
    
    /// TAR type.
    ///
    /// All archive items are combined and stored as one continuous stream without compression and without password protection.
    /// - Note: For this type, the `Method` is ignored.
    /// - Link: https://en.wikipedia.org/wiki/Tar_(computing)
    case tar = 3
}

extension plzma_file_type: Enum {
    
    public typealias EType = FileType
}

/// Compression method.
public enum Method: UInt8, Enum {

    public typealias EType = plzma_method
    
    /// Default and general compression method of 7z format.
    /// - Link: https://www.7-zip.org/7z.html
    case LZMA = 1
    
    /// Improved version of `LZMA`.
    /// - Link: https://www.7-zip.org/7z.html
    case LZMA2 = 2
    
    /// Dmitry Shkarin's `PPMdH` with small changes.
    /// - Link: https://www.7-zip.org/7z.html
    case PPMd = 3
}

extension plzma_method: Enum {
    
    public typealias EType = Method
}

/// The enumeration with bitmask options for opening directory path.
/// Currently uses for defining behavior of directory iteration.
public struct OpenDirMode: OptionSet {
    public typealias RawValue = plzma_open_dir_mode_t
    
    public let rawValue: plzma_open_dir_mode_t
    
    public init(rawValue: plzma_open_dir_mode_t) {
        self.rawValue = rawValue
    }
    
    /// Default behaviour, ignore any options.
    public static let `default` = OpenDirMode([])
    
    /// Follow the symbolic links.
    public static let followSymlinks = OpenDirMode(rawValue: 1 << 0)
}

public enum MultiStreamPartNameFormat: UInt8, Enum {

    public typealias EType = plzma_plzma_multi_stream_part_name_format
    
    /// "File"."Extension"."002". The maximum number of parts is 999.
    case nameExt00x = 1
}

extension plzma_plzma_multi_stream_part_name_format: Enum {
    
    public typealias EType = MultiStreamPartNameFormat
}
