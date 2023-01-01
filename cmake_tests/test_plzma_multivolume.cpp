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
#include "../test_files/file__18_7z_001.h"
#include "../test_files/file__18_7z_002.h"
#include "../test_files/file__18_7z_003.h"

// images
#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__zombies_jpg.h"

static void dummyFreeCallback(void * LIBPLZMA_NULLABLE memory) {
    (void)memory;
}

using namespace plzma;

int test_plzma_multivolume_test1(void) {
#if !defined(LIBPLZMA_NO_CRYPTO)
    InStreamArray streams(3);
    streams.push(makeSharedInStream(FILE__18_7z_001_PTR, FILE__18_7z_001_SIZE));
    
    const Path secondPartPath = Path::tmpPath().appendingRandomComponent();
    PLZMA_TESTS_ASSERT(secondPartPath.exists() == false)
    FILE__18_7z_002_write_to_path(secondPartPath.utf8());
    PLZMA_TESTS_ASSERT(secondPartPath.exists() == true)
    streams.push(makeSharedInStream(secondPartPath));
    //streams.push(makeSharedInStream(FILE__18_7z_002_PTR, FILE__18_7z_002_SIZE));
    
    streams.push(makeSharedInStream(FILE__18_7z_003_PTR, FILE__18_7z_003_SIZE));
    
    PLZMA_TESTS_ASSERT(streams.count() == 3)
    auto inStream = makeSharedInStream(static_cast<InStreamArray &&>(streams));
    PLZMA_TESTS_ASSERT(streams.count() == 0)
    PLZMA_TESTS_ASSERT(inStream.get() != nullptr)
    
    auto decoder = makeSharedDecoder(inStream, plzma_file_type_7z);
    decoder->setPassword("1234");
    bool opened = decoder->open();
    PLZMA_TESTS_ASSERT(opened == true)
    PLZMA_TESTS_ASSERT(decoder->count() == 5)
    
    auto memoryMap = makeShared<ItemOutStreamArray>(5);
    auto fileMap = makeShared<ItemOutStreamArray>(5);
    for (plzma_size_t i = 0; i < 5; i++) {
        memoryMap->push(ItemOutStreamArray::ElementType(decoder->itemAt(i), makeSharedOutStream()));
        fileMap->push(ItemOutStreamArray::ElementType(decoder->itemAt(i), makeSharedOutStream(Path::tmpPath().appendingRandomComponent())));
    }
    PLZMA_TESTS_ASSERT(memoryMap->count() == 5)
    PLZMA_TESTS_ASSERT(fileMap->count() == 5)
    
    bool extracted = decoder->extract(memoryMap);
    PLZMA_TESTS_ASSERT(extracted == true)
    extracted = decoder->extract(fileMap);
    PLZMA_TESTS_ASSERT(extracted == true)
    
    for (plzma_size_t i = 0; i < 10; i++) {
        auto pair = (i < 5) ? memoryMap->at(i) : fileMap->at(i - 5);
        const auto content = pair.second->copyContent();
        bool erased = pair.second->erase(plzma_erase_zero);
        PLZMA_TESTS_ASSERT(erased == true)
        if (pair.first->path() == "shutuptakemoney.jpg") {
            PLZMA_TESTS_ASSERT(pair.first->size() == FILE__shutuptakemoney_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.second == FILE__shutuptakemoney_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE) == 0);
        } else if (pair.first->path() == "SouthPark.jpg") {
            PLZMA_TESTS_ASSERT(pair.first->size() == FILE__southpark_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.second == FILE__southpark_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE) == 0);
        } else if (pair.first->path() == "zombies.jpg") {
            PLZMA_TESTS_ASSERT(pair.first->size() == FILE__zombies_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.second == FILE__zombies_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE) == 0);
        } else if (pair.first->path() == "Мюнхен.jpg" || pair.first->path() == "München.jpg") {
            PLZMA_TESTS_ASSERT(pair.first->size() == FILE__munchen_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.second == FILE__munchen_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) == 0);
        } else {
            PLZMA_TESTS_ASSERT(false)
        }
    }
    
    decoder.clear();
    
    PLZMA_TESTS_ASSERT(secondPartPath.remove() == true)
    PLZMA_TESTS_ASSERT(secondPartPath.exists() == false)
