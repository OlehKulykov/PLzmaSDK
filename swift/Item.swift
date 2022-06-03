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

/// The archive item.
public final class Item {
    private var _path: Path?
    internal let object: plzma_item
    
    /// The item's path inside the archive.
    /// - Throws: `Exception`.
    public func path() throws -> Path {
        if let p = _path {
            return p
        }
        var item = object
        let pathObject = plzma_item_path(&item)
        if let exception = pathObject.exception {
            throw Exception(object: exception)
        }
        let p = Path(object: pathObject)
        _path = p
        return p
    }
    
    
    /// The item's index inside the archive.
    public var index: Size {
        var item = object
        return plzma_item_index(&item)
    }
    
    
    /// The size in bytes of the item.
    public var size: UInt64 {
        get {
            var item = object
            return plzma_item_size(&item)
        }
        set {
            var item = object
            plzma_item_set_size(&item, newValue)
        }
    }
    
    
    /// The packed size in bytes of the item.
    public var packSize: UInt64 {
        get {
            var item = object
            return plzma_item_pack_size(&item)
        }
        set {
            var item = object
            plzma_item_set_pack_size(&item, newValue)
        }
    }
    
    
    /// The CRC-32 checksum of the item's content.
    public var crc32: UInt32 {
        get {
            var item = object
            return plzma_item_crc32(&item)
        }
        set {
            var item = object
            plzma_item_set_crc32(&item, newValue)
        }
    }

    
    /// The creation date of the item. Based on unix timestamp.
    public var creationDate: Date {
        get {
            var item = object
            return Date(timeIntervalSince1970: TimeInterval(plzma_item_creation_time(&item)))
        }
        set {
            var item = object
            plzma_item_set_creation_time(&item, time_t(newValue.timeIntervalSince1970))
        }
    }
    
    
    /// The last access date of the item. Based on unix timestamp.
    public var accessDate: Date {
        get {
            var item = object
            return Date(timeIntervalSince1970: TimeInterval(plzma_item_access_time(&item)))
        }
        set {
            var item = object
            plzma_item_set_access_time(&item, time_t(newValue.timeIntervalSince1970))
        }
    }

    
    /// The last modification date of the item. Based on unix timestamp.
    public var modificationDate: Date {
        get {
            var item = object
            return Date(timeIntervalSince1970: TimeInterval(plzma_item_modification_time(&item)))
        }
        set {
            var item = object
            plzma_item_set_modification_time(&item, time_t(newValue.timeIntervalSince1970))
        }
    }

    
    /// The item is encrypted or not.
    public var encrypted: Bool {
        get {
            var item = object
            return plzma_item_encrypted(&item)
        }
        set {
            var item = object
            plzma_item_set_encrypted(&item, newValue)
        }
    }
    
    
    /// The item is directory or file.
    public var isDir: Bool {
        get {
            var item = object
            return plzma_item_is_dir(&item)
        }
        set {
            var item = object
            plzma_item_set_is_dir(&item, newValue)
        }
    }
    
    internal init(object o: plzma_item) {
        object = o
    }
    
    
    /// Initializes the item with movable path and optional index inside the archive.
    /// - Parameter movablePath: The associated movable item's path.
    /// - Parameter index: Optional index of the item inside the archive.
    /// - Throws: `Exception`.
    public init(movablePath path: Path, index: Size = 0) throws {
        var pathObject = path.object
        let item = plzma_item_create_with_pathm(&pathObject, index)
        if let exception = item.exception {
            throw Exception(object: exception)
        }
        object = item
    }
    
    
    /// Initializes the item with path and optional index inside the archive.
    /// - Parameter path: The associated item's path.
    /// - Parameter index: Optional index of the item inside the archive.
    /// - Throws: `Exception`.
    public init(path: Path, index: Size = 0) throws {
        var pathObject = path.object
        let item = plzma_item_create_with_path(&pathObject, index)
        if let exception = item.exception {
            throw Exception(object: exception)
        }
        object = item
    }
    
    /// Initializes the item with path string and optional index inside the archive.
    /// - Parameter path: The associated item's path.
    /// - Parameter index: Optional index of the item inside the archive.
    /// - Throws: `Exception`.
    public convenience init(_ string: String, index: Size = 0) throws {
        try self.init(movablePath: Path(string))
    }
    
    deinit {
        var item = object
        plzma_item_release(&item)
    }
}

extension Item: Hashable {
    
    public func hash(into hasher: inout Hasher) {
        hasher.combine(index)
    }
    
    public static func == (left: Item, right: Item) -> Bool {
        return left.index == right.index
    }
}

extension Item: CustomStringConvertible {
    
    public var description: String {
        let p = try? path()
        return "Path: \((p != nil) ? p!.description : "") \nIndex: \(index)\nSize: \(size)\nPack size: \(packSize)\nCRC32: \(crc32)\nCreation date: \(creationDate)\nAccess date: \(accessDate)\nModification date: \(modificationDate)\nEncrypted: \(encrypted)\nIs dir: \(isDir)"
    }
}

extension Item: CustomDebugStringConvertible {
    
    public var debugDescription: String {
        return description
    }
}
