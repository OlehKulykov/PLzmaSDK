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

/// The array of item/out-stream pairs.
public final class ItemOutStreamArray {
    internal let object: plzma_item_out_stream_array

    /// The item/out-stream pair type.
    public typealias Pair = (Item, OutStream)
    
    
    /// The number of item/out-stream pairs inside the array.
    public var count: Size {
        var map = object
        return plzma_item_out_stream_array_count(&map)
    }
    
    
    /// Receives the item/out-stream pair at index.
    /// - Parameter index: The index of the item/out-stream pair. Must be less than the number of items, i.e. the value of the `count` property.
    /// - Returns: The item/out-stream pair at index.
    /// - Throws: `Exception`.
    public func pair(at index: Size) throws -> Pair {
        var map = object
        var pair = plzma_item_out_stream_array_pair_at(&map, index)
        if let exception = pair.exception {
            plzma_item_release(&pair.item)
            plzma_out_stream_release(&pair.stream)
            throw Exception(object: exception)
        }
        return Pair(Item(object: pair.item), OutStream(object: pair.stream))
    }
    
    
    /// Adds the retained item/out-stream pair to the array.
    /// - Parameter item: The item to add. Similar to a map key.
    /// - Parameter stream: The out-stream to add. Similar to a map value for a key.
    /// - Throws: `Exception`.
    public func add(item: Item, stream: OutStream) throws {
        var map = object
        var itemObject = item.object
        var streamObject = stream.object
        plzma_item_out_stream_array_add(&map, &itemObject, &streamObject)
        if let exception = map.exception {
            throw Exception(object: exception)
        }
    }
    
    
    /// Sorts the array's item/out-stream pairs by the item's index.
    public func sort() {
        var map = object
        plzma_item_out_stream_array_sort(&map)
    }
        
    internal init(object o: plzma_item_out_stream_array) {
        object = o
    }
    
    
    /// Initializes the array of item/out-stream pairs with optional capacity for a container.
    /// - Parameter capacity: The optinal initial capacity of the array. Not a number of items.
    /// - Throws: `Exception`.
    public init(capacity: Size = 0) throws {
        let map = plzma_item_out_stream_array_create(capacity)
        if let exception = map.exception {
            throw Exception(object: exception)
        }
        object = map
    }
    
    
    /// Initializes the array of item/out-stream pairs with native item/out-stream dictionary.
    /// - Parameter items: The native item/out-stream dictionary.
    /// - Throws: `Exception`.
    public convenience init(items: [Item: OutStream]) throws {
        try self.init(capacity: Size(items.count))
        var map = object
        for p in items {
            var item = p.key.object
            var stream = p.value.object
            plzma_item_out_stream_array_add(&map, &item, &stream)
            if let exception = map.exception {
                throw Exception(object: exception)
            }
        }
    }
    
    deinit {
        var map = object
        plzma_item_out_stream_array_release(&map)
    }
}

extension ItemOutStreamArray {
    
    public static func += (left: ItemOutStreamArray, right: Pair) throws {
        try left.add(item: right.0, stream: right.1)
    }
}
