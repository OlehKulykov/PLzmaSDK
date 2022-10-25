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

/// The input stream.
///
/// The stream could be initialized with path or memory data.
public final class InStream {
    private final class DataNoCopyContext {
        let data: Data
        var offset = Int64(0)
        init(_ d: Data) {
            data = d
        }
    }
    
    internal let object: plzma_in_stream
    
    
    /// Checks the input file stream is opened.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func opened() throws -> Bool {
        var stream = object
        let result = plzma_in_stream_opened(&stream)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Erases and removes the content of the stream.
    /// - Parameter erase: The type of erasing the content.
    /// - Returns: The erasing result.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func erase(_ erase: Erase) throws -> Bool {
        var stream = object
        let result = plzma_in_stream_erase(&stream, erase.type)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    internal init(object: plzma_in_stream) {
        self.object = object
    }
    
    
    /// Initializes the input file stream with movable path.
    /// - Parameter movablePath: The movable non-empty input file path.
    /// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
    public init(movablePath path: Path) throws {
        var pathObject = path.object
        let stream = plzma_in_stream_create_with_pathm(&pathObject)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    
    /// Initializes the input file stream with movable path.
    /// - Parameter path: The non-empty input file path.
    /// - Throws: `Exception` with `.invalidArguments` code in case if path is empty.
    public init(path: Path) throws {
        var pathObject = path.object
        let stream = plzma_in_stream_create_with_path(&pathObject)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    
    /// Initializes the input file stream with the file data.
    /// During the creation, the data will copyed.
    /// - Parameter dataCopy: The file data.
    /// - Throws: `Exception` with `.notEnoughMemory` code in case if can't allocate required size of memory.
    /// - Throws: `Exception` with `.invalidArguments` code in case if file data is empty.
    public init(dataCopy: Data) throws {
        let size = dataCopy.count
        let stream: plzma_in_stream = try dataCopy.withUnsafeBytes({ ptr in
            guard let address = ptr.baseAddress else {
                throw Exception(code: .invalidArguments,
                                what: "Can't instantiate in-stream without memory.",
                                reason: "The memory is null.",
                                file: #file,
                                line: #line)
            }
            return plzma_in_stream_create_with_memory_copy(address, size)
        })
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        object = stream
    }
    
    
    /// Initializes the input file stream with the file data.
    /// During the creation and lifetime the data will not be copyed.
    /// - Parameter dataNoCopy: The file data.
    /// - Throws: `Exception` with `.invalidArguments` code in case if file data is empty.
    public init(dataNoCopy: Data) throws {
        if dataNoCopy.isEmpty {
            throw Exception(code: .invalidArguments,
                            what: "Can't instantiate in-stream without memory.",
                            reason: "The memory size is zero.",
                            file: #file,
                            line: #line)
        }
        let context = DataNoCopyContext(dataNoCopy)
        let unmanagedContext = Unmanaged<DataNoCopyContext>.passRetained(context)
        let contextObject = plzma_context(context: unmanagedContext.toOpaque()) { unmanagedContext in
            Unmanaged<DataNoCopyContext>.fromOpaque(unmanagedContext).release()
        }
        let stream = plzma_in_stream_create_with_callbacks({ unmanagedContext in
            guard let unmanagedContext = unmanagedContext else {
                return false
            }
            let context = Unmanaged<DataNoCopyContext>.fromOpaque(unmanagedContext).takeUnretainedValue()
            context.offset = 0
            return true
        }, { unmanagedContext in
            guard let unmanagedContext = unmanagedContext else {
                return
            }
            let context = Unmanaged<DataNoCopyContext>.fromOpaque(unmanagedContext).takeUnretainedValue()
            context.offset = 0
        }, { (unmanagedContext, offset, origin, newPosition) -> Bool in
            guard let unmanagedContext = unmanagedContext else {
                return false
            }
            let context = Unmanaged<DataNoCopyContext>.fromOpaque(unmanagedContext).takeUnretainedValue()
            let finalOffset: Int64
            switch Int(origin) {
                case Int(SEEK_SET):
                    finalOffset = offset
                    break
                case Int(SEEK_CUR):
                    finalOffset = context.offset + offset
                    break
                case Int(SEEK_END):
                    finalOffset = Int64(context.data.count) + offset
                    break
                default:
                    finalOffset = -1
                    break
            }
            if finalOffset >= 0 && finalOffset <= context.data.count {
                context.offset = finalOffset
                newPosition.pointee = UInt64(finalOffset)
                return true
            }
            context.offset = 0
            newPosition.pointee = UInt64(0)
            return false
        }, { (unmanagedContext, data, size, processedSize) -> Bool in
            guard let unmanagedContext = unmanagedContext else {
                return false
            }
            let context = Unmanaged<DataNoCopyContext>.fromOpaque(unmanagedContext).takeUnretainedValue()
            let available = Int64(context.data.count) - context.offset
            var sizeToRead = Int64(0)
            if available > 0 {
                sizeToRead = (size <= available) ? Int64(size) : available
                context.data.copyBytes(to: data.assumingMemoryBound(to: UInt8.self),
                                       from: Int(context.offset)..<Int(context.offset + sizeToRead))
                context.offset += sizeToRead
            }
            processedSize.pointee = UInt32(sizeToRead)
            return true
        }, contextObject)
        if let exception = stream.exception {
            unmanagedContext.release()
            throw Exception(object: exception)
        }
        object = stream
    }
    
    /// Initializes multi input stream with an array of input streams.
    /// The array should not be empty. The order: file.001, file.002, ..., file.XXX
    /// - Parameter streams: The non-empty array of input streams. Each stream inside array should also exist.
    /// - Note: The content of array will be moved to the newly created stream.
    /// - Note: The array should not be empty.
    /// - Throws: `Exception` with `.invalidArguments` code in case if streams array is empty or contains empty stream.
    public init(streams: [InStream]) throws {
        let count = streams.count
        if count < 0 || count > Int64(PLZMA_SIZE_T_MAX) {
            throw Exception(code: .invalidArguments,
                            what: "Invalid number of streams.",
                            reason: "The number should be positive and less than maximum supported size.",
                            file: #file,
                            line: #line)
        }
        
        var streamsArrayObject = plzma_in_stream_array_create_with_capacity(Size(count))
        if let exception = streamsArrayObject.exception {
            throw Exception(object: exception)
        }
        
        defer {
            plzma_in_stream_array_release(&streamsArrayObject)
        }
        
        for subStream in streams {
            var subStreamObject = subStream.object
            plzma_in_stream_array_add(&streamsArrayObject, &subStreamObject)
            if let exception = streamsArrayObject.exception {
                throw Exception(object: exception)
            }
        }
        
        let stream = plzma_in_stream_create_with_stream_arraym(&streamsArrayObject)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        
        object = stream
    }
    
    deinit {
        var stream = object
        plzma_in_stream_release(&stream)
    }
}