#endif
    
    return 0;
}

int test_plzma_multivolume_test2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS) && !defined(LIBPLZMA_NO_CRYPTO)
    plzma_in_stream_array streams = plzma_in_stream_array_create_with_capacity(3);
    PLZMA_TESTS_ASSERT(streams.exception == nullptr)
    PLZMA_TESTS_ASSERT(plzma_in_stream_array_count(&streams) == 0)
    
    plzma_in_stream tmpInStream = plzma_in_stream_create_with_memory(FILE__18_7z_001_PTR, FILE__18_7z_001_SIZE, dummyFreeCallback);
    PLZMA_TESTS_ASSERT(tmpInStream.exception == nullptr)
    plzma_in_stream_array_add(&streams, &tmpInStream);
    PLZMA_TESTS_ASSERT(streams.exception == nullptr)
    plzma_in_stream_release(&tmpInStream);
    
    plzma_path secondPartPath = plzma_path_create_with_tmp_dir();
    PLZMA_TESTS_ASSERT(secondPartPath.exception == nullptr)
    plzma_path_append_random_component(&secondPartPath);
    PLZMA_TESTS_ASSERT(secondPartPath.exception == nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&secondPartPath, nullptr) == false)
    PLZMA_TESTS_ASSERT(secondPartPath.exception == nullptr)
    FILE__18_7z_002_write_to_path(plzma_path_utf8_string(&secondPartPath));
    PLZMA_TESTS_ASSERT(plzma_path_exists(&secondPartPath, nullptr) == true)
    tmpInStream = plzma_in_stream_create_with_path(&secondPartPath);
    PLZMA_TESTS_ASSERT(tmpInStream.exception == nullptr)
    plzma_in_stream_array_add(&streams, &tmpInStream);
    PLZMA_TESTS_ASSERT(streams.exception == nullptr)
    plzma_in_stream_release(&tmpInStream);
    
    tmpInStream = plzma_in_stream_create_with_memory(FILE__18_7z_003_PTR, FILE__18_7z_003_SIZE, dummyFreeCallback);
    PLZMA_TESTS_ASSERT(tmpInStream.exception == nullptr)
    plzma_in_stream_array_add(&streams, &tmpInStream);
    PLZMA_TESTS_ASSERT(streams.exception == nullptr)
    plzma_in_stream_release(&tmpInStream);
    
    PLZMA_TESTS_ASSERT(plzma_in_stream_array_count(&streams) == 3)
    
    plzma_in_stream inStream = plzma_in_stream_create_with_stream_arraym(&streams);
    PLZMA_TESTS_ASSERT(inStream.exception == nullptr)
    plzma_in_stream_array_release(&streams);
    
    plzma_decoder decoder = plzma_decoder_create(&inStream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
    plzma_decoder_set_password_utf8_string(&decoder, "1234");
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
    bool opened = plzma_decoder_open(&decoder);
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
    PLZMA_TESTS_ASSERT(opened == true)
    PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)

    plzma_item_out_stream_array memoryMap = plzma_item_out_stream_array_create(5);
    PLZMA_TESTS_ASSERT(memoryMap.exception == nullptr)
    plzma_item_out_stream_array fileMap = plzma_item_out_stream_array_create(5);
    PLZMA_TESTS_ASSERT(fileMap.exception == nullptr)
    for (plzma_size_t i = 0; i < 5; i++) {
        plzma_item tmpItem = plzma_decoder_item_at(&decoder, i);
        PLZMA_TESTS_ASSERT(tmpItem.exception == nullptr)
        
        plzma_out_stream tmpOutStream = plzma_out_stream_create_memory_stream();
        PLZMA_TESTS_ASSERT(tmpOutStream.exception == nullptr)
        plzma_item_out_stream_array_add(&memoryMap, &tmpItem, &tmpOutStream);
        PLZMA_TESTS_ASSERT(memoryMap.exception == nullptr)
        plzma_out_stream_release(&tmpOutStream);
        
        plzma_path tmpPath = plzma_path_create_with_tmp_dir();
        PLZMA_TESTS_ASSERT(tmpPath.exception == nullptr)
        plzma_path_append_random_component(&tmpPath);
        PLZMA_TESTS_ASSERT(tmpPath.exception == nullptr)
        tmpOutStream = plzma_out_stream_create_with_pathm(&tmpPath);
        PLZMA_TESTS_ASSERT(tmpOutStream.exception == nullptr)
        plzma_path_release(&tmpPath);
        
        plzma_item_out_stream_array_add(&fileMap, &tmpItem, &tmpOutStream);
        PLZMA_TESTS_ASSERT(fileMap.exception == nullptr)
        plzma_out_stream_release(&tmpOutStream);
        plzma_item_release(&tmpItem);
    }
    
    PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&memoryMap) == 5)
    PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&fileMap) == 5)

    bool extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &memoryMap);
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
    PLZMA_TESTS_ASSERT(extracted == true)
    extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &fileMap);
    PLZMA_TESTS_ASSERT(decoder.exception == nullptr)
    PLZMA_TESTS_ASSERT(extracted == true)

    for (plzma_size_t i = 0; i < 10; i++) {
        plzma_item_out_stream_array_pair pair = (i < 5) ? plzma_item_out_stream_array_pair_at(&memoryMap, i) : plzma_item_out_stream_array_pair_at(&fileMap, i - 5);
        PLZMA_TESTS_ASSERT(memoryMap.exception == nullptr)
        PLZMA_TESTS_ASSERT(fileMap.exception == nullptr)
        plzma_memory content = plzma_out_stream_copy_content(&pair.stream);
        bool erased = plzma_out_stream_erase(&pair.stream, plzma_erase_zero);
        PLZMA_TESTS_ASSERT(pair.stream.exception == nullptr)
        PLZMA_TESTS_ASSERT(erased == true)
        plzma_path itemPath = plzma_item_path(&pair.item);
        PLZMA_TESTS_ASSERT(itemPath.exception == nullptr)
        if (strcmp(plzma_path_utf8_string(&itemPath), "shutuptakemoney.jpg") == 0) {
            PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__shutuptakemoney_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.size == FILE__shutuptakemoney_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE) == 0);
        } else if (strcmp(plzma_path_utf8_string(&itemPath), "SouthPark.jpg") == 0) {
            PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__southpark_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.size == FILE__southpark_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE) == 0);
        } else if (strcmp(plzma_path_utf8_string(&itemPath), "zombies.jpg") == 0) {
            PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__zombies_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.size == FILE__zombies_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE) == 0);
        } else if (strcmp(plzma_path_utf8_string(&itemPath), "Мюнхен.jpg") == 0 ||
                   strcmp(plzma_path_utf8_string(&itemPath), "München.jpg") == 0) {
            PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__munchen_jpg_SIZE)
            PLZMA_TESTS_ASSERT(content.size == FILE__munchen_jpg_SIZE)
            PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) == 0);
        } else {
            PLZMA_TESTS_ASSERT(false)
        }
        plzma_path_release(&itemPath);
        plzma_free(content.memory);
        plzma_item_out_stream_array_pair_release(&pair);
    }
    
    plzma_decoder_abort(&decoder);
    
    plzma_item_out_stream_array_release(&memoryMap);
    plzma_item_out_stream_array_release(&fileMap);
    plzma_in_stream_release(&inStream);
    plzma_decoder_release(&decoder);
    
    PLZMA_TESTS_ASSERT(plzma_path_remove(&secondPartPath, false) == true)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&secondPartPath, nullptr) == false)
    
    plzma_path_release(&secondPartPath);
    
