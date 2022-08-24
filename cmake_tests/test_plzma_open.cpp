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


#include "plzma_public_tests.hpp"

#include <thread>

// archives
#include "../test_files/file__1_7z.h"
#include "../test_files/file__2_7z.h"
#include "../test_files/file__3_7z.h"
#include "../test_files/file__4_7z.h"
#include "../test_files/file__5_7z.h"
#include "../test_files/file__6_7z.h"
#include "../test_files/file__7_7z.h"
#include "../test_files/file__8_7z.h"
#include "../test_files/file__9_7z.h"
#include "../test_files/file__10_7z.h"
#include "../test_files/file__11_7z.h"
#include "../test_files/file__12_7z.h"
#include "../test_files/file__13_7z.h"
#include "../test_files/file__14_7z.h"
#include "../test_files/file__15_tar.h"
#include "../test_files/file__16_tar_xz.h"
#include "../test_files/file__17_jpg_xz.h"

// images
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__zombies_jpg.h"

// text files
#include "../test_files/file__names_txt.h"

using namespace plzma;

static void dummy_free_callback(void * LIBPLZMA_NULLABLE p) {
    // do nothing
}

static int itemsMustEqual(const SharedPtr<Item> & a, const SharedPtr<Item> & b) {
    PLZMA_TESTS_ASSERT(a.get() != nullptr)
    PLZMA_TESTS_ASSERT(b.get() != nullptr)
    PLZMA_TESTS_ASSERT(a->path().utf8() != nullptr)
    PLZMA_TESTS_ASSERT(b->path().utf8() != nullptr)
    PLZMA_TESTS_ASSERT(a->path().wide() != nullptr)
    PLZMA_TESTS_ASSERT(b->path().wide() != nullptr)
    PLZMA_TESTS_ASSERT(a->path().count() == b->path().count())
    PLZMA_TESTS_ASSERT(strcmp(a->path().utf8(), b->path().utf8()) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(a->path().wide(), b->path().wide()) == 0)
    PLZMA_TESTS_ASSERT(a->index() == b->index())
    PLZMA_TESTS_ASSERT(a->size() == b->size())
    PLZMA_TESTS_ASSERT(a->packSize() == b->packSize())
    PLZMA_TESTS_ASSERT(a->crc32() == b->crc32())
    PLZMA_TESTS_ASSERT(a->creationTime() == b->creationTime())
    PLZMA_TESTS_ASSERT(a->accessTime() == b->accessTime())
    PLZMA_TESTS_ASSERT(a->modificationTime() == b->modificationTime())
    PLZMA_TESTS_ASSERT(a->encrypted() == b->encrypted())
    PLZMA_TESTS_ASSERT(a->isDir() == b->isDir())

    return 0;
}

#if !defined(LIBPLZMA_NO_C_BINDINGS)
static int plzma_items_must_equal(plzma_item * a, plzma_item * b) {
    PLZMA_TESTS_ASSERT(a != nullptr)
    PLZMA_TESTS_ASSERT(b != nullptr)
    plzma_path aPath = plzma_item_path(a);
    plzma_path bPath = plzma_item_path(b);
    PLZMA_TESTS_ASSERT(a->exception == nullptr)
    PLZMA_TESTS_ASSERT(b->exception == nullptr)
    PLZMA_TESTS_ASSERT(aPath.exception == nullptr)
    PLZMA_TESTS_ASSERT(bPath.exception == nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_utf8_string(&aPath) != nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_utf8_string(&bPath) != nullptr)
    PLZMA_TESTS_ASSERT(strcmp(plzma_path_utf8_string(&aPath), plzma_path_utf8_string(&bPath)) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(plzma_path_wide_string(&aPath), plzma_path_wide_string(&bPath)) == 0)
    PLZMA_TESTS_ASSERT(plzma_item_size(a) == plzma_item_size(b))
    PLZMA_TESTS_ASSERT(plzma_item_pack_size(a) == plzma_item_pack_size(b))
    PLZMA_TESTS_ASSERT(plzma_item_crc32(a) == plzma_item_crc32(b))
    PLZMA_TESTS_ASSERT(plzma_item_creation_time(a) == plzma_item_creation_time(b))
    PLZMA_TESTS_ASSERT(plzma_item_access_time(a) == plzma_item_access_time(b))
    PLZMA_TESTS_ASSERT(plzma_item_modification_time(a) == plzma_item_modification_time(b))
    PLZMA_TESTS_ASSERT(plzma_item_encrypted(a) == plzma_item_encrypted(b))
    PLZMA_TESTS_ASSERT(plzma_item_is_dir(a) == plzma_item_is_dir(b))

    plzma_path_release(&aPath);
    plzma_path_release(&bPath);
    
    return 0;
}
#endif // !LIBPLZMA_NO_C_BINDINGS

