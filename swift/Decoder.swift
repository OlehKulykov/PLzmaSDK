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

/// The interface to a progress delegate of the decoder.
public protocol DecoderDelegate: AnyObject {
    /// Reports the progress of the operation.
    /// - Parameter decoder: The decoder.
    /// - Parameter path: The currently processing item's path.
    /// - Parameter progress: The total progress of the operation.
    func decoder(decoder: Decoder, path: String, progress: Double)
}

/// The Decoder for extracting or testing the archive items.
public final class Decoder {
    private final class Context {
        weak var decoder: Decoder?
        weak var delegate: DecoderDelegate?
    }
    
    private let object: plzma_decoder
    
    // MARK: - Actions
    
    /// Opens the archive.
    ///
    /// During the process, the decoder is self-retained as long the operation is in progress.
    /// - Returns: `true` the archive was successfully opened, otherwice `false`.
    /// - Note: After successful opening, the input stream will be opened as long as a decoder exists.
    /// - Note: The opening progress might be executed in a separate thread.
    /// - Note: The opening progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func open() throws -> Bool {
        var decoder = object
        let result = plzma_decoder_open(&decoder)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Aborts opening, extracting or testing process.
    /// - Note: The aborted decoder is no longer valid.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func abort() throws {
        var decoder = object
        plzma_decoder_abort(&decoder)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
    }
    
    // MARK: - Properties
    
    /// - Returns: Receives the number of items in archive.
    /// - Note: The decoder must be opened.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func count() throws -> Size {
        var decoder = object
        let count = plzma_decoder_count(&decoder)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return count
    }
    
    
    /// Receives all archive items.
    /// - Returns: The array with all archive items.
    /// - Note: The decoder must be opened.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func items() throws -> ItemArray {
        var decoder = object
        let items = plzma_decoder_items(&decoder)
        if let exception = items.exception {
            throw Exception(object: exception)
        }
        return ItemArray(object: items)
    }

    
    /// Receives single archive item at a specific index.
    /// - Parameter index: The index of the item inside the arhive. Must be less than the number of items reported by the `count()` method.
    /// - Returns: The archive item.
    /// - Note: The decoder must be opened.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func item(at index: Size) throws -> Item {
        var decoder = object
        let item = plzma_decoder_item_at(&decoder, index)
        if let exception = item.exception {
            throw Exception(object: exception)
        }
        return Item(object: item)
    }
    
    // MARK: - Extracting
    
    /// Extracts all archive items to a specific path.
    /// - Parameter path: The directory path to extract all items.
    /// - Parameter itemsFullPath: Exctract item using it's full path or only last path component.
    /// - Note: The extracting progress might be executed in a separate thread.
    /// - Note: The extracting progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func extract(to path: Path, itemsFullPath: Bool = true) throws -> Bool {
        var decoder = object
        var pathObject = path.object
        let result = plzma_decoder_extract_all_items_to_path(&decoder, &pathObject, itemsFullPath)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Extracts some archive items to a specific path.
    /// - Parameter items: The array of items to extract.
    /// - Parameter path: The directory path to extract all items.
    /// - Parameter itemsFullPath: Exctract item using it's full path or only the last path component.
    /// - Note: The extracting progress might be executed in a separate thread.
    /// - Note: The extracting progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func extract(items: ItemArray, to path: Path, itemsFullPath: Bool = true) throws -> Bool {
        var decoder = object
        var itemsObject = items.object
        var pathObject = path.object
        let result = plzma_decoder_extract_items_to_path(&decoder, &itemsObject, &pathObject, itemsFullPath)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Extracts each archive item to a separate out-stream.
    /// - Parameter items: The array with item/out-stream pairs.
    /// - Note: The extracting progress might be executed in a separate thread.
    /// - Note: The extracting progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func extract(itemsToStreams items: ItemOutStreamArray) throws -> Bool {
        var decoder = object
        var itemsObject = items.object
        let result = plzma_decoder_extract_item_out_stream_array(&decoder, &itemsObject)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    //MARK: - Testing
    
    /// Tests specific archive items.
    /// - Parameter items: The array with items to test.
    /// - Note: The testing progress might be executed in a separate thread.
    /// - Note: The testing progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func test(items: ItemArray) throws -> Bool {
        var decoder = object
        var itemsObject = items.object
        let result = plzma_decoder_test_items(&decoder, &itemsObject)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Tests all archive items.
    /// - Note: The testing progress might be executed in a separate thread.
    /// - Note: The testing progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func test() throws -> Bool {
        var decoder = object
        let result = plzma_decoder_test(&decoder)
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    //MARK: - Initialization
    
    /// Provides the archive password for opening, extracting or testing items.
    /// - Parameter items password: The password.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func setPassword(_ password: String?) throws {
        var decoder = object
        if let pass = password {
            let _ = pass.utf8CString.withUnsafeBufferPointer { ptr in
                plzma_decoder_set_password_utf8_string(&decoder, ptr.baseAddress)
            }
        } else {
            plzma_decoder_set_password_utf8_string(&decoder, nil)
        }
        if let exception = decoder.exception {
            throw Exception(object: exception)
        }
    }
    
    /// Initializes the decoder.
    /// - Parameter stream: The input stream with archive file content.
    ///                     After successful opening, the input stream will be opened as long as a decoder exists.
    /// - Parameter fileType: The file type of the input stream.
    /// - Parameter delegate: Optional delegate.
    /// - Throws: `Exception`.
    public init(stream: InStream, fileType: FileType, delegate: DecoderDelegate? = nil) throws {
        var context: Context?
        let contextObject: plzma_context
        var unmanagedContext: Unmanaged<Context>?
        if delegate != nil {
            let ctx = Context()
            ctx.delegate = delegate
            let unmanagedCtx = Unmanaged<Context>.passRetained(ctx)
            contextObject = plzma_context(context: unmanagedCtx.toOpaque()) { unmanagedContext in
                Unmanaged<Context>.fromOpaque(unmanagedContext).release()
            }
            context = ctx
            unmanagedContext = unmanagedCtx
        } else {
            contextObject = plzma_context(context: nil, deinitializer: nil)
        }
        
        var streamObject = stream.object
        var decoder = plzma_decoder_create(&streamObject, fileType.type, contextObject)
        if let exception = decoder.exception {
            if let unmanagedContext = unmanagedContext {
                unmanagedContext.release()
            }
            throw Exception(object: exception)
        }
        
        if delegate != nil {
            plzma_decoder_set_progress_delegate_utf8_callback(&decoder) { (ctx, utf8CString, progress) in
                guard let ctx = ctx else {
                    return
                }
                let context = Unmanaged<Context>.fromOpaque(ctx).takeUnretainedValue()
                guard let decoder = context.decoder, let delegate = context.delegate else {
                    return
                }
                delegate.decoder(decoder: decoder, path: String(utf8CString: utf8CString), progress: progress)
            }
            if let exception = decoder.exception {
                if let unmanagedContext = unmanagedContext {
                    unmanagedContext.release()
                }
                throw Exception(object: exception)
            }
        }
        
        object = decoder
        
        if let context = context {
            context.decoder = self
        }
    }
    
    deinit {
        var decoder = object
        plzma_decoder_release(&decoder)
    }
}
