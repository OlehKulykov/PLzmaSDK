//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2025 Oleh Kulykov <olehkulykov@gmail.com>
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


#include <thread>

#include "plzma_public_tests.hpp"

#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__zombies_jpg.h"

using namespace plzma;

static void dummy_free(void * LIBPLZMA_NULLABLE mem) {
    
}

class TestProgressDelegate : public ProgressDelegate {
public:
    virtual void onProgress(void * LIBPLZMA_NULLABLE context, const String & path, const double progress) override final {
#if defined(LIBPLZMA_OS_WINDOWS)
        std::wcout << L"Path: " << path.wide() << L", progress: " << progress << std::endl;
#else
        std::flush(std::cout) << "Path: " << path.utf8() << ", progress: " << progress << std::endl;
#endif
    }
    virtual ~TestProgressDelegate() { }
};

static TestProgressDelegate * _progressDelegate = new TestProgressDelegate();

int test_plzma_encode_xz_from_file_to_file_and_stream(void) {
    const auto tmpPath = Path::tmpPath();
    
    for (size_t i = 0; i < 4; i++) {
        auto inFilePath = tmpPath;
        inFilePath.appendRandomComponent();
        switch (i) {
            case 0:
                PLZMA_TESTS_ASSERT(FILE__southpark_jpg_write_to_file(inFilePath.utf8()) == true)
                break;
            case 1:
                PLZMA_TESTS_ASSERT(FILE__shutuptakemoney_jpg_write_to_file(inFilePath.utf8()) == true)
                break;
            case 2:
                PLZMA_TESTS_ASSERT(FILE__munchen_jpg_write_to_file(inFilePath.utf8()) == true)
                break;
            case 3:
                PLZMA_TESTS_ASSERT(FILE__zombies_jpg_write_to_file(inFilePath.utf8()) == true)
                break;
            default:
                break;
        }
        
        bool isDir = true;
        PLZMA_TESTS_ASSERT(inFilePath.exists(&isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == false)
        
        auto outPath = tmpPath;
        outPath.appendRandomComponent();
        
        auto outMemStream = makeSharedOutStream();
        auto outPathStream = makeSharedOutStream(outPath);
        PLZMA_TESTS_ASSERT(outPathStream.get() != nullptr)
        auto pathEncoder = makeSharedEncoder(outPathStream, plzma_file_type_xz, plzma_method_LZMA2);
        auto memEncoder = makeSharedEncoder(outMemStream, plzma_file_type_xz, plzma_method_LZMA2);
        PLZMA_TESTS_ASSERT(pathEncoder.get() != nullptr)
        PLZMA_TESTS_ASSERT(memEncoder.get() != nullptr)
        pathEncoder->setProgressDelegate(_progressDelegate);
        memEncoder->setProgressDelegate(_progressDelegate);
        pathEncoder->setCompressionLevel(9);
        memEncoder->setCompressionLevel(9);
        pathEncoder->add(inFilePath);
        memEncoder->add(inFilePath);
        PLZMA_TESTS_ASSERT(pathEncoder->open() == true)
        PLZMA_TESTS_ASSERT(memEncoder->open() == true)
        PLZMA_TESTS_ASSERT(pathEncoder->compress() == true)
        PLZMA_TESTS_ASSERT(memEncoder->compress() == true)
        pathEncoder->abort();
        memEncoder->abort();
        PLZMA_TESTS_ASSERT(outPathStream->opened() == false)
        PLZMA_TESTS_ASSERT(outMemStream->opened() == false)
        const auto pathContent = outPathStream->copyContent();
        auto memContent = outMemStream->copyContent();
        PLZMA_TESTS_ASSERT(static_cast<const void *>(pathContent.first) != nullptr)
        PLZMA_TESTS_ASSERT(pathContent.second > 0)
        PLZMA_TESTS_ASSERT(static_cast<const void *>(memContent.first) != nullptr)
        PLZMA_TESTS_ASSERT(memContent.second > 0)
        
        PLZMA_TESTS_ASSERT(pathContent.second == memContent.second)
        PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(pathContent.first), static_cast<const void *>(memContent.first), pathContent.second) == 0)
        
        isDir = true;
        PLZMA_TESTS_ASSERT(outPath.exists(&isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == false)
        
        // decode encoded results
        auto inPathStream = makeSharedInStream(outPath);
        auto inMemStream = makeSharedInStream(memContent.first, memContent.second, dummy_free);
        
        auto pathDecoder = makeSharedDecoder(inPathStream, plzma_file_type_xz);
        auto memDecoder = makeSharedDecoder(inMemStream, plzma_file_type_xz);
        pathDecoder->setProgressDelegate(_progressDelegate);
        memDecoder->setProgressDelegate(_progressDelegate);
        PLZMA_TESTS_ASSERT(pathDecoder->open() == true)
        PLZMA_TESTS_ASSERT(memDecoder->open() == true)
        
        // xz - 1 item
        PLZMA_TESTS_ASSERT(pathDecoder->count() == 1)
        PLZMA_TESTS_ASSERT(memDecoder->count() == 1)
        
        auto pathItem = pathDecoder->itemAt(0);
        auto memItem = memDecoder->itemAt(0);
        
        PLZMA_TESTS_ASSERT(pathItem->index() == 0)
        PLZMA_TESTS_ASSERT(memItem->index() == 0)
        
        switch (i) {
            case 0:
                PLZMA_TESTS_ASSERT(pathItem->size() == FILE__southpark_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memItem->size() == FILE__southpark_jpg_SIZE)
                break;
            case 1:
                PLZMA_TESTS_ASSERT(pathItem->size() == FILE__shutuptakemoney_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memItem->size() == FILE__shutuptakemoney_jpg_SIZE)
                break;
            case 2:
                PLZMA_TESTS_ASSERT(pathItem->size() == FILE__munchen_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memItem->size() == FILE__munchen_jpg_SIZE)
                break;
            case 3:
                PLZMA_TESTS_ASSERT(pathItem->size() == FILE__zombies_jpg_SIZE)
                PLZMA_TESTS_ASSERT(memItem->size() == FILE__zombies_jpg_SIZE)
                break;
            default:
                break;
        }
        
        PLZMA_TESTS_ASSERT(pathItem->packSize() != 0)
        PLZMA_TESTS_ASSERT(memItem->packSize() != 0)
        
        PLZMA_TESTS_ASSERT(pathItem->packSize() == memItem->packSize())
        
        auto outPathItemsStreams = makeShared<ItemOutStreamArray>();
        auto outPathItem = tmpPath;
        outPathItem.appendRandomComponent();
        
        pathDecoder->extract(outPathItem);
        
        auto outPathItemStream = makeSharedOutStream(outPathItem);
        outPathItemsStreams->push(ItemOutStreamArray::ElementType(pathItem, outPathItemStream));
        
        pathDecoder->extract(outPathItemsStreams);
        pathDecoder->abort();
        
        auto outMemItemsStreams = makeShared<ItemOutStreamArray>();
        auto outMemItemStream = makeSharedOutStream();
        outMemItemsStreams->push(ItemOutStreamArray::ElementType(memItem, outMemItemStream));
        
        memDecoder->extract(outMemItemsStreams);
        memDecoder->abort();
        
        PLZMA_TESTS_ASSERT(outPathItemStream->opened() == false)
        PLZMA_TESTS_ASSERT(outMemItemStream->opened() == false)
        
        auto outPathItemStreamContent = outPathItemStream->copyContent();
        auto outMemItemStreamContent = outMemItemStream->copyContent();
        PLZMA_TESTS_ASSERT(outPathItemStreamContent.second != 0)
        PLZMA_TESTS_ASSERT(outMemItemStreamContent.second != 0)
        PLZMA_TESTS_ASSERT(outPathItemStreamContent.second == outMemItemStreamContent.second)
        
        PLZMA_TESTS_ASSERT(memcmp(static_cast<const void *>(outPathItemStreamContent.first),
                                  static_cast<const void *>(outPathItemStreamContent.first),
                                  outPathItemStreamContent.second) == 0)
        
        PLZMA_TESTS_ASSERT(outPathItem.remove() == true)
        PLZMA_TESTS_ASSERT(inFilePath.remove() == true)
        
        PLZMA_TESTS_ASSERT(outPathStream->erase(plzma_erase_zero) == true);
        PLZMA_TESTS_ASSERT(outPath.exists() == false)
    }
    return 0;
}

