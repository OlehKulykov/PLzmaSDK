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

#include "../test_files/file__1_7z.h"

using namespace plzma;

static void dummy_free_callback(void * LIBPLZMA_NULLABLE p) {
    // do nothing
}

#if defined(LIBPLZMA_OS_WINDOWS)
static void dummy_extract_wide_callback(void * LIBPLZMA_NULLABLE context,
                                        const wchar_t * LIBPLZMA_NONNULL wide_path,
                                        const double progress) {
    std::wcout << L"Extracting: " << wide_path << L", progress: " << progress << std::endl;
}
#else
static void dummy_extract_utf8_callback(void * LIBPLZMA_NULLABLE context,
                                        const char * LIBPLZMA_NONNULL utf8_path,
                                        const double progress) {
    std::flush(std::cout) << "Extracting: " << utf8_path << ", progress: " << progress << std::endl;
}
#endif

class TestProgressDelegate : public ProgressDelegate {
public:
    virtual void onProgress(void * LIBPLZMA_NULLABLE context, const String & path, const double progress) override final {
#if defined(LIBPLZMA_OS_WINDOWS)
        std::wcout << L"Extracting: " << path.wide() << L", progress: " << progress << std::endl;
#else
        std::flush(std::cout) << "Extracting: " << path.utf8() << ", progress: " << progress << std::endl;
#endif
    }
    virtual ~TestProgressDelegate() { }
};

static TestProgressDelegate * _progressDelegate = new TestProgressDelegate();

