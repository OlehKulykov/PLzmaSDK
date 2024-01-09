//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2024 Oleh Kulykov <olehkulykov@gmail.com>
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

int test_plzma_string_test4(void) {
    auto len = String::lengthMaxCount(nullptr, static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 0 && len.second == 0)
    len = String::lengthMaxCount("", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 0 && len.second == 0)
    len = String::lengthMaxCount("a", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 1 && len.second == 1)
    len = String::lengthMaxCount("ä", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 2 && len.second == 1 && len.first == strlen("ä"))
    len = String::lengthMaxCount("ы", static_cast<size_t>(plzma_max_size()));
    PLZMA_TESTS_ASSERT(len.first == 2 && len.second == 1 && len.first == strlen("ы"))
    return 0;
}

int test_plzma_string_test3(void) {
    String s;
    
    s.set(L"a");
    PLZMA_TESTS_ASSERT(s.utf8() != nullptr)
    PLZMA_TESTS_ASSERT(s.count() == 1)
    
    s.set("ä");
    PLZMA_TESTS_ASSERT(s.count() == 1)
    
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"ä") == 0)
    s.set("H");
    PLZMA_TESTS_ASSERT(s.count() == 1)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"H") == 0)
    s.set("");
    PLZMA_TESTS_ASSERT(s.count() == 0)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"") == 0)
    
    return 0;
}

int test_plzma_string_test1(void) {
    String s("");
    PLZMA_TESTS_ASSERT(s.count() == 0)
    PLZMA_TESTS_ASSERT(s.count() == 0)
    s.set("a");
    PLZMA_TESTS_ASSERT(s.count() > 0)
    PLZMA_TESTS_ASSERT(s.utf8() != nullptr)
    s.set("a");
    PLZMA_TESTS_ASSERT(s.count() > 0)
    PLZMA_TESTS_ASSERT(s.wide() != nullptr)
    s.append("b");
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "ab") == 0)
    PLZMA_TESTS_ASSERT(s.count() == 2)
    s.append(L"c");
    PLZMA_TESTS_ASSERT(s.count() == 3)
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "abc") == 0)
    s.append(L"ä");
    PLZMA_TESTS_ASSERT(s.count() == 4)
    PLZMA_TESTS_ASSERT(strcmp(s.utf8(), "abcä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(s.wide(), L"abcä") == 0)
    s.clear(plzma_erase_zero);
    return 0;
}

int main(int argc, char* argv[]) {
    std::flush(std::cout) << plzma_version() << std::endl;
    int ret = 0;
    
    if ( (ret = test_plzma_string_test1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_string_test3()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_string_test4()) ) {
        return ret;
    }
    
//    while (1) {
//        usleep(50);
//    }
    
    return ret;
}
