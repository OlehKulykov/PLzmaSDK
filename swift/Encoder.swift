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

/// The interface to a progress delegate of the encoder.
public protocol EncoderDelegate: AnyObject {
    /// Reports the progress of the operation.
    /// - Parameter encoder: The encoder.
    /// - Parameter path: The currently processing item's path.
    /// - Parameter progress: The total progress of the operation.
    func encoder(encoder: Encoder, path: String, progress: Double)
}

/// The encoder for compressing the archive items.
public final class Encoder {
    private final class Context {
        weak var encoder: Encoder?
        weak var delegate: EncoderDelegate?
    }
    
    private let object: plzma_encoder
    
    // MARK: - Actions
    
    /// Opens the encoder for compressing.
    ///
    /// During the process, the encoder is self-retained as long the operation is in progress.
    /// - Returns: `false` if nothing to compress or encoder aborted or incorrect number of items or number of items greater than supported.
    /// otherwise `true`.
    /// - Note: The opening progress might be executed in a separate thread.
    /// - Note: The opening progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func open() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_open(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    /// Aborts the opening or compressing process.
    /// - Note: The aborted encoder is no longer valid.
    /// - Throws: `Exception`.
    public func abort() throws {
        var encoder = object
        plzma_encoder_abort(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }

    
    /// Compresses the provided paths and streams.
    ///
    /// During the process, the encoder is self-retained as long the operation is in progress.
    /// - Note: The compress progress might be executed in a separate thread.
    /// - Note: The compress progress might be aborted via `abort()` method.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func compress() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_compress(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    // MARK: - Add data
    
    /// Adds the physical file or directory path to the encoder.
    /// - Parameter path: The file or directory path.
    /// - Parameter mode: The mode for opening directory in case if `path` is a directory path.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func add(path: Path, mode: OpenDirMode = .default) throws {
        var encoder = object
        var pathObject = path.object
        plzma_encoder_add_path(&encoder, &pathObject, mode.rawValue, nil)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Adds the physical file or directory path to the encoder.
    /// - Parameter path: The uniq. file or directory path. Duplicated value is not allowed.
    /// - Parameter mode: The mode for opening directory in case if `path` is a directory path.
    /// - Parameter archivePath: The custom path of how the `path` will be presented in archive.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func add(path: Path, mode: OpenDirMode, archivePath: Path) throws {
        var encoder = object
        var pathObject = path.object
        var archivePathObject = archivePath.object
        plzma_encoder_add_path(&encoder, &pathObject, mode.rawValue, &archivePathObject)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Adds the file in-sctream to the encoder.
    /// - Parameter stream: The input file stream to add. Empty stream is not allowed.
    /// - Parameter archivePath: The custom path of how the `path` will be presented in archive. Empty path is not allowed.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func add(stream: InStream, archivePath: Path) throws {
        var encoder = object
        var streamObject = stream.object
        var archivePathObject = archivePath.object
        plzma_encoder_add_stream(&encoder, &streamObject, &archivePathObject)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    // MARK: - Properties
    
    /// Provides the password for archive.
    ///
    /// This password will be used for encrypting header and the content if such options are enabled
    /// and selected type supports password protection.
    /// - SeeAlso: `setShouldEncryptHeader`, `setShouldEncryptContent` methods and `FileType` enum.
    /// - Parameter password: The password. nil or zero length password means no password provided.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setPassword(_ password: String?) throws {
        var encoder = object
        if let pass = password {
            let _ = pass.utf8CString.withUnsafeBufferPointer { ptr in
                plzma_encoder_set_password_utf8_string(&encoder, ptr.baseAddress)
            }
        } else {
            plzma_encoder_set_password_utf8_string(&encoder, nil)
        }
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Getter for a 'solid' archive property.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldCreateSolidArchive() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_create_solid_archive(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Setter for a 'solid' archive property.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldCreateSolidArchive(_ solid: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_create_solid_archive(&encoder, solid)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Getter for a compression level.
    /// - Returns: The level in a range [0; 9].
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func compressionLevel() throws -> UInt8 {
        var encoder = object
        let result = plzma_encoder_compression_level(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Setter for an archive compression level.
    /// - Parameter level: The level in a range [0; 9].
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setCompressionLevel(_ level: UInt8) throws {
        var encoder = object
        plzma_encoder_set_compression_level(&encoder, level)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder compress the archive header.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldCompressHeader() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_compress_header(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will compress the archive header.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldCompressHeader(_ compress: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_compress_header(&encoder, compress)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder fully compress the archive header.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldCompressHeaderFull() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_compress_header_full(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will fully compress the archive header.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldCompressHeaderFull(_ compress: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_compress_header_full(&encoder, compress)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder encrypt the content of the archive items.
    /// - Note: The password will be required to decode/extract archive items.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldEncryptContent() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_encrypt_content(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will encrypt the content of the archive items.
    ///
    /// The encryption will take place only if this option enabled, the type supports password protection
    /// and the password has been provided.
    /// - SeeAlso: `setPassword` method and `FileType` enum.
    /// - Note: The password will be required to decode/extract archive items.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldEncryptContent(_ encrypt: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_encrypt_content(&encoder, encrypt)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder encrypt the header with the list of archive items.
    /// - Note: The password will be required to open archive and list the items.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldEncryptHeader() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_encrypt_header(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will encrypt the header with the list of archive items.
    ///
    /// The encryption will take place only if this option enabled, the type supports password protection
    /// and the password has been provided.
    /// - SeeAlso: `setPassword` method and `FileType` enum.
    /// - Note: The password will be required to open archive and list the items.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldEncryptHeader(_ encrypt: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_encrypt_header(&encoder, encrypt)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder store the creation time of each item to the header if such available.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldStoreCreationTime() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_store_creation_time(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will store the creation time of each item to the header if such available.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldStoreCreationTime(_ store: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_store_creation_time(&encoder, store)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder store the access time of each item to the header if such available.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldStoreAccessTime() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_store_access_time(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will store the access time of each item to the header if such available.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldStoreAccessTime(_ store: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_store_access_time(&encoder, store)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Should encoder store the last modification time of each item to the header if such available.
    /// - Note: Enabled by default, the value is `true`.
    /// - Note: Thread-safe.
    /// - Throws: `Exception`.
    public func shouldStoreModificationTime() throws -> Bool {
        var encoder = object
        let result = plzma_encoder_should_store_modification_time(&encoder)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Set encoder will store the last modification time of each item to the header if such available.
    /// - Note: Thread-safe. Must be set before opening.
    /// - Throws: `Exception`.
    public func setShouldStoreModificationTime(_ store: Bool) throws {
        var encoder = object
        plzma_encoder_set_should_store_modification_time(&encoder, store)
        if let exception = encoder.exception {
            throw Exception(object: exception)
        }
    }
    
    // MARK: - Initialization
    
    /// Initializes the encoder with output file stream.
    /// - Parameter stream: The output file stream to write the archive's file content.
    /// - Parameter fileType: The type of the archive.
    /// - Parameter method: The compresion method.
    /// - Parameter delegate: Optional delegate.
    /// - Throws: `Exception` with `.invalidArguments` code in case if provided stream is empty.
    public init(stream: OutStream, fileType: FileType, method: Method, delegate: EncoderDelegate? = nil) throws {
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
        var encoder: plzma_encoder
        if stream.isMulti {
            encoder = plzma_encoder_create_with_multi_stream(&streamObject, fileType.type, method.type, contextObject)
        } else {
            encoder = plzma_encoder_create(&streamObject, fileType.type, method.type, contextObject)
        }
        if let exception = encoder.exception {
            if let unmanagedContext = unmanagedContext {
                unmanagedContext.release()
            }
            throw Exception(object: exception)
        }
        
        if delegate != nil {
            plzma_encoder_set_progress_delegate_utf8_callback(&encoder) { (ctx, utf8CString, progress) in
                guard let ctx = ctx else {
                    return
                }
                let context = Unmanaged<Context>.fromOpaque(ctx).takeUnretainedValue()
                guard let encoder = context.encoder, let delegate = context.delegate else {
                    return
                }
                delegate.encoder(encoder: encoder, path: String(utf8CString: utf8CString), progress: progress)
            }
            if let exception = encoder.exception {
                if let unmanagedContext = unmanagedContext {
                    unmanagedContext.release()
                }
                throw Exception(object: exception)
            }
        }
        
        object = encoder
        
        if let context = context {
            context.encoder = self
        }
    }
    
    deinit {
        var encoder = object
        plzma_encoder_release(&encoder)
    }
}
