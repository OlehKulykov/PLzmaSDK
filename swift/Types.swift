//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2020 Oleh Kulykov <olehkulykov@gmail.com>
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

public enum ErrorCode: UInt8, Enum {
    
    public typealias EType = plzma_error_code
    
    case unknown = 0
    case invalidArguments = 1
    case notEnoughMemory = 2
    case io = 3
    case `internal` = 4
}

extension plzma_error_code: Enum {
    
    public typealias EType = ErrorCode
}

public enum Erase: UInt8, Enum {
    
    public typealias EType = plzma_erase
    
    case none = 0
    case zero = 1
}

extension plzma_erase: Enum {
    
    public typealias EType = Erase
}

public enum FileType: UInt8, Enum {
    
    public typealias EType = plzma_file_type
    
    case sevenZ = 1 // 7z
    case xz = 2
    case tar = 3
}

extension plzma_file_type: Enum {
    
    public typealias EType = FileType
}

public enum Method: UInt8, Enum {

    public typealias EType = plzma_method
    
    case LZMA = 1
    case LZMA2 = 2
    case PPMd = 3
}

extension plzma_method: Enum {
    
    public typealias EType = Method
}

public struct OpenDirMode: OptionSet {
    public typealias RawValue = plzma_open_dir_mode_t
    
    public let rawValue: plzma_open_dir_mode_t
    
    public init(rawValue: plzma_open_dir_mode_t) {
        self.rawValue = rawValue
    }
    
    public static let `default` = OpenDirMode([])
    public static let followSymlinks = OpenDirMode(rawValue: 1 << 0)
}
