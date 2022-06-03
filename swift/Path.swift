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

/// The path.
///
/// During the manipulation of the path's string presentation:
/// - The path separator will be automatically replaced with the platform specific one.
/// - The previous content will be erased with `.zero`, i.e. zero-filled.
public final class Path {
    internal let object: plzma_path
    
    /// Type of the path stat struct.
    public typealias Stat = plzma_path_stat
    
    
    /// The number of unicode characters.
    public var count: Size {
        var path = object
        return plzma_path_count(&path)
    }
    
    
    /// Checks the path exists.
    /// - Throws: `Exception`.
    public func exists() throws -> Bool {
        var path = object
        let result = plzma_path_exists(&path, nil)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Checks the path exists with optional checking for a directory.
    /// - Parameter isDir: The optional pointer to a boolean variable to store the result of checking for a directory.
    /// - Throws: `Exception`.
    @discardableResult
    public func exists(isDir: inout Bool) throws -> Bool {
        var path = object
        let result = plzma_path_exists(&path, &isDir)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Checks the path exists and has read permissions.
    /// - Returns: `true` if path exists and readable, otherwise `false`.
    /// - Throws: `Exception`.
    public func readable() throws -> Bool {
        var path = object
        let result = plzma_path_readable(&path)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Checks the path exists and has write permissions.
    /// - Returns: `true` if path exists and writable, otherwise `false`.
    /// - Throws: `Exception`.
    public func writable() throws -> Bool {
        var path = object
        let result = plzma_path_writable(&path)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Checks the path exists and has read-write permissions.
    /// - Returns: `true` if path exists, readable and writable, otherwise `false`.
    /// - Throws: `Exception`.
    public func readableAndWritable() throws -> Bool {
        var path = object
        let result = plzma_path_readable_and_writable(&path)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    /// Provides the stat info of the path.
    /// - Returns: The stat info of the path or empty/zero-filled struct if operation was failed.
    /// - Throws: `Exception`.
    public func stat() throws -> Stat {
        var path = object
        let result = plzma_path_get_stat(&path)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Clears the path's string presentation.
    /// - Parameter erase: The type of the erasing the path's string.
    public func clear(erase: Erase = .zero) {
        var path = object
        plzma_path_clear(&path, erase.type)
    }
    
    
    /// Physically removes the directory with all content or file associated with the path.
    /// - Parameter skipErrors: Skips errors and continuing removing or stop on first error.
    /// - Throws: `Exception`.
    @discardableResult
    public func remove(skipErrors: Bool = false) throws -> Bool {
        var path = object
        let result = plzma_path_remove(&path, skipErrors)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Receives the last component of the path.
    /// - Throws: `Exception`.
    public func lastComponent() throws -> Path {
        var path = object
        let result = plzma_path_last_component(&path)
        if let exception = result.exception {
            throw Exception(object: exception)
        }
        return Path(object: result)
    }
    
    
    /// Creates the directory at specific path.
    /// - Parameter withIntermediates: Create intermediate directories for each component or not.
    /// - Throws: `Exception`.
    public func createDir(withIntermediates: Bool) throws -> Bool {
        var path = object
        let result = plzma_path_create_dir(&path, withIntermediates)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return result
    }
    
    
    /// Opens a directory associated with path for iterating the content.
    /// - Parameter mode: The open directory mode.
    /// - Returns: The path iterator.
    /// - Throws: `Exception` with `.io` code in case if a directory can't be opened.
    public func openDir(mode: OpenDirMode = .default) throws -> Iterator {
        var path = object
        let iterator = plzma_path_open_dir(&path, mode.rawValue)
        if let exception = iterator.exception {
            throw Exception(object: exception)
        }
        return Iterator(object: iterator)
    }
    
    
    /// Appends the random component to the path.
    /// 
    /// If path successfully updated, then the updated path doesn't exists in a root directory.
    /// The component consists of ASCII characters in range ['a'; 'z'].
    /// - Throws: `Exception` with `.internal` code in case if path can't be appended.
    public func appendRandomComponent() throws {
        var path = object
        plzma_path_append_random_component(&path)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
    }
    
    /// - SeeAlso: `appendRandomComponent` method.
    /// - Throws: `Exception`
    public func appendingRandomComponent() throws -> Path {
        var path = object
        let result = plzma_path_appending_random_component(&path)
        if let exception = result.exception {
            throw Exception(object: exception)
        }
        return Path(object: result)
    }
    
    /// Appends the path component to the path.
    /// - Parameter component: The path component to append.
    /// - Throws: `Exception`.
    public func append(component: Path) throws {
        var componentObject = component.object
        let utf8CString = plzma_path_utf8_string(&componentObject)
        if let exception = componentObject.exception {
            throw Exception(object: exception)
        }
        var path = object
        plzma_path_append_utf8_component(&path, utf8CString)
        if let exception = path.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Appends the path component to the path.
    /// - Parameter component: The string component to append.
    /// - Throws: `Exception`.
    public func append(_ component: String) throws {
        var path = object
        let _ = component.utf8CString.withUnsafeBufferPointer { ptr in
            plzma_path_append_utf8_component(&path, ptr.baseAddress)
        }
        if let exception = path.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Set the new path from another path.
    /// - Throws: `Exception`.
    public func set(path: Path) throws {
        var target = path.object
        let utf8CString = plzma_path_utf8_string(&target)
        if let exception = target.exception {
            throw Exception(object: exception)
        }
        var pathObject = object
        plzma_path_set_utf8_string(&pathObject, utf8CString)
        if let exception = pathObject.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Set the new path from another path's string.
    /// - Throws: `Exception`.
    public func set(_ string: String) throws {
        var path = object
        let _ = string.utf8CString.withUnsafeBufferPointer { ptr in
            plzma_path_set_utf8_string(&path, ptr.baseAddress)
        }
        if let exception = path.exception {
            throw Exception(object: exception)
        }
    }
    
    internal init(object o: plzma_path) {
        object = o
    }
    
    
    /// Initializes the path object with another path.
    /// During the creation, the path separators are converted/normalized to a platform specific ones.
    /// - Throws: `Exception`.
    public init(path: Path) throws {
        var target = path.object
        let utf8CString = plzma_path_utf8_string(&target)
        if let exception = target.exception {
            throw Exception(object: exception)
        }
        let pathObject = plzma_path_create_with_utf8_string(utf8CString)
        if let exception = pathObject.exception {
            throw Exception(object: exception)
        }
        object = pathObject
    }
    
    
    /// Initializes the path object with path's string.
    /// During the creation, the path separators are converted/normalized to a platform specific ones.
    /// - Note: No `ExpressibleByStringLiteral`
    /// - Throws: `Exception`.
    public init(_ string: String) throws {
        let path: plzma_path
        if string.count > 0 {
            path = string.utf8CString.withUnsafeBufferPointer { ptr in
                plzma_path_create_with_utf8_string(ptr.baseAddress)
            }
        } else {
            path = plzma_path_create_with_utf8_string(nil)
        }
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        object = path
    }
    
    deinit {
        var path = object
        plzma_path_release(&path)
    }
    
    
    /// Provides the path with the platform specific temporary directory for the library.
    ///
    /// The provided directory path, if such exists, has a read-write permissions.
    /// - Returns: The path with existed temporary directory or empty path.
    /// - Throws: `Exception`.
    public static func tmpDir() throws -> Path {
        let path = plzma_path_create_with_tmp_dir()
        if let exception = path.exception {
            throw Exception(object: exception)
        }
        return Path(object: path)
    }
}

extension Path {
    
    public static func &= (left: Path, right: Path) throws {
        try left.set(path: right)
    }
    
    public static func &= (left: Path, right: String) throws {
        try left.set(right)
    }
    
    public static func += (left: Path, right: Path) throws {
        try left.append(component: right)
    }
    
    public static func += (left: Path, right: String) throws {
        try left.append(right)
    }
}

extension Path: CustomStringConvertible {
    
    public var description: String {
        var path = object
        return String(utf8CString: plzma_path_utf8_string(&path))
    }
}

extension Path: CustomDebugStringConvertible {
    
    public var debugDescription: String {
        return self.description
    }
}

extension Path {
    
    /// Platform specific directory path iterator.
    public final class Iterator {
        internal let object: plzma_path_iterator
        
        
        /// Recevies the current file or directory component.
        /// - Throws: `Exception`.
        public func component() throws -> Path {
            var iterator = object
            let path = plzma_path_iterator_component(&iterator)
            if let exception = path.exception {
                throw Exception(object: exception)
            }
            return Path(object: path)
        }
        
        
        /// Recevies the current file or directory path.
        /// - Throws: `Exception`.
        public func path() throws -> Path {
            var iterator = object
            let path = plzma_path_iterator_path(&iterator)
            if let exception = path.exception {
                throw Exception(object: exception)
            }
            return Path(object: path)
        }
        
        
        /// Recevies the current file or directory full path, prefixed with root path.
        /// - Throws: `Exception`.
        public func fullPath() throws -> Path {
            var iterator = object
            let path = plzma_path_iterator_full_path(&iterator)
            if let exception = path.exception {
                throw Exception(object: exception)
            }
            return Path(object: path)
        }
        
        
        /// Checks the current iterator's path is directory.
        /// - Returns: \a true the iterator's path is directory.
        public var isDir: Bool {
            var iterator = object
            return plzma_path_iterator_is_dir(&iterator)
        }
        
        
        /// Continue iteration.
        /// - Returns: \a true The next file or directory located, otherwise iteration is finished.
        /// - Throws: `Exception`.
        public func next() throws -> Bool {
            var iterator = object
            let result = plzma_path_iterator_next(&iterator)
            if let exception = iterator.exception {
                throw Exception(object: exception)
            }
            return result
        }
        
        
        /// Closes iteration and all open directory descriptors/handlers.
        public func close() {
            var iterator = object
            plzma_path_iterator_close(&iterator)
        }
        
        internal init(object o: plzma_path_iterator) {
            object = o
        }
        
        deinit {
            var iterator = object
            plzma_path_iterator_release(&iterator)
        }
    }
}


/// Contains stat info of the path.
extension plzma_path_stat {
    
    /// Path creation date based on unix timestamp.
    public var creationDate: Date {
        return Date(timeIntervalSince1970: TimeInterval(creation))
    }
    
    /// Last path access date based on unix timestamp.
    public var lastAccessDate: Date {
        return Date(timeIntervalSince1970: TimeInterval(last_access))
    }
    
    /// Last path modification date based on unix timestamp.
    public var lastModificationDate: Date {
        return Date(timeIntervalSince1970: TimeInterval(last_modification))
    }
}

extension plzma_path_stat: CustomStringConvertible {
    
    public var description: String {
        return "Creation: \(creationDate)\nLast access: \(lastAccessDate)\nLast modification: \(lastModificationDate)"
    }
}

extension plzma_path_stat: CustomDebugStringConvertible {
    
    public var debugDescription: String {
        return self.description
    }
}
