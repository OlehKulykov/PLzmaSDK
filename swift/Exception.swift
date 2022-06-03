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

/// The exception class for all cases in the library.
public struct Exception: Error {
    /// Returns an explanatory string.
    public let what: String
    
    
    /// Returns the reason why the exception was thrown.
    public let reason: String
    
    
    /// Returns the file name in which the exception was thrown.
    public let file: String
    
    
    /// Returns the current version of the library.
    public let version: String
    
    
    /// Returns predefined code, type of the exception.
    public let code: ErrorCode
    
    
    /// Returns the line number of the file where exception was thrown.
    public let line: Int
    
    internal init(object: plzma_exception_ptr) {
        code = plzma_exception_code(object).type
        what = String(utf8CString: plzma_exception_what_utf8_string(object))
        reason = String(utf8CString: plzma_exception_reason_utf8_string(object))
        file = String(utf8CString: plzma_exception_file_utf8_string(object))
        version = String(utf8CString: plzma_version())
        line = Int(plzma_exception_line(object))
        plzma_exception_release(object)
    }
    
    init(code: ErrorCode, what: String, reason: String, file: String? = nil, line: Int = 0) {
        self.code = code
        self.what = what
        self.reason = reason
        if let file = file {
            self.file = URL(fileURLWithPath: file).lastPathComponent
        } else {
            self.file = ""
        }
        self.line = line
        version = String(utf8CString: plzma_version())
    }
}

extension Exception: CustomStringConvertible {
    
    public var description: String {
        return "Code: \(code)\nWhat: \(what)\nReason: \(reason)\nFile: \(file)\nLine: \(line)\nVersion: \(version)"
    }
}

extension Exception: CustomDebugStringConvertible {
    
    public var debugDescription: String {
        return description
    }
}