#endif // !LIBPLZMA_NO_C_BINDINGS && !LIBPLZMA_NO_CRYPTO
    return 0;
}

int test_plzma_multivolume_test3(void) {
    auto multiStream = makeSharedOutMultiStream(1024);
    PLZMA_TESTS_ASSERT(multiStream.get() != nullptr)
    PLZMA_TESTS_ASSERT(multiStream->opened() == false)
    PLZMA_TESTS_ASSERT(multiStream->erase() == true)
    auto content = multiStream->copyContent();
    PLZMA_TESTS_ASSERT(static_cast<const void *>(content.first) == nullptr)
    PLZMA_TESTS_ASSERT(content.second == 0)
    auto subStreams = multiStream->streams();
    PLZMA_TESTS_ASSERT(subStreams.count() == 0)
    
    auto stream = multiStream.cast<OutStream>();
    PLZMA_TESTS_ASSERT(stream.get() != nullptr)
    PLZMA_TESTS_ASSERT(stream->opened() == false)
    PLZMA_TESTS_ASSERT(stream->erase() == true)
    content = stream->copyContent();
    PLZMA_TESTS_ASSERT(static_cast<const void *>(content.first) == nullptr)
    PLZMA_TESTS_ASSERT(content.second == 0)
    
    auto encoder = makeSharedEncoder(stream, plzma_file_type_7z, plzma_method_LZMA);
    PLZMA_TESTS_ASSERT(encoder.get() != nullptr)
    
    encoder = makeSharedEncoder(multiStream, plzma_file_type_7z, plzma_method_LZMA2);
    PLZMA_TESTS_ASSERT(encoder.get() != nullptr)
    
    return 0;
}