int test_plzma_encode_test2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_out_stream stream = plzma_out_stream_create_memory_stream();
    PLZMA_TESTS_ASSERT(stream.exception == nullptr)
    PLZMA_TESTS_ASSERT(stream.object != nullptr)
    plzma_encoder encoder = plzma_encoder_create(&stream, plzma_file_type_7z, plzma_method_LZMA, plzma_context{nullptr, nullptr});
    PLZMA_TESTS_ASSERT(encoder.exception == nullptr)
    PLZMA_TESTS_ASSERT(encoder.object != nullptr)
    plzma_out_stream_release(&stream);
    plzma_encoder_release(&encoder);
#endif // !LIBPLZMA_NO_C_BINDINGS
    
    return 0;
}

int test_plzma_encode_example(void) {
    
    return 0;
}

int test_plzma_encode_test3(void) {
    const auto tmpPath = Path::tmpPath();
    const auto emptyDir = tmpPath.appendingRandomComponent();
    bool res = emptyDir.createDir(false);
    PLZMA_TESTS_ASSERT(res == true)
    bool isDir = false;
    res = emptyDir.exists(&isDir);
    PLZMA_TESTS_ASSERT(res == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    const auto emptyFile = emptyDir.appendingRandomComponent();
    FILE * filePtr = emptyFile.openFile("w+b");
    PLZMA_TESTS_ASSERT(filePtr != nullptr)
    fclose(filePtr);
    const auto stat = emptyFile.stat();
    PLZMA_TESTS_ASSERT(stat.size == 0)
    PLZMA_TESTS_ASSERT(stat.timestamp.creation != 0)
    PLZMA_TESTS_ASSERT(stat.timestamp.last_access != 0)
    PLZMA_TESTS_ASSERT(stat.timestamp.last_modification != 0)
    
    res = emptyDir.remove();
    PLZMA_TESTS_ASSERT(res == true)
    
    return 0;
}

int main(int argc, char* argv[]) {
    std::flush(std::cout) << plzma_version();
    int ret = 0;
    
    try {    
        if ( (ret = test_plzma_encode_xz_from_file_to_file_and_stream()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_encode_test2()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_encode_test3()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_encode_example()) ) {
            return ret;
        }
    } catch (const Exception & e) {
        std::flush(std::cout) << "PLZMA Exception [" << e.code() << "]:" << std::endl;
        if (e.what()) {
            std::flush(std::cout) << "what: " << e.what() << std::endl;
        }
        if (e.reason()) {
            std::flush(std::cout) << "reason: " << e.reason() << std::endl;
        }
        if (e.file()) {
            std::flush(std::cout) << "file: " << e.file() << std::endl;
        }
        std::flush(std::cout) << "line: " << e.line() << std::endl;
        throw;
    } catch (const std::exception & e) {
        std::flush(std::cout) << "std exception:" << std::endl;
        if (e.what()) {
            std::flush(std::cout) << "what: " << e.what() << std::endl;
        }
        throw;
    } catch (...) {
        std::flush(std::cout) << "unknown exception:" << std::endl;
        throw;
    }
    
//    while (1) {
//        usleep(50);
//    }
    
    return ret;
}

#include "../test_files/file__shutuptakemoney_jpg.h"
#include "../test_files/file__southpark_jpg.h"
#include "../test_files/file__munchen_jpg.h"
#include "../test_files/file__zombies_jpg.h"
