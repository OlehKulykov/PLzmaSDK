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

/// The array of the archive items.
public final class ItemArray {
    internal let object: plzma_item_array
    
    /// The number of items inside the array.
    public var count: Size {
        var array = object
        return plzma_item_array_count(&array)
    }
    
    
    /// Receives the retained item at index.
    /// - Parameter index: The index of the item. Must be less than the number of items, i.e. the value of the `count` property.
    /// - Returns: The item at index.
    /// - Throws: `Exception`.
    public func item(at index: Size) throws -> Item {
        var array = object
        let item = plzma_item_array_at(&array, index)
        if let exception = item.exception {
            throw Exception(object: exception)
        }
        return Item(object: item)
    }
    
    
    /// Adds the retained item to the array.
    /// - Parameter item: The item to add.
    /// - Throws: `Exception`.
    public func add(item i: Item) throws {
        var array = object
        var item = i.object
        plzma_item_array_add(&array, &item)
        if let exception = array.exception {
            throw Exception(object: exception)
        }
    }
    
    /// Sorts the array's items by the item's index.
    public func sort() {
        var array = object
        plzma_item_array_sort(&array)
    }
        
    internal init(object o: plzma_item_array) {
        object = o
    }
    
    /// Initializes the array of items with optional capacity for a container.
    /// - Parameter capacity: The optinal initial capacity of the array. Not a number of items.
    /// - Throws: `Exception`.
    public init(capacity: Size = 0) throws {
        let array = plzma_item_array_create(capacity)
        if let exception = array.exception {
            throw Exception(object: exception)
        }
        object = array
    }
    
    
    /// Initializes the array of items with native array.
    /// - Parameter items: The native array with items.
    /// - Throws: `Exception`.
    public convenience init(items: [Item]) throws {
        try self.init(capacity: Size(items.count))
        var array = object
        for i in items {
            var item = i.object
            plzma_item_array_add(&array, &item)
            if let exception = array.exception {
                throw Exception(object: exception)
            }
        }
    }
    
    deinit {
        var array = object
        plzma_item_array_release(&array)
    }
}

extension ItemArray {
    
    public static func += (left: ItemArray, right: Item) throws {
        try left.add(item: right)
    }
}
