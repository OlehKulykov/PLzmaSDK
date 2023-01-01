//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2023 Oleh Kulykov <olehkulykov@gmail.com>
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


#include "plzma_public_tests.hpp"

using namespace plzma;

int test_plzma_containers_test1(void) {
    auto array = makeShared<ItemArray>(0);
    PLZMA_TESTS_ASSERT(array)
    array->clear();
    
    Path path("path");
    auto item = makeShared<Item>(path, 2);
    PLZMA_TESTS_ASSERT(path.count() > 0)
    array->push(item);
    PLZMA_TESTS_ASSERT(array->count() == 1)
    
    path.set("path");
    item = makeShared<Item>(std::move(path), 0);
    PLZMA_TESTS_ASSERT(path.count() == 0)
    array->push(item);
    PLZMA_TESTS_ASSERT(array->count() == 2)
    
    item = makeShared<Item>(path, 1);
    PLZMA_TESTS_ASSERT(path.count() == 0)
    array->push(item);
    PLZMA_TESTS_ASSERT(array->count() == 3)
    
    item = makeShared<Item>(path, 3);
    PLZMA_TESTS_ASSERT(path.count() == 0)
    array->push(item);
    PLZMA_TESTS_ASSERT(array->count() == 4)
    
    PLZMA_TESTS_ASSERT(array->count() == 4)
    PLZMA_TESTS_ASSERT(array->at(3) == item)
    
    array->sort();
        
    for (plzma_size_t i = 0, n = array->count(); i < n; i++) {
        const auto item2 = array->at(i);
        PLZMA_TESTS_ASSERT(item2->index() == i)
    }
    array->clear();
    array->clear();
    return 0;
}

int test_plzma_containers_test2(void) {
    auto map = makeShared<ItemOutStreamArray>(0);
    map->clear();
    
    Path path("path");
    auto item = makeShared<Item>(path, 1);
    auto stream = makeSharedOutStream(path);
    PLZMA_TESTS_ASSERT(path.count() > 0)
    map->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(item, stream));
    PLZMA_TESTS_ASSERT(map->count() == 1);
    auto pair = map->at(0);
    PLZMA_TESTS_ASSERT(pair.first.get() == item.get());
    PLZMA_TESTS_ASSERT(pair.second.get() == stream.get());
    item = makeShared<Item>(path, 0);
    stream = makeSharedOutStream(std::move(path));
    PLZMA_TESTS_ASSERT(path.count() == 0)
    map->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(item, stream));
    PLZMA_TESTS_ASSERT(map->count() == 2)
    pair = map->at(1);
    PLZMA_TESTS_ASSERT(pair.first.get() == item.get())
    PLZMA_TESTS_ASSERT(pair.second.get() == stream.get())
    
    path.set("path");
    item = makeShared<Item>(path, 2);
    stream = makeSharedOutStream(path);
    map->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(item, stream));
    PLZMA_TESTS_ASSERT(map->count() == 3)
    pair = map->at(2);
    PLZMA_TESTS_ASSERT(pair.first.get() == item.get())
    PLZMA_TESTS_ASSERT(pair.second.get() == stream.get())
    
    map->sort();
    auto * pairPtr = map->bsearch<plzma_size_t>(123);
    
    PLZMA_TESTS_ASSERT(pairPtr == nullptr)
    
    pair = *map->bsearch<plzma_size_t>(1);
    PLZMA_TESTS_ASSERT(pair.first.get() != nullptr)
    PLZMA_TESTS_ASSERT(pair.second.get() != nullptr)
    PLZMA_TESTS_ASSERT(pair.first->index() == 1)
    
    for (plzma_size_t i = 0, n = map->count(); i < n; i++) {
        auto pair = map->at(i);
        PLZMA_TESTS_ASSERT(pair.first->index() == i)
    }

    map->clear();
    map->clear();
    return 0;
}