int test_plzma_extract_test1(void) {
#if !defined(LIBPLZMA_NO_CRYPTO)
    auto stream = makeSharedInStream(FILE__1_7z_PTR, FILE__1_7z_SIZE, &dummy_free_callback);
    auto decoder = makeSharedDecoder(stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    decoder->setPassword("1234");
    decoder->open();
    PLZMA_TESTS_ASSERT(decoder->count() == 5)
    decoder->setProgressDelegate(_progressDelegate);
    PLZMA_TESTS_ASSERT(decoder->test() == true)
#endif
    
    return 0;
}

int test_plzma_extract_test2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_in_stream stream = plzma_in_stream_create_with_memory(FILE__1_7z_PTR, FILE__1_7z_SIZE, &dummy_free_callback);
    plzma_decoder decoder = plzma_decoder_create(&stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    plzma_in_stream_release(&stream);
    plzma_decoder_set_password_utf8_string(&decoder, "1234");
#if defined(LIBPLZMA_OS_WINDOWS)
    plzma_decoder_set_progress_delegate_wide_callback(&decoder, &dummy_extract_wide_callback);
#else
    plzma_decoder_set_progress_delegate_utf8_callback(&decoder, &dummy_extract_utf8_callback);
#endif
    plzma_decoder_open(&decoder);
    PLZMA_TESTS_ASSERT(plzma_decoder_count(&decoder) == 5)
    PLZMA_TESTS_ASSERT(plzma_decoder_test(&decoder) == true)
    plzma_decoder_release(&decoder);
#endif // !LIBPLZMA_NO_C_BINDINGS
    
    return 0;
}

int test_plzma_extract_test3(void) {
//    //Path path("");
//    //InStream * stream = inStreamCreate(path);
//    //InStream * stream = inStreamCreate(FILE__lzma1900_7z_PTR, FILE__lzma1900_7z_SIZE, &dummy_free_callback);
//    InStream * stream = inStreamCreate(FILE__lzma_7z_PTR, FILE__lzma_7z_SIZE, &dummy_free_callback);
//    const Error * error = stream->error();
//    PLZMA_TESTS_ASSERT(error == nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    Decoder * decoder = decoderCreate(stream);
//    CLZMA_RELEASE_CLEAN(stream)
//    error = decoder->error();
//    PLZMA_TESTS_ASSERT(error == nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    PLZMA_TESTS_ASSERT(decoder->setPassword("1234") == true)
//    PLZMA_TESTS_ASSERT(decoder->open() == true)
//
//    ItemOutStreamMap * map = ItemOutStreamMap::create();
//    PLZMA_TESTS_ASSERT(map != nullptr)
//    for (plzma_size_t i = 0, n = decoder->itemsCount(); i < n; i++) {
//        const Item * item = decoder->createItemAt(i);
//        PLZMA_TESTS_ASSERT(item != nullptr)
//        OutStream * outStream = outStreamCreateMemoryStream();
//        PLZMA_TESTS_ASSERT(outStream != nullptr)
//        PLZMA_TESTS_ASSERT(map->add(item, outStream) == true)
//        CLZMA_RELEASE_CLEAN(item)
//        CLZMA_RELEASE_CLEAN(outStream)
//    }
//
//#if defined(LIBPLZMA_OS_WINDOWS)
//    decoder->setProgressCallback(dummy_extract_wide_callback);
//#else
//    decoder->setProgressCallback(dummy_extract_utf8_callback);
//#endif
//    time_t t = time(nullptr);
//    std::flush(std::cout) << t << " Start extracting ... \n";
//    bool extracted = false;
//    std::thread thread([&](){
//        decoder->retain();
//        extracted = decoder->extract(map);
//        decoder->release();
//    });
//    thread.join();
//    PLZMA_TESTS_ASSERT(extracted == true)
//    std::flush(std::cout) << time(nullptr) - t << " Extracting done.\n";
//    for (plzma_size_t i = 0, n = map->size(); i < n; i++) {
//        const auto pair = map->at(i);
//        PLZMA_TESTS_ASSERT(pair.first->size() == pair.second->size())
//        PLZMA_TESTS_ASSERT(pair.second->memory() != nullptr)
//    }
//
//    CLZMA_RELEASE_CLEAN(map)
//    error = decoder->error();
//    PLZMA_TESTS_ASSERT(error == nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    CLZMA_RELEASE_CLEAN(decoder)
    
    return 0;
}

int test_plzma_extract_test4(void) {
#if !defined(LIBPLZMA_NO_CRYPTO)
    auto stream = makeSharedInStream(FILE__1_7z_PTR, FILE__1_7z_SIZE, &dummy_free_callback);
    auto decoder = makeSharedDecoder(stream, plzma_file_type_7z, plzma_context{nullptr, nullptr});
    decoder->setPassword("1234");
    PLZMA_TESTS_ASSERT(decoder->open() == true);
    PLZMA_TESTS_ASSERT(decoder->count() == 5)
    decoder->setProgressDelegate(_progressDelegate);
    Path extractPath = Path::tmpPath();
    PLZMA_TESTS_ASSERT(extractPath.exists() == true);
    extractPath.appendRandomComponent();
    PLZMA_TESTS_ASSERT(extractPath.createDir(true) == true);
    std::flush(std::cout) << "Tmp extract path: " << extractPath.utf8() << std::endl;
    PLZMA_TESTS_ASSERT(decoder->extract(extractPath) == true);
    PLZMA_TESTS_ASSERT(extractPath.remove() == true);
#endif
    
    return 0;
}

int test_plzma_extract_broken_input_stream1(void) {
//    Path path;
//    InStream * stream = inStreamCreate(path);
//    PLZMA_TESTS_ASSERT(path.size() == 0)
//    const Error * error = stream->error();
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    CLZMA_RELEASE_CLEAN(stream)
//
//    stream = inStreamCreate("", 0);
//    error = stream->error();
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    CLZMA_RELEASE_CLEAN(stream)
//
//    uint8_t * streamMem = nullptr;
//    stream = inStreamCreate(streamMem, 0, dummy_free_callback);
//    error = stream->error();
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    CLZMA_RELEASE_CLEAN(error)
//    CLZMA_RELEASE_CLEAN(stream)
    return 0;
}

int test_plzma_extract_broken_input_stream2(void) {
//#if !defined(LIBPLZMA_NO_C_BINDINGS)
//    plzma_path * path = plzma_path_create_with_utf8_string("");
//    plzma_in_stream * stream = plzma_in_stream_create_with_path(path);
//    plzma_path_release(path);
//    PLZMA_TESTS_ASSERT(plzma_path_size(path) == 0)
//    const plzma_error * error = plzma_in_stream_error(stream);
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    plzma_error_release(error);
//    plzma_in_stream_release(stream);
//    
//    stream = plzma_in_stream_create_with_memory_copy("", 0);
//    error = plzma_in_stream_error(stream);
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    plzma_error_release(error);
//    plzma_in_stream_release(stream);
//    
//    uint8_t * streamMem = nullptr;
//    stream = plzma_in_stream_create_with_memory(streamMem, 0, dummy_free_callback);
//    error = plzma_in_stream_error(stream);
//    PLZMA_TESTS_ASSERT(error != nullptr)
//    plzma_error_release(error);
//    plzma_in_stream_release(stream);
//#endif // !LIBPLZMA_NO_C_BINDINGS    
    return 0;
}

int test_plzma_extract_test_settings(void) {
    PLZMA_TESTS_ASSERT(plzma_stream_read_size() > 0)
    PLZMA_TESTS_ASSERT(plzma_stream_write_size() > 0)
    PLZMA_TESTS_ASSERT(plzma_decoder_read_size() > 0)
    PLZMA_TESTS_ASSERT(plzma_decoder_write_size() > 0)
    return 0;
}

int main(int argc, char* argv[]) {
    int ret = 0;
    try {
        std::flush(std::cout) << plzma_version() << std::endl;
        
        if ( (ret = test_plzma_extract_test_settings()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_broken_input_stream1()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_broken_input_stream2()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_test4()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_test3()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_test2()) ) {
            return ret;
        }
        
        if ( (ret = test_plzma_extract_test1()) ) {
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

#include "../test_files/file__1_7z.h"