enum Test4Case {
    Test4CaseFileNoPassword =   0,
    Test4CaseMemNoPassword =    1,
    Test4CaseFileWithPassword = 2,
    Test4CaseMemWithPassword =  3,
    
    Test4CaseCount =            4
};

int test_plzma_multivolume_test4(void) {
#if !defined(LIBPLZMA_NO_CRYPTO)
    for (int caseIndex = 0; caseIndex < static_cast<int>(Test4CaseCount); caseIndex++) {
        const Test4Case testCase = static_cast<Test4Case>(caseIndex);
        
        auto path = Path::tmpPath();
        path.appendRandomComponent();
        const size_t partSize = 32 * 1024;
        SharedPtr<OutMultiStream> multiStream;
        switch (testCase) {
            case Test4CaseFileNoPassword:
            case Test4CaseFileWithPassword:
                multiStream = makeSharedOutMultiStream(path, "file", "7z", plzma_plzma_multi_stream_part_name_format_name_ext_00x, partSize);
                break;
            case Test4CaseMemNoPassword:
            case Test4CaseMemWithPassword:
                multiStream = makeSharedOutMultiStream(partSize);
                break;
            default: break;
        }
        PLZMA_TESTS_ASSERT(multiStream.get() != nullptr)
        
        auto encoder = makeSharedEncoder(multiStream, plzma_file_type_7z, plzma_method_LZMA);
        
        switch (testCase) {
            case Test4CaseFileWithPassword:
            case Test4CaseMemWithPassword:
#if defined(LIBPLZMA_OS_WINDOWS)
                encoder->setPassword(L"hello");
#else
                encoder->setPassword("hello");
#endif
                break;
            default: break;
        }
        
        encoder->add(makeSharedInStream(FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE) , "shutuptakemoney.jpg");
        encoder->add(makeSharedInStream(FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE) , "SouthPark.jpg");
        encoder->add(makeSharedInStream(FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE) , "zombies.jpg");
        encoder->add(makeSharedInStream(FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) , "Мюнхен.jpg");
        encoder->add(makeSharedInStream(FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) , "München.jpg");
        bool opened = encoder->open();
        PLZMA_TESTS_ASSERT(opened == true)
        bool compressed = encoder->compress();
        PLZMA_TESTS_ASSERT(compressed == true)
        encoder.clear();
        
        auto content = multiStream->copyContent();
        
        uint64_t totalPartsSize = 0;
        switch (testCase) {
            case Test4CaseFileNoPassword:
            case Test4CaseFileWithPassword: {
                auto pathIterator = path.openDir();
                while (pathIterator->next()) {
                    const auto fullPath = pathIterator->fullPath();
                    std::cout << fullPath.utf8() << std::endl;
                    auto partStat = fullPath.stat();
                    totalPartsSize += partStat.size;
                }
                std::cout << std::endl;
                PLZMA_TESTS_ASSERT(content.second == totalPartsSize)
            } break;
                
            default: break;
        }
        
        OutStreamArray outMultiStreams = multiStream->streams();
        PLZMA_TESTS_ASSERT(outMultiStreams.count() > 0)
        
        totalPartsSize = 0;
        for (plzma_size_t i = 0, n = outMultiStreams.count(); i < n; i++) {
            const auto partContent = outMultiStreams.at(i)->copyContent();
            if (i + 1 == n) {
                PLZMA_TESTS_ASSERT(partContent.second <= partSize)
            } else {
                PLZMA_TESTS_ASSERT(partContent.second == partSize)
            }
            totalPartsSize += partContent.second;
        }
        PLZMA_TESTS_ASSERT(static_cast<const void *>(content.first) != nullptr)
        PLZMA_TESTS_ASSERT(content.second > 0)
        PLZMA_TESTS_ASSERT(content.second == totalPartsSize)
        bool erased = multiStream->erase();
        PLZMA_TESTS_ASSERT(erased == true)
        
        auto decoder = makeSharedDecoder(makeSharedInStream(static_cast<void *>(content.first), content.second, dummyFreeCallback), plzma_file_type_7z);
        switch (testCase) {
            case Test4CaseFileWithPassword:
            case Test4CaseMemWithPassword:
#if defined(LIBPLZMA_OS_WINDOWS)
                decoder->setPassword(L"hello");
#else
                decoder->setPassword("hello");
#endif
                break;
            default: break;
        }
        opened = decoder->open();
        PLZMA_TESTS_ASSERT(opened == true)
        PLZMA_TESTS_ASSERT(decoder->count() == 5)
        
        auto memoryMap = makeShared<ItemOutStreamArray>(5);
        auto fileMap = makeShared<ItemOutStreamArray>(5);
        for (plzma_size_t i = 0; i < 5; i++) {
            memoryMap->push(ItemOutStreamArray::ElementType(decoder->itemAt(i), makeSharedOutStream()));
            fileMap->push(ItemOutStreamArray::ElementType(decoder->itemAt(i), makeSharedOutStream(Path::tmpPath().appendingRandomComponent())));
        }
        PLZMA_TESTS_ASSERT(memoryMap->count() == 5)
        PLZMA_TESTS_ASSERT(fileMap->count() == 5)
        
        bool extracted = decoder->extract(memoryMap);
        PLZMA_TESTS_ASSERT(extracted == true)
        extracted = decoder->extract(fileMap);
        PLZMA_TESTS_ASSERT(extracted == true)
        
        for (plzma_size_t i = 0; i < 10; i++) {
            auto pair = (i < 5) ? memoryMap->at(i) : fileMap->at(i - 5);
            const auto content = pair.second->copyContent();
            bool erased = pair.second->erase(plzma_erase_zero);
            PLZMA_TESTS_ASSERT(erased == true)
            if (pair.first->path() == "shutuptakemoney.jpg") {
                PLZMA_TESTS_ASSERT(pair.first->size() == FILE__shutuptakemoney_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.second == FILE__shutuptakemoney_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE) == 0);
            } else if (pair.first->path() == "SouthPark.jpg") {
                PLZMA_TESTS_ASSERT(pair.first->size() == FILE__southpark_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.second == FILE__southpark_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE) == 0);
            } else if (pair.first->path() == "zombies.jpg") {
                PLZMA_TESTS_ASSERT(pair.first->size() == FILE__zombies_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.second == FILE__zombies_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE) == 0);
            } else if (pair.first->path() == "Мюнхен.jpg" || pair.first->path() == "München.jpg") {
                PLZMA_TESTS_ASSERT(pair.first->size() == FILE__munchen_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.second == FILE__munchen_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(content.first), FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) == 0);
            } else {
                PLZMA_TESTS_ASSERT(false)
            }
        }
        
        decoder.clear();
        path.remove();
        
        for (plzma_size_t i = 0; i < 5; i++) {
            memoryMap->at(i).second->erase();
            fileMap->at(i).second->erase();
        }
    }
