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
public final class OutMultiStream: OutStream {
 
    internal override var isMulti: Bool {
        return true
    }
    
    /// - Returns: The array of created sub-streams. The stream must be closed.
    /// If stream is opened, then the array is empty.
    /// - Throws: `Exception`.
    func streams() throws -> [OutStream] {
        var stream = object
        var arrayObject = plzma_out_multi_stream_streams(&stream)
        if let exception = arrayObject.exception {
            throw Exception(object: exception)
        }
        defer {
            plzma_out_stream_array_release(&arrayObject)
        }
        
        var array = [OutStream]()
        let count = plzma_out_stream_array_count(&arrayObject);
        var index = Size(0)
        while index < count {
            let outStreamObject = plzma_out_stream_array_at(&arrayObject, index)
            if let exception = outStreamObject.exception {
                throw Exception(object: exception)
            }
            array.append(OutStream(object: outStreamObject))
            index += 1
        }
        return array
    }
    
    
    /// Initializes the output multi stream with directory path, part name, extension, format and part size.
    /// All sub-streams are file streams.
    /// - Parameter directoryPath: The non-empty output directory path.
    /// - Parameter partName: The non-empty output file name.
    /// - Parameter partExtension: Optional extension.
    /// - Parameter format: The format of the result file name part.
    /// - Parameter partSize: The maximum size in bytes of each out file sub-stream.
    /// If the number of file parts/sub-streams will exceed the maximum for a `format`, then the runtime exception will be thrown.
    /// - Throws: `Exception` with `io` code in case if path doesn't exist and stream can't create new one.
    /// - Throws: `Exception` with `invalidArguments` code in case if path is empty.
    /// - Throws: `Exception` with `invalidArguments` code in case if path is not a directory or there are no write permissions.
    public init(directoryPath: Path,
                partName: String,
                partExtension: String,
                format: MultiStreamPartNameFormat,
                partSize: Size) throws {
        var dirPathObject = directoryPath.object
        let stream: plzma_out_multi_stream = try partName.utf8CString.withUnsafeBufferPointer { partNamePtr in
            guard let partNameBaseAddress = partNamePtr.baseAddress else {
                throw Exception(code: .invalidArguments,
                                what: "The baseAddress of the partName argument is nil.",
                                reason: "The partName is empty.",
                                file: #file,
                                line: #line)
            }
            return partExtension.utf8CString.withUnsafeBufferPointer { partExtensionPtr in
                return plzma_out_multi_stream_create_with_directory_path_utf8_name_ext_format_part_size(&dirPathObject,
                                                                                                        partNameBaseAddress,
                                                                                                        partExtensionPtr.baseAddress,
                                                                                                        format.type,
                                                                                                        partSize)
            }
        }
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        
        super.init(object: stream)
    }
    
    
    /// Initializes the output multi stream object for writing to memory.
    /// All sub-streams are memory streams.
    /// - Parameter partSize: The maximum size in bytes of each out memory sub-stream.
    public init(partSize: Size) throws {
        let stream = plzma_out_multi_stream_create_memory_with_part_size(partSize)
        if let exception = stream.exception {
            throw Exception(object: exception)
        }
        
        super.init(object: stream)
    }
    
}
