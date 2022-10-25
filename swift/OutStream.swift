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

/// The out file or memory or multi stream.
public class OutStream {
    internal let object: plzma_out_stream
    
    internal var isMulti: Bool {
        return false
    }
    
    /// Checks the out stream is opened.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func opened() throws -> Bool {
        var stream = object
        let result = plzma_out_stream_opened(&stream)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Copies the content of the stream to `Data`.
    ///
    /// The stream must be closed.
    /// - Returns: The `Data` with stream's content.
    /// - Throws: `Exception` with `.notEnoughMemory` code in case if required amount of memory can't be allocated.
    /// - Note: Thread-safe.
    public func copyContent() throws -> Data {
        var stream = object
        let content = plzma_out_stream_copy_content(&stream)
        if let exception = content.exception {
            throw Exception(object: exception)
        }
        if content.size > 0, let memory = content.memory {
            return Data(bytesNoCopy: memory, count: content.size, deallocator: .custom({ (memory, _) in
                plzma_free(memory)
            }))
        }
        plzma_free(content.memory)
        return Data()
    }
    
    
    /// Erases and removes the content of the stream.
    /// - Parameter erase: The type of erasing the content.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func erase(erase: Erase = .none) throws -> Bool {
        var stream = object
        let result = plzma_out_stream_erase(&stream, erase.type)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    internal init(object o: plzma_out_stream) {
        object = o
    }
    
    
    /// Initializes the output file stream with movable path.
    /// - Parameter path: The movable non-empty input file path.
    /// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
    public init(movablePath path: Path) throws {
        var pathObject = path.object
        let stream = plzma_out_stream_create_with_pathm(&pathObject)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    
    /// Initializes the output file stream with movable path.
    /// - Parameter path: The non-empty input file path.
    /// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
    public init(path: Path) throws {
        var pathObject = path.object
        let stream = plzma_out_stream_create_with_path(&pathObject)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    
    /// Initializes the output file stream object for writing to memory.
    /// - Throws: `Exception`.
    public init() throws {
        let stream = plzma_out_stream_create_memory_stream()
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    deinit {
        var stream = object
        plzma_out_stream_release(&stream)
    }
}