#endif
    
    return 0;
}

int test_plzma_multivolume_test5(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS) && !defined(LIBPLZMA_NO_CRYPTO)
    for (int caseIndex = 0; caseIndex < static_cast<int>(Test4CaseCount); caseIndex++) {
        const Test4Case testCase = static_cast<Test4Case>(caseIndex);
        
        plzma_path path = plzma_path_create_with_tmp_dir();
        plzma_path_append_random_component(&path);
        const size_t partSize = 32 * 1024;
        plzma_out_multi_stream multiStream;
        switch (testCase) {
            case Test4CaseFileNoPassword:
            case Test4CaseFileWithPassword:
#if defined(LIBPLZMA_OS_WINDOWS)
                multiStream = plzma_out_multi_stream_create_with_directory_path_wide_name_ext_format_part_size(&path, L"file", L"7z", plzma_plzma_multi_stream_part_name_format_name_ext_00x, partSize);
#else
                multiStream = plzma_out_multi_stream_create_with_directory_path_utf8_name_ext_format_part_size(&path, "file", "7z", plzma_plzma_multi_stream_part_name_format_name_ext_00x, partSize);
#endif
                break;
            case Test4CaseMemNoPassword:
            case Test4CaseMemWithPassword:
                multiStream = plzma_out_multi_stream_create_memory_with_part_size(partSize);
                break;
            default: break;
        }
        PLZMA_TESTS_ASSERT(multiStream.object != nullptr)
        
        plzma_encoder encoder = plzma_encoder_create_with_multi_stream(&multiStream, plzma_file_type_7z, plzma_method_LZMA, plzma_context{nullptr, nullptr});
        
        switch (testCase) {
            case Test4CaseFileWithPassword:
            case Test4CaseMemWithPassword:
#if defined(LIBPLZMA_OS_WINDOWS)
                plzma_encoder_set_password_wide_string(&encoder, L"hello");
#else
                plzma_encoder_set_password_utf8_string(&encoder, "hello");
#endif
                break;
            default: break;
        }
        
        plzma_in_stream tmpInStream = plzma_in_stream_create_with_memory_copy(FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE);
        plzma_path tmpPath = plzma_path_create_with_utf8_string("shutuptakemoney.jpg");
        plzma_encoder_add_stream(&encoder, &tmpInStream, &tmpPath);
        plzma_in_stream_release(&tmpInStream);
        plzma_path_release(&tmpPath);
        
        tmpInStream = plzma_in_stream_create_with_memory_copy(FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE);
        tmpPath = plzma_path_create_with_utf8_string("SouthPark.jpg");
        plzma_encoder_add_stream(&encoder, &tmpInStream, &tmpPath);
        plzma_in_stream_release(&tmpInStream);
        plzma_path_release(&tmpPath);
        
        tmpInStream = plzma_in_stream_create_with_memory_copy(FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE);
        tmpPath = plzma_path_create_with_utf8_string("zombies.jpg");
        plzma_encoder_add_stream(&encoder, &tmpInStream, &tmpPath);
        plzma_in_stream_release(&tmpInStream);
        plzma_path_release(&tmpPath);
        
        tmpInStream = plzma_in_stream_create_with_memory_copy(FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE);
        tmpPath = plzma_path_create_with_utf8_string("Мюнхен.jpg");
        plzma_encoder_add_stream(&encoder, &tmpInStream, &tmpPath);
        plzma_in_stream_release(&tmpInStream);
        plzma_path_release(&tmpPath);
        
        tmpInStream = plzma_in_stream_create_with_memory_copy(FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE);
        tmpPath = plzma_path_create_with_utf8_string("München.jpg");
        plzma_encoder_add_stream(&encoder, &tmpInStream, &tmpPath);
        plzma_in_stream_release(&tmpInStream);
        plzma_path_release(&tmpPath);
        
        bool opened = plzma_encoder_open(&encoder);
        PLZMA_TESTS_ASSERT(opened == true)
        bool compressed = plzma_encoder_compress(&encoder);
        PLZMA_TESTS_ASSERT(compressed == true)
        plzma_encoder_release(&encoder);
        
        plzma_memory content = plzma_out_stream_copy_content(&multiStream);
        
        size_t totalPartsSize = 0;
        switch (testCase) {
            case Test4CaseFileNoPassword:
            case Test4CaseFileWithPassword: {
                plzma_path_iterator pathIterator = plzma_path_open_dir(&path, plzma_open_dir_mode_follow_symlinks);
                while (plzma_path_iterator_next(&pathIterator)) {
                    plzma_path fullPath = plzma_path_iterator_full_path(&pathIterator);
                    std::cout << plzma_path_utf8_string(&fullPath) << std::endl;
                    plzma_path_stat partStat = plzma_path_get_stat(&fullPath);
                    totalPartsSize += partStat.size;
                    plzma_path_release(&fullPath);
                }
                std::cout << std::endl;
                plzma_path_iterator_release(&pathIterator);
                PLZMA_TESTS_ASSERT(content.size == totalPartsSize)
            } break;
                
            default: break;
        }
        plzma_out_stream_array outMultiStreams = plzma_out_multi_stream_streams(&multiStream);
        PLZMA_TESTS_ASSERT(plzma_out_stream_array_count(&outMultiStreams) > 0)
        
        totalPartsSize = 0;
        for (plzma_size_t i = 0, n = plzma_out_stream_array_count(&outMultiStreams); i < n; i++) {
            plzma_out_stream tmpOutStream = plzma_out_stream_array_at(&outMultiStreams, i);
            plzma_memory partContent = plzma_out_stream_copy_content(&tmpOutStream);
            if (i + 1 == n) {
                PLZMA_TESTS_ASSERT(partContent.size <= partSize)
            } else {
                PLZMA_TESTS_ASSERT(partContent.size == partSize)
            }
            plzma_free(partContent.memory);
            plzma_out_stream_release(&tmpOutStream);
            totalPartsSize += partContent.size;
        }
        PLZMA_TESTS_ASSERT(content.memory != nullptr)
        PLZMA_TESTS_ASSERT(content.size > 0)
        PLZMA_TESTS_ASSERT(content.size == totalPartsSize)
        
        bool erased = plzma_out_stream_erase(&multiStream, plzma_erase_none);
        PLZMA_TESTS_ASSERT(erased == true)
        
        tmpInStream = plzma_in_stream_create_with_memory_copy(content.memory, content.size);
        plzma_decoder decoder = plzma_decoder_create(&tmpInStream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
        plzma_in_stream_release(&tmpInStream);
        
        plzma_free(content.memory);
        content.memory = nullptr;
        content.size = 0;
        
        switch (testCase) {
            case Test4CaseFileWithPassword:
            case Test4CaseMemWithPassword:
                plzma_decoder_set_password_utf8_string(&decoder, "hello");
                break;
            default: break;
        }
        opened = plzma_decoder_open(&decoder);
        PLZMA_TESTS_ASSERT(opened == true)
        PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)
        
        auto memoryMap = plzma_item_out_stream_array_create(5);
        auto fileMap = plzma_item_out_stream_array_create(5);
        for (plzma_size_t i = 0; i < 5; i++) {
            plzma_item tmpItem = plzma_decoder_item_at(&decoder, i);
            plzma_out_stream tmpOutStream = plzma_out_stream_create_memory_stream();
            plzma_item_out_stream_array_add(&memoryMap, &tmpItem, &tmpOutStream);
            plzma_out_stream_release(&tmpOutStream);
            
            tmpPath = plzma_path_create_with_tmp_dir();
            plzma_path_append_random_component(&tmpPath);
            tmpOutStream = plzma_out_stream_create_with_pathm(&tmpPath);
            plzma_path_release(&tmpPath);
            
            plzma_item_out_stream_array_add(&fileMap, &tmpItem, &tmpOutStream);
            plzma_out_stream_release(&tmpOutStream);
            plzma_item_release(&tmpItem);
        }
        
        PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&memoryMap) == 5)
        PLZMA_TESTS_ASSERT(plzma_item_out_stream_array_count(&fileMap) == 5)
        
        bool extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &memoryMap);
        PLZMA_TESTS_ASSERT(extracted == true)
        extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &fileMap);
        PLZMA_TESTS_ASSERT(extracted == true)
        
        for (plzma_size_t i = 0; i < 10; i++) {
            plzma_item_out_stream_array_pair pair = (i < 5) ? plzma_item_out_stream_array_pair_at(&memoryMap, i) : plzma_item_out_stream_array_pair_at(&fileMap, i - 5);
            plzma_memory content = plzma_out_stream_copy_content(&pair.stream);
            bool erased = plzma_out_stream_erase(&pair.stream, plzma_erase_none);
            PLZMA_TESTS_ASSERT(erased == true)
            tmpPath = plzma_item_path(&pair.item);
            if (strcmp(plzma_path_utf8_string(&tmpPath), "shutuptakemoney.jpg") == 0) {
                PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__shutuptakemoney_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.size == FILE__shutuptakemoney_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__shutuptakemoney_jpg_PTR, FILE__shutuptakemoney_jpg_SIZE) == 0);
            } else if (strcmp(plzma_path_utf8_string(&tmpPath), "SouthPark.jpg") == 0) {
                PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__southpark_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.size == FILE__southpark_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__southpark_jpg_PTR, FILE__southpark_jpg_SIZE) == 0);
            } else if (strcmp(plzma_path_utf8_string(&tmpPath), "zombies.jpg") == 0) {
                PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__zombies_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.size == FILE__zombies_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__zombies_jpg_PTR, FILE__zombies_jpg_SIZE) == 0);
            } else if (strcmp(plzma_path_utf8_string(&tmpPath), "Мюнхен.jpg") == 0 || strcmp(plzma_path_utf8_string(&tmpPath), "München.jpg") == 0) {
                PLZMA_TESTS_ASSERT(plzma_item_size(&pair.item) == FILE__munchen_jpg_SIZE)
                PLZMA_TESTS_ASSERT(content.size == FILE__munchen_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memcmp(content.memory, FILE__munchen_jpg_PTR, FILE__munchen_jpg_SIZE) == 0);
            } else {
                PLZMA_TESTS_ASSERT(false)
            }
            plzma_free(content.memory);
            content.memory = nullptr;
            plzma_path_release(&tmpPath);
            plzma_item_out_stream_array_pair_release(&pair);
        }
        
        plzma_decoder_release(&decoder);
        
        plzma_path_remove(&path, false);
        plzma_path_release(&path);
        
        for (plzma_size_t i = 0; i < 5; i++) {
            plzma_item_out_stream_array_pair pair = plzma_item_out_stream_array_pair_at(&memoryMap, i);
            plzma_out_stream_erase(&pair.stream, plzma_erase_none);
            plzma_item_out_stream_array_pair_release(&pair);
            
            pair = plzma_item_out_stream_array_pair_at(&fileMap, i);
            plzma_out_stream_erase(&pair.stream, plzma_erase_none);
            plzma_item_out_stream_array_pair_release(&pair);
        }
        
        plzma_out_stream_array_release(&outMultiStreams);
        plzma_out_multi_stream_release(&multiStream);
        plzma_item_out_stream_array_release(&memoryMap);
        plzma_item_out_stream_array_release(&fileMap);
    }
#endif // !LIBPLZMA_NO_C_BINDINGS && !LIBPLZMA_NO_CRYPTO
    return 0;
}

int main(int argc, char* argv[]) {
    std::cout << plzma_version() << std::endl;
    int ret = 0;
    
    if ( (ret = test_plzma_multivolume_test4()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_multivolume_test5()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_multivolume_test1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_multivolume_test2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_multivolume_test3()) ) {
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
#include "../test_files/file__18_7z_001.h"
#include "../test_files/file__18_7z_002.h"
#include "../test_files/file__18_7z_003.h"

// images
#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__zombies_jpg.h"
