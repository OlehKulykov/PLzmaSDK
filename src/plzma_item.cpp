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


#include <cstddef>

#include "../libplzma.hpp"
#include "plzma_private.hpp"
#include "plzma_common.hpp"

namespace plzma {
    
    const Path & Item::path() const noexcept {
        return _path;
    }
    
    plzma_size_t Item::index() const noexcept {
        return _index;
    }
    
    uint64_t Item::size() const noexcept {
        return _size;
    }
    
    uint64_t Item::packSize() const noexcept {
        return _packSize;
    }
    
    uint32_t Item::crc32() const noexcept {
        return _crc32;
    }
    
    time_t Item::creationTime() const noexcept {
        return _creationTime;
    }
    
    time_t Item::accessTime() const noexcept {
        return _accessTime;
    }
    
    time_t Item::modificationTime() const noexcept {
        return _modificationTime;
    }
    
    bool Item::encrypted() const noexcept {
        return _encrypted;
    }
    
    bool Item::isDir() const noexcept {
        return _isDir;
    }
    
    void Item::setSize(const uint64_t size) noexcept {
        _size = size;
    }
    
    void Item::setPackSize(const uint64_t size) noexcept {
        _packSize = size;
    }
    
    void Item::setCrc32(const uint32_t crc) noexcept {
        _crc32 = crc;
    }
    
    void Item::setCreationTime(const time_t time) noexcept {
        _creationTime = time;
    }
    
    void Item::setAccessTime(const time_t time) noexcept {
        _accessTime = time;
    }
    
    void Item::setModificationTime(const time_t time) noexcept {
        _modificationTime = time;
    }
    
    void Item::setEncrypted(const bool encrypted) noexcept {
        _encrypted = encrypted;
    }
    
    void Item::setIsDir(const bool dir) noexcept {
        _isDir = dir;
    }
    
    void Item::retain() noexcept {
        LIBPLZMA_RETAIN_IMPL(_referenceCounter)
    }
    
    void Item::release() noexcept {
        LIBPLZMA_RELEASE_IMPL(_referenceCounter)
    }
    
    Item::Item(const Path & path, const plzma_size_t index) :
        _path(path),
        _index(index) {
            
    }
    
    Item::Item(Path && path, const plzma_size_t index) noexcept :
        _path(static_cast<Path &&>(path)),
        _index(index) {
            
    }

} // namespace plzma


#include "plzma_c_bindings_private.hpp"

#if !defined(LIBPLZMA_NO_C_BINDINGS)

using namespace plzma;

plzma_item plzma_item_create_with_path(const plzma_path * LIBPLZMA_NONNULL path, const plzma_size_t index) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_item, path)
    auto item = makeShared<Item>(*static_cast<const Path *>(path->object), index);
    createdCObject.object = static_cast<void *>(item.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_item plzma_item_create_with_pathm(plzma_path * LIBPLZMA_NONNULL path, const plzma_size_t index) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_item, path)
    auto item = makeShared<Item>(static_cast<Path &&>(*static_cast<Path *>(path->object)), index);
    createdCObject.object = static_cast<void *>(item.take());
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_path plzma_item_path(const plzma_item * LIBPLZMA_NONNULL item) {
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_FROM_TRY(plzma_path, item)
    createdCObject.object = static_cast<void *>(new Path(static_cast<const Item *>(item->object)->path()));
    LIBPLZMA_C_BINDINGS_CREATE_OBJECT_CATCH
}

plzma_size_t plzma_item_index(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->index();
}

uint64_t plzma_item_size(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->size();
}

uint64_t plzma_item_pack_size(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->packSize();
}

uint32_t plzma_item_crc32(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->crc32();
}

time_t plzma_item_creation_time(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->creationTime();
}

time_t plzma_item_access_time(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->accessTime();
}

time_t plzma_item_modification_time(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? 0 : static_cast<const Item *>(item->object)->modificationTime();
}

bool plzma_item_encrypted(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? false : static_cast<const Item *>(item->object)->encrypted();
}

bool plzma_item_is_dir(const plzma_item * LIBPLZMA_NONNULL item) {
    return item->exception ? false : static_cast<const Item *>(item->object)->isDir();
}

void plzma_item_set_size(plzma_item * LIBPLZMA_NONNULL item, const uint64_t size) {
    if (!item->exception) { static_cast<Item *>(item->object)->setSize(size); }
}

void plzma_item_set_pack_size(plzma_item * LIBPLZMA_NONNULL item, const uint64_t size) {
    if (!item->exception) { static_cast<Item *>(item->object)->setPackSize(size); }
}

void plzma_item_set_crc32(plzma_item * LIBPLZMA_NONNULL item, const uint32_t crc) {
    if (!item->exception) { static_cast<Item *>(item->object)->setCrc32(crc); }
}

void plzma_item_set_creation_time(plzma_item * LIBPLZMA_NONNULL item, const time_t time) {
    if (!item->exception) { static_cast<Item *>(item->object)->setCreationTime(time); }
}

void plzma_item_set_access_time(plzma_item * LIBPLZMA_NONNULL item, const time_t time) {
    if (!item->exception) { static_cast<Item *>(item->object)->setAccessTime(time); }
}

void plzma_item_set_modification_time(plzma_item * LIBPLZMA_NONNULL item, const time_t time) {
    if (!item->exception) { static_cast<Item *>(item->object)->setModificationTime(time); }
}

void plzma_item_set_encrypted(plzma_item * LIBPLZMA_NONNULL item, const bool is_encrypted) {
    if (!item->exception) { static_cast<Item *>(item->object)->setEncrypted(is_encrypted); }
}

void plzma_item_set_is_dir(plzma_item * LIBPLZMA_NONNULL item, const bool is_dir) {
    if (!item->exception) { static_cast<Item *>(item->object)->setIsDir(is_dir); }
}

void plzma_item_release(plzma_item * LIBPLZMA_NONNULL item) {
    plzma_object_exception_release(item);
    SharedPtr<Item> itemSPtr;
    itemSPtr.assign(static_cast<Item *>(item->object));
    item->object = nullptr;
}

#endif // !LIBPLZMA_NO_C_BINDINGS