int test_plzma_containers_test3(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_item_array array = plzma_item_array_create(1);
    
    plzma_path path = plzma_path_create_with_utf8_string("");
    plzma_item item = plzma_item_create_with_path(&path, 2);
    plzma_item_array_add(&array, &item);
    PLZMA_TESTS_ASSERT(plzma_item_array_count(&array) == 1)
    plzma_item item2 = plzma_item_array_at(&array, 0);
    PLZMA_TESTS_ASSERT(item2.object == item.object)
    plzma_item_release(&item);
    plzma_item_release(&item2);
    
    item = plzma_item_create_with_path(&path, 0);
    plzma_item_array_add(&array, &item);
    PLZMA_TESTS_ASSERT(plzma_item_array_count(&array) == 2)
    item2 = plzma_item_array_at(&array, 1);
    PLZMA_TESTS_ASSERT(item2.object == item.object)
    plzma_item_release(&item);
    plzma_item_release(&item2);
    
    item = plzma_item_create_with_path(&path, 1);
    plzma_item_array_add(&array, &item);
    PLZMA_TESTS_ASSERT(plzma_item_array_count(&array) == 3)
    item2 = plzma_item_array_at(&array, 2);
    PLZMA_TESTS_ASSERT(item2.object == item.object)
    plzma_item_release(&item);
    plzma_item_release(&item2);
    
    plzma_item_array_sort(&array);
        
    for (plzma_size_t i = 0, n = plzma_item_array_count(&array); i < n; i++) {
        plzma_item item2 = plzma_item_array_at(&array, i);
        PLZMA_TESTS_ASSERT(plzma_item_index(&item2) == i)
        plzma_item_release(&item2);
    }
    
    plzma_item_array_release(&array);
    plzma_path_release(&path);
#endif // !defined(LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int test_plzma_containers_test4(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_item_out_stream_array map = plzma_item_out_stream_array_create(2);
    
    plzma_path path = plzma_path_create_with_utf8_string("path");
    plzma_item item = plzma_item_create_with_path(&path, 2);
    PLZMA_TESTS_ASSERT(plzma_path_count(&path) > 0)
    plzma_out_stream stream = plzma_out_stream_create_with_path(&path);
    PLZMA_TESTS_ASSERT(plzma_path_count(&path) > 0)
    plzma_item_out_stream_array_add(&map, &item, &stream);
    PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&map) == 1)

    plzma_item_out_stream_array_pair pair = plzma_item_out_stream_array_pair_at(&map, 0);
    PLZMA_TESTS_ASSERT(pair.item.object == item.object)
    PLZMA_TESTS_ASSERT(pair.stream.object == stream.object)
    
    plzma_item_release(&item);
    plzma_out_stream_release(&stream);
    plzma_item_out_stream_array_pair_release(&pair);

    plzma_path_set_utf8_string(&path, "path");
    item = plzma_item_create_with_path(&path, 0);
    stream = plzma_out_stream_create_with_pathm(&path);
    PLZMA_TESTS_ASSERT(plzma_path_count(&path) == 0)
    plzma_item_out_stream_array_add(&map, &item, &stream);
    plzma_item_release(&item);
    plzma_out_stream_release(&stream);
    PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&map) == 2)

    item = plzma_item_create_with_path(&path, 1);
    stream = plzma_out_stream_create_with_path(&path);
    plzma_item_out_stream_array_add(&map, &item, &stream);
    plzma_item_release(&item);
    plzma_out_stream_release(&stream);
    PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&map) == 3)

    plzma_item_out_stream_array_sort(&map);
    
    for (plzma_size_t i = 0, n = plzma_item_out_stream_array_count(&map); i < n; i++) {
        plzma_item_out_stream_array_pair pair1 = plzma_item_out_stream_array_pair_at(&map, i);
        PLZMA_TESTS_ASSERT(plzma_item_index(&pair1.item) == i)
        plzma_item_out_stream_array_pair_release(&pair1);
    }

    plzma_item_out_stream_array_release(&map);
    plzma_path_release(&path);
#endif // !LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int test_plzma_containers_test5(void) {
    Path itemPath;
    Vector<String> strings1(1000), strings2(1000);
    for (plzma_size_t i = 0; i < 1000; i++) {
        String s("Hello");
        PLZMA_TESTS_ASSERT(s.count() == 5)
        strings1.push(static_cast<String &&>(s));
        PLZMA_TESTS_ASSERT(s.count() == 0)
    }
    PLZMA_TESTS_ASSERT(strings1.count() == 1000)
    for (plzma_size_t i = 0; i < 1000; i++) {
        PLZMA_TESTS_ASSERT(strings1.at(i).count() == 5)
        PLZMA_TESTS_ASSERT(strcmp(strings1.at(i).utf8(), "Hello") == 0)
    }
    PLZMA_TESTS_ASSERT(strings1.count() == 1000)
    for (plzma_size_t i = 0; i < 1000; i++) {
        String s("Hello");
        PLZMA_TESTS_ASSERT(s.count() == 5)
        strings2.push(static_cast<String &&>(s));
        PLZMA_TESTS_ASSERT(s.count() == 0)
    }
    PLZMA_TESTS_ASSERT(strings2.count() == 1000)
    for (plzma_size_t i = 0; i < 1000; i++) {
        strings2.pop();
    }
    PLZMA_TESTS_ASSERT(strings2.count() == 0)
    return 0;
}

int test_plzma_containers_test6(void) {
    auto v1 = makeShared<ItemArray>(2);
    v1->push(makeShared<Item>(Path("3"), 3));
    v1->push(makeShared<Item>(Path("1"), 1));
    v1->push(makeShared<Item>(Path("2"), 2));
    v1->sort();
    return 0;
}

int main(int argc, char* argv[]) {
    std::cout << plzma_version() << std::endl;
    int ret = 0;
    
    if ( (ret = test_plzma_containers_test1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_containers_test2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_containers_test3()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_containers_test4()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_containers_test5()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_containers_test6()) ) {
        return ret;
    }
    
//    while (1) {
//        usleep(50);
//    }
    
    return ret;
}