int test_plzma_open_1(void) {
    const size_t filesCount = 15;
    unsigned char * files[filesCount + 1] = {
        nullptr,
        FILE__1_7z_PTR,
        FILE__2_7z_PTR,
        FILE__3_7z_PTR,
        FILE__4_7z_PTR,
        FILE__5_7z_PTR,
        FILE__6_7z_PTR,
        FILE__7_7z_PTR,
        FILE__8_7z_PTR,
        FILE__9_7z_PTR,
        FILE__10_7z_PTR,
        FILE__11_7z_PTR,
        FILE__12_7z_PTR,
        FILE__13_7z_PTR,
        FILE__14_7z_PTR,
        FILE__15_tar_PTR
    };
    size_t fileSizes[filesCount + 1] = {
        0,
        FILE__1_7z_SIZE,
        FILE__2_7z_SIZE,
        FILE__3_7z_SIZE,
        FILE__4_7z_SIZE,
        FILE__5_7z_SIZE,
        FILE__6_7z_SIZE,
        FILE__7_7z_SIZE,
        FILE__8_7z_SIZE,
        FILE__9_7z_SIZE,
        FILE__10_7z_SIZE,
        FILE__11_7z_SIZE,
        FILE__12_7z_SIZE,
        FILE__13_7z_SIZE,
        FILE__14_7z_SIZE,
        FILE__15_tar_SIZE
    };
    
    for (size_t fileIndex = 1; fileIndex <= filesCount; fileIndex++) {
        auto stream = makeSharedInStream(files[fileIndex], fileSizes[fileIndex], &dummy_free_callback);
        SharedPtr<Decoder> decoder;
        switch (fileIndex) {
            case 15: // FILE__15_tar...
#if defined(LIBPLZMA_NO_TAR)
                continue;
#else
                decoder = makeSharedDecoder(stream, plzma_file_type_tar, plzma_context{nullptr, nullptr});
#endif
                break;
#if defined(LIBPLZMA_NO_CRYPTO)
            case 5:
            case 6:
            case 7:
            case 8:
            case 10:
            case 11:
            case 13:
            case 14:
                continue;
#endif
            default:
                decoder = makeSharedDecoder(stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
                break;
        }
        
        bool didCatchExpectedException = false;
        try { decoder->setPassword("1234"); } catch (...) { didCatchExpectedException = true; }
#if defined(LIBPLZMA_NO_CRYPTO)
        PLZMA_TESTS_ASSERT(didCatchExpectedException == true)
#endif
        
        auto allItems = decoder->items();
        PLZMA_TESTS_ASSERT(allItems.get() == nullptr)
        decoder->open();
        PLZMA_TESTS_ASSERT(decoder->count() == 5)
        allItems = decoder->items();
        PLZMA_TESTS_ASSERT(allItems.get() != nullptr)
        PLZMA_TESTS_ASSERT(allItems->count() == 5)

        for (plzma_size_t itemIndex = 0, n = decoder->count(); itemIndex < n; itemIndex++) {
            const auto item = decoder->itemAt(itemIndex);
            PLZMA_TESTS_ASSERT(item->index() == itemIndex)
            PLZMA_TESTS_ASSERT(item->path().utf8() != nullptr)
            PLZMA_TESTS_ASSERT(item->path().wide() != nullptr)
            PLZMA_TESTS_ASSERT(item->size() > 0)
            switch (fileIndex) {
                case 15: // FILE__15_tar...
#if !defined(LIBPLZMA_NO_TAR)
                    PLZMA_TESTS_ASSERT(item->crc32() == 0)
#endif
                    break;
                    
                default:
                    PLZMA_TESTS_ASSERT(item->crc32() > 0)
                    break;
            }
            PLZMA_TESTS_ASSERT(item->isDir() == false)
            PLZMA_TESTS_ASSERT(itemsMustEqual(item, allItems->at(itemIndex)) == 0)
            switch (fileIndex) {
                case 5:
                case 6:
                case 7:
                case 8:
                case 10:
                case 11:
                case 13:
                case 14:
                    PLZMA_TESTS_ASSERT(item->encrypted() == true)
                    break;

                default:
                    break;
            }
        }
    }

#if !defined(LIBPLZMA_NO_C_BINDINGS)
    for (size_t fileIndex = 1; fileIndex <= filesCount; fileIndex++) {
        plzma_in_stream stream = plzma_in_stream_create_with_memory(files[fileIndex], fileSizes[fileIndex], &dummy_free_callback);
        PLZMA_TESTS_ASSERT(stream.exception == nullptr)
        PLZMA_TESTS_ASSERT(stream.object != nullptr)
        plzma_decoder decoder;
        switch (fileIndex) {
            case 15: // FILE__15_tar...
#if defined(LIBPLZMA_NO_TAR)
                continue;
#else
                decoder = plzma_decoder_create(&stream, plzma_file_type_tar, plzma_context{nullptr, nullptr});
#endif
                break;
#if defined(LIBPLZMA_NO_CRYPTO)
            case 5:
            case 6:
            case 7:
            case 8:
            case 10:
            case 11:
            case 13:
            case 14:
                continue;
#endif
            default:
                decoder = plzma_decoder_create(&stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
                break;
        }
        plzma_in_stream_release(&stream);
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        PLZMA_TESTS_ASSERT(decoder.object != nullptr)
        plzma_decoder_set_password_wide_string(&decoder, L"1234");
#if defined(LIBPLZMA_NO_CRYPTO)
        PLZMA_TESTS_ASSERT(decoder.exception != nullptr)
        plzma_exception_release(decoder.exception);
        decoder.exception = nullptr;
#else
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
#endif
        plzma_item_array allItems = plzma_decoder_items(&decoder);
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        PLZMA_TESTS_ASSERT(allItems.object == nullptr)
        PLZMA_TESTS_ASSERT(allItems.exception == nullptr)
        plzma_item_array_release(&allItems);
        PLZMA_TESTS_ASSERT(plzma_decoder_open(&decoder) == true)
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        
        allItems = plzma_decoder_items(&decoder);
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        PLZMA_TESTS_ASSERT(allItems.object != nullptr)
        PLZMA_TESTS_ASSERT(allItems.exception == nullptr)
        PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)
        PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
        PLZMA_TESTS_ASSERT(plzma_item_array_count(&allItems) == 5)
        PLZMA_TESTS_ASSERT(allItems.exception == nullptr)
        
        for (plzma_size_t itemIndex = 0, n = plzma_decoder_count(&decoder); itemIndex < n; itemIndex++) {
            plzma_item item = plzma_decoder_item_at(&decoder, itemIndex);
            PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
            PLZMA_TESTS_ASSERT(item.object != nullptr)
            PLZMA_TESTS_ASSERT(item.exception == nullptr)
            PLZMA_TESTS_ASSERT(plzma_item_index(&item) == itemIndex)
            
            plzma_path itemPath = plzma_item_path(&item);
            PLZMA_TESTS_ASSERT(item.exception == nullptr)
            PLZMA_TESTS_ASSERT(itemPath.object != nullptr)
            PLZMA_TESTS_ASSERT(itemPath.exception == nullptr)
            PLZMA_TESTS_ASSERT(plzma_path_utf8_string(&itemPath) != nullptr)
            PLZMA_TESTS_ASSERT(plzma_path_wide_string(&itemPath) != nullptr)
            plzma_path_release(&itemPath);
            
            PLZMA_TESTS_ASSERT(plzma_item_size(&item) > 0)
            switch (fileIndex) {
                case 15: // FILE__15_tar...
#if !defined(LIBPLZMA_NO_TAR)
                    PLZMA_TESTS_ASSERT(plzma_item_crc32(&item) == 0)
#endif
                    break;

                default:
                    PLZMA_TESTS_ASSERT(plzma_item_crc32(&item) > 0)
                    break;
            }
            PLZMA_TESTS_ASSERT(plzma_item_is_dir(&item) == false)
            
            plzma_item itemFromAllItems = plzma_item_array_at(&allItems, itemIndex);
            PLZMA_TESTS_ASSERT(itemFromAllItems.object != nullptr)
            PLZMA_TESTS_ASSERT(itemFromAllItems.exception == nullptr)

            PLZMA_TESTS_ASSERT(plzma_items_must_equal(&item, &itemFromAllItems) == 0)
            plzma_item_release(&itemFromAllItems);
            switch (fileIndex) {
                case 5:
                case 6:
                case 7:
                case 8:
                case 10:
                case 11:
                case 13:
                case 14:
                    PLZMA_TESTS_ASSERT(plzma_item_encrypted(&item) == true)
                    break;

                default:
                    break;
            }
            plzma_item_release(&item);
        }
        plzma_item_array_release(&allItems);
        plzma_decoder_release(&decoder);
    }
#endif // !LIBPLZMA_NO_C_BINDINGS
    
    return 0;
}

int test_plzma_open_2(void) {
    auto stream = makeSharedInStream(FILE__1_7z_PTR, FILE__1_7z_SIZE, &dummy_free_callback);
    auto decoder = makeSharedDecoder(stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    bool didCatchExpectedException = false;
    try { decoder->setPassword("1234"); } catch (...) { didCatchExpectedException = true; }
#if defined(LIBPLZMA_NO_CRYPTO)
    PLZMA_TESTS_ASSERT(didCatchExpectedException == true)
    return 0;
#endif
    bool opened = false;
    std::thread thread([&](){
        opened = decoder->open();
    });
    thread.join();
    PLZMA_TESTS_ASSERT(opened == true)
    PLZMA_TESTS_ASSERT(decoder->count() == 5)
    
    return 0;
}

int test_plzma_open_3(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_in_stream stream = plzma_in_stream_create_with_memory(FILE__1_7z_PTR, FILE__1_7z_SIZE, &dummy_free_callback);
    plzma_decoder decoder = plzma_decoder_create(&stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    plzma_decoder_set_password_utf8_string(&decoder, "1234");
#if defined(LIBPLZMA_NO_CRYPTO)
    PLZMA_TESTS_ASSERT(decoder.exception != nullptr)
    plzma_exception_release(decoder.exception);
    decoder.exception = nullptr;
    return 0;
#else
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
#endif
    bool opened = false;
    std::thread thread([&](){
        opened = plzma_decoder_open(&decoder);
    });
    thread.join();
    PLZMA_TESTS_ASSERT(opened == true)
    PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)
    plzma_in_stream_release(&stream);
    plzma_decoder_release(&decoder);
#endif
    return 0;
}

int test_plzma_open_cpp_doc(void) {
#if !defined(LIBPLZMA_NO_CRYPTO)
    try {
        Path archivePath("path/to/archive"); // Path(L"C:\\\\path\\to\\archive");
        auto archivePathInStream = makeSharedInStream(archivePath /* std::move(archivePath) */);
        
        auto archiveDataInStream = makeSharedInStream(FILE__7_7z_PTR, FILE__7_7z_SIZE);
        
        auto decoder = makeSharedDecoder(archiveDataInStream, plzma_file_type_7z);
        decoder->setPassword("1234"); // decoder->setPassword(L"1234");
        
        bool opened = decoder->open();
        PLZMA_TESTS_ASSERT(opened == true)
        
        auto allArchiveItems = decoder->items();
        
        plzma_size_t numberOfArchiveItems = decoder->count();
        auto selectedItemsDuringIteration = makeShared<ItemArray>(numberOfArchiveItems);
        auto selectedItemsToStreams = makeShared<ItemOutStreamArray>();
        for (plzma_size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
            auto item = decoder->itemAt(itemIndex);
            selectedItemsDuringIteration->push(item /* std::move(item) */);
            selectedItemsToStreams->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(item, makeSharedOutStream()));
        }
        PLZMA_TESTS_ASSERT(numberOfArchiveItems == 5)
        PLZMA_TESTS_ASSERT(numberOfArchiveItems == allArchiveItems->count())
        PLZMA_TESTS_ASSERT(numberOfArchiveItems == selectedItemsDuringIteration->count())
        PLZMA_TESTS_ASSERT(numberOfArchiveItems == selectedItemsToStreams->count())
        
        //bool extracted = decoder->extract(Path("path/outdir"));
        //bool extracted = decoder->extract(selectedItemsDuringIteration, Path("path/outdir"));
        bool extracted = decoder->extract(selectedItemsToStreams);
        PLZMA_TESTS_ASSERT(extracted == true)
    } catch (const Exception & exception) {
        std::cout << "Exception: " << exception.what() << std::endl;
    }
#endif
    return 0;
}

int test_plzma_open_c_doc(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS) && !defined(LIBPLZMA_NO_CRYPTO)
    plzma_path archivePath = plzma_path_create_with_utf8_string("path/to/archive"); // plzma_path_create_with_wide_string(L"C:\\\\path\\to\\archive");
    
    plzma_in_stream archivePathInStream = plzma_in_stream_create_with_path(&archivePath); // plzma_in_stream_create_with_pathm(...);
    plzma_path_release(&archivePath);
    plzma_in_stream_release(&archivePathInStream); //
    
    plzma_in_stream archiveDataInStream = plzma_in_stream_create_with_memory_copy(FILE__7_7z_PTR, FILE__7_7z_SIZE); // plzma_in_stream_create_with_memory(...);
    plzma_decoder decoder = plzma_decoder_create(&archiveDataInStream, plzma_file_type_7z, plzma_context{ nullptr, nullptr }); // C2059 = { .context = nullptr, .deinitializer = nullptr }
    plzma_in_stream_release(&archiveDataInStream);
    
    // plzma_decoder_set_progress_delegate_utf8_callback(&decoder, <UTF8 CALLBACK>);  // plzma_decoder_set_progress_delegate_wide_callback(...);
    plzma_decoder_set_password_utf8_string(&decoder, "1234"); // plzma_decoder_set_password_wide_string(&decoder, L"1234");
    
    bool opened = plzma_decoder_open(&decoder);
    PLZMA_TESTS_ASSERT(opened == true)
    
    plzma_item_array allArchiveItems = plzma_decoder_items(&decoder);
    
    plzma_size_t numberOfArchiveItems = plzma_decoder_count(&decoder);
    plzma_item_array selectedItemsDuringIteration = plzma_item_array_create(numberOfArchiveItems);
    plzma_item_out_stream_array selectedItemsToStreams = plzma_item_out_stream_array_create(0);
    for (plzma_size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
        plzma_item item = plzma_decoder_item_at(&decoder, itemIndex);
        plzma_item_array_add(&selectedItemsDuringIteration, &item);
        plzma_out_stream outItemStream = plzma_out_stream_create_memory_stream();
        plzma_item_out_stream_array_add(&selectedItemsToStreams, &item, &outItemStream);
        plzma_out_stream_release(&outItemStream);
        plzma_item_release(&item);
    }
    
    PLZMA_TESTS_ASSERT(numberOfArchiveItems == 5)
    PLZMA_TESTS_ASSERT(numberOfArchiveItems == plzma_item_array_count(&allArchiveItems))
    PLZMA_TESTS_ASSERT(numberOfArchiveItems == plzma_item_array_count(&selectedItemsDuringIteration))
    PLZMA_TESTS_ASSERT(numberOfArchiveItems == plzma_item_out_stream_array_count(&selectedItemsToStreams))
    
//    plzma_path extractPath = plzma_path_create_with_utf8_string("path/outdir");
//    bool extracted = plzma_decoder_extract_all_items_to_path(&decoder, &extractPath, true);
//    plzma_path_release(&extractPath);
    
//    plzma_path extractPath = plzma_path_create_with_utf8_string("path/outdir");
//    bool extracted = plzma_decoder_extract_items_to_path(&decoder, &selectedItemsDuringIteration, &extractPath, true);
//    plzma_path_release(&extractPath);
    
    bool extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &selectedItemsToStreams);
    PLZMA_TESTS_ASSERT(extracted == true)
    
    plzma_item_array_release(&selectedItemsDuringIteration);
    plzma_item_array_release(&allArchiveItems);
    plzma_item_out_stream_array_release(&selectedItemsToStreams);
    plzma_decoder_release(&decoder);
#endif
    return 0;
}

int main(int argc, char* argv[]) {
    std::cout << plzma_version() << std::endl;
    int ret = 0;

    if ( (ret = test_plzma_open_1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_open_2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_open_3()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_open_cpp_doc()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_open_c_doc()) ) {
        return ret;
    }
        
//    while (1) {
//        usleep(50);
//    }
    
	return ret;
}

// archives
#include "../test_files/file__1_7z.h"
#include "../test_files/file__2_7z.h"
#include "../test_files/file__3_7z.h"
#include "../test_files/file__4_7z.h"
#include "../test_files/file__5_7z.h"
#include "../test_files/file__6_7z.h"
#include "../test_files/file__7_7z.h"
#include "../test_files/file__8_7z.h"
#include "../test_files/file__9_7z.h"
#include "../test_files/file__10_7z.h"
#include "../test_files/file__11_7z.h"
#include "../test_files/file__12_7z.h"
#include "../test_files/file__13_7z.h"
#include "../test_files/file__14_7z.h"
#include "../test_files/file__15_tar.h"
#include "../test_files/file__16_tar_xz.h"
#include "../test_files/file__17_jpg_xz.h"

// images
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__zombies_jpg.h"

// text files
#include "../test_files/file__names_txt.h"
