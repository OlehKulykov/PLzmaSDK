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

#if defined(LIBPLZMA_OS_WINDOWS)
#define CSEP "\\"
#define WSEP L"\\"
#else
#define CSEP "/"
#define WSEP L"/"
#endif

int test_plzma_path_remove_last1(void) {
    Path path, pathCopy;
    
    path.set("/tmp/scratch.png");
    std::flush(std::cout) << path.utf8() << " " << __LINE__ << std::endl;
    std::wcout << path.wide() << L" " << __LINE__ << L"\n";
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    std::flush(std::cout) << path.utf8() << " " << __LINE__ << std::endl;
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP "tmp") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP L"tmp") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP "tmp") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP L"tmp") == 0)
    
    path.set("/tmp/lock/");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP "tmp") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP L"tmp") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP "tmp") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP L"tmp") == 0)
    
    path.set("/tmp/");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP) == 0)
    
    path.set("/tmp");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP) == 0)
    
    path.set("/");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP) == 0)

    path.set("scratch.png");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(path.count() == 0)
    PLZMA_TESTS_ASSERT(pathCopy.count() == 0)
    
    path.set("c:/");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "c:" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"c:" WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "c:" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"c:" WSEP) == 0)
    
    path.set("c:/a");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "c:" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"c:" WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "c:" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"c:" WSEP) == 0)
    
    path.set("c:\\\\a");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "c:" CSEP CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"c:" WSEP WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "c:" CSEP CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"c:" WSEP WSEP) == 0)
    
    path.set("c:\\\\a/b");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "c:" CSEP CSEP "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"c:" WSEP WSEP "a") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "c:" CSEP CSEP "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"c:" WSEP WSEP "a") == 0)
    
    path.set("");
    pathCopy = path.removingLastComponent();
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(path.count() == 0)
    PLZMA_TESTS_ASSERT(pathCopy.count() == 0)
    
    path.set(L".");
    PLZMA_TESTS_ASSERT(path.count() == 1)
    pathCopy = path.appending(L"*");
    path.append(L"*");
    PLZMA_TESTS_ASSERT(path.count() == 3)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"." WSEP "*") == 0)
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "." CSEP "*") == 0)
    PLZMA_TESTS_ASSERT(pathCopy.count() == 3)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"." WSEP "*") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "." CSEP "*") == 0)
    
    return 0;
}

int test_plzma_path_last_component1(void) {
    Path path;
    auto comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 0)
    
    path.set("/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 0)
    
    path.set("a");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"a") == 0)
    
    path.set("a/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"a") == 0)
    
    path.set("/a");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"a") == 0)
    
    path.set("/a/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"a") == 0)
    
    path.set("/tmp/scratch.png");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == strlen("scratch.png"))
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "scratch.png") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"scratch.png") == 0)
    
    path.set("/tmp/scratch");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == strlen("scratch"))
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "scratch") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"scratch") == 0)
    
    path.set("/tmp/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == strlen("tmp"))
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "tmp") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"tmp") == 0)
    
    path.set("scratch///");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == strlen("scratch"))
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "scratch") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"scratch") == 0)

    path.set("ä");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "ä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"ä") == 0)
    
    path.set("ä/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "ä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"ä") == 0)
    
    path.set("/ä");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "ä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"ä") == 0)
    
    path.set("/ä/");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "ä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"ä") == 0)
    
    path.set("\\ä\\");
    comp = path.lastComponent();
    PLZMA_TESTS_ASSERT(comp.count() == 1)
    PLZMA_TESTS_ASSERT(strcmp(comp.utf8(), "ä") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(comp.wide(), L"ä") == 0)
    
    return 0;
}

int test_plzma_path_test_append1(void) {
    Path path, pathCopy;
    
    pathCopy = path.appending("");
    path.append("");
    PLZMA_TESTS_ASSERT(path.count() == 0)
    PLZMA_TESTS_ASSERT(pathCopy.count() == 0)
    
    path.clear();
    pathCopy = path.appending("a");
    path.append("a");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a") == 0)
    
    path.set("a");
    pathCopy = path.appending("b");
    path.append("b");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b") == 0)
    
    path.set("\\//a");
    pathCopy = path.appending("b");
    path.append("b");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP L"a" WSEP L"b") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP L"a" WSEP L"b") == 0)

    path.set("\\a");
    pathCopy = path.appending("\\b");
    path.append("\\b");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), CSEP "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), WSEP L"a" WSEP L"b") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), CSEP "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), WSEP L"a" WSEP L"b") == 0)

    path.clear();
    pathCopy = path.appending("a").appending("\\b");
    path.append("a");
    path.append("\\b");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b") == 0)
    
    path.set("a/b\\");
    pathCopy = path.appending("/c\\d");
    path.append("/c\\d");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b" CSEP "c" CSEP "d") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b" CSEP "c" CSEP "d") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d") == 0)
    
    path.set("a/b/");
    pathCopy = path.appending("/c/d/");
    path.append("/c/d/");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b" CSEP "c" CSEP "d" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d" WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b" CSEP "c" CSEP "d" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d" WSEP) == 0)

    path.set("a\\b/");
    pathCopy = path.appending("\\c/d");
    path.append("\\c/d");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b" CSEP "c" CSEP "d") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b" WSEP L"c" WSEP "d") == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b" CSEP "c" CSEP "d") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b" WSEP L"c" WSEP "d") == 0)
    
    path.set("a\\b\\");
    pathCopy = path.appending("\\c\\/\\d\\");
    path.append("\\c\\/\\d\\");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a" CSEP "b" CSEP "c" CSEP "d" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d" WSEP) == 0)
    PLZMA_TESTS_ASSERT(strcmp(pathCopy.utf8(), "a" CSEP "b" CSEP "c" CSEP "d" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(pathCopy.wide(), L"a" WSEP L"b" WSEP L"c" WSEP L"d" WSEP) == 0)
    
    return 0;
}

int test_plzma_path_test_win1(void) {
    Path path("a://\\\\//b//c");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "a:" CSEP CSEP "b" CSEP "c") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"a:" WSEP WSEP L"b" WSEP L"c") == 0)
    
    path.set("C://\\\\//b//c\\");
    PLZMA_TESTS_ASSERT(strcmp(path.utf8(), "C:" CSEP CSEP "b" CSEP "c" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(path.wide(), L"C:" WSEP WSEP L"b" WSEP L"c" WSEP) == 0)
    return 0;
}

int test_plzma_path_test_win2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_path tmp = plzma_path_create_with_utf8_string("a://\\\\//b//c");
    PLZMA_TESTS_ASSERT(tmp.object != nullptr)
    PLZMA_TESTS_ASSERT(tmp.exception == nullptr)
    
    PLZMA_TESTS_ASSERT(strcmp(plzma_path_utf8_string(&tmp), "a:" CSEP CSEP "b" CSEP "c") == 0)
    PLZMA_TESTS_ASSERT(wcscmp(plzma_path_wide_string(&tmp), L"a:" WSEP WSEP L"b" WSEP L"c") == 0)
    
    plzma_path_set_utf8_string(&tmp, "C://\\\\//b//c\\");
    PLZMA_TESTS_ASSERT(strcmp(plzma_path_utf8_string(&tmp), "C:" CSEP CSEP "b" CSEP "c" CSEP) == 0)
    PLZMA_TESTS_ASSERT(wcscmp(plzma_path_wide_string(&tmp), L"C:" WSEP WSEP L"b" WSEP L"c" WSEP) == 0)
    plzma_path_release(&tmp);
#endif // !LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int test_plzma_path_test_tmp1(void) {
    Path tmp = Path::tmpPath();
    bool boolValue = false;
    
    PLZMA_TESTS_ASSERT(tmp.count() > 0)
    PLZMA_TESTS_ASSERT(tmp.exists(&boolValue) && boolValue == true)
    PLZMA_TESTS_ASSERT(tmp.readable())
    PLZMA_TESTS_ASSERT(tmp.writable())
    PLZMA_TESTS_ASSERT(tmp.readableAndWritable())
    PLZMA_TESTS_ASSERT(tmp.wide() != nullptr)
    PLZMA_TESTS_ASSERT(tmp.utf8() != nullptr)
    
    std::flush(std::cout) << "Temp dir: " << tmp.utf8() << std::endl;
    
    tmp.append("test.file");
    
    FILE * file1 = tmp.openFile("w+b");
    PLZMA_TESTS_ASSERT(file1 != nullptr);
    fclose(file1);
    
    plzma_path_timestamp timestamp = { 0 };
    timestamp.last_access = timestamp.last_modification = 1000;
    PLZMA_TESTS_ASSERT(tmp.applyFileTimestamp(timestamp) == true);
    plzma_path_stat stat = tmp.stat();
    PLZMA_TESTS_ASSERT(timestamp.last_access == stat.timestamp.last_access);
    PLZMA_TESTS_ASSERT(timestamp.last_modification == stat.timestamp.last_modification);
    
    tmp.remove();
    PLZMA_TESTS_ASSERT(tmp.exists() == false)
    
    std::flush(std::cout) << "Temp file opened: " << tmp.utf8() << std::endl;
    return 0;
}

int test_plzma_path_test_tmp2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_path tmp = plzma_path_create_with_tmp_dir();
    bool boolValue = false;
    
    PLZMA_TESTS_ASSERT(tmp.object != nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&tmp, &boolValue) && boolValue == true)
    PLZMA_TESTS_ASSERT(plzma_path_readable(&tmp))
    PLZMA_TESTS_ASSERT(plzma_path_writable(&tmp))
    PLZMA_TESTS_ASSERT(plzma_path_readable_and_writable(&tmp))
    PLZMA_TESTS_ASSERT(plzma_path_wide_string(&tmp) != nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_utf8_string(&tmp) != nullptr)
    
    std::flush(std::cout) << "Temp dir: " << plzma_path_utf8_string(&tmp) << std::endl;
    
    plzma_path_append_utf8_component(&tmp, "test.file");
    FILE * f = plzma_path_open_file(&tmp, "w+b");
    PLZMA_TESTS_ASSERT(f != nullptr)
    std::flush(std::cout) << "Temp file opened: " << plzma_path_utf8_string(&tmp) << std::endl;
    fclose(f);
    PLZMA_TESTS_ASSERT(plzma_path_remove(&tmp, false) == true);
    plzma_path_release(&tmp);
#endif // !LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int test_plzma_path_test_open_dir1(void) {
    SharedPtr<Path::Iterator> it;
    
    Path rootPath = Path::tmpPath();
    rootPath.appendRandomComponent();
    try {
        it = rootPath.openDir();
    } catch (const Exception & e) {
        std::flush(std::cout) << "Exception:" << e.what() << std::endl;
    } catch (...) {
        std::flush(std::cout) << "Exception unknown" << std::endl;
    }
    
    rootPath = Path::tmpPath();
    PLZMA_TESTS_ASSERT(rootPath.count() > 0)
    rootPath.removeLastComponent();
    PLZMA_TESTS_ASSERT(rootPath.count() > 0)
    
    std::flush(std::cout) << "Open path: " << rootPath.utf8() << std::endl;
    try {
        it = rootPath.openDir();
    } catch (const Exception & e) {
        std::flush(std::cout) << "Exception:" << e.what() << std::endl;
    } catch (...) {
        std::flush(std::cout) << "Exception unknown" << std::endl;
    }
    std::flush(std::cout) << std::endl;
    while (it->next()) {
        auto component = it->component();
        auto path = it->path();
        auto fullPath = it->fullPath();
        const auto times = fullPath.stat().timestamp;
#if defined(LIBPLZMA_OS_WINDOWS)
        if (component.wide()) {
            std::wcout << "Comp: " << component.wide() << std::endl;
        }
        if (path.wide()) {
            std::wcout << "Path: " << path.wide() << std::endl;
        }
        if (fullPath.wide()) {
            std::wcout << "Full: " << fullPath.wide() << std::endl;
        }
        std::wcout << L"Creation: " << _wctime(&times.creation) <<
        L"Last access: " << _wctime(&times.last_access) <<
        L"Last modification: " << _wctime(&times.last_modification);
#else
        if (component.utf8()) {
            std::flush(std::cout) << "Comp: " << component.utf8() << std::endl;
        }
        if (path.utf8()) {
            std::flush(std::cout) << "Path: " << path.utf8() << std::endl;
        }
        if (fullPath.utf8()) {
            std::flush(std::cout) << "Full: " << fullPath.utf8() << std::endl;
        }
        std::flush(std::cout) << "Creation: " << ctime(&times.creation) <<
        "Last access: " << ctime(&times.last_access) <<
        "Last modification: " << ctime(&times.last_modification);
#endif
        bool isDir = false;
        PLZMA_TESTS_ASSERT(fullPath.exists(&isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == it->isDir())
        fullPath.set(rootPath);
        fullPath.append(path);
        PLZMA_TESTS_ASSERT(fullPath.exists(&isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == it->isDir())
        
        std::flush(std::cout) << std::endl;
    }
    std::flush(std::cout) << "Done." << std::endl;
    return 0;
}

int test_plzma_path_test_open_dir2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_path rootPath = plzma_path_create_with_tmp_dir();
    PLZMA_TESTS_ASSERT(rootPath.object != nullptr)
    PLZMA_TESTS_ASSERT(plzma_path_count(&rootPath) > 0)
    plzma_path_remove_last_component(&rootPath);
    PLZMA_TESTS_ASSERT(plzma_path_count(&rootPath) > 0)
    
    std::flush(std::cout) << "Open path: " << plzma_path_utf8_string(&rootPath) << std::endl;
    plzma_path_iterator it = plzma_path_open_dir(&rootPath, 0);
    PLZMA_TESTS_ASSERT(it.object != nullptr)
    //TODO: error
    std::flush(std::cout) << std::endl;
    while (plzma_path_iterator_next(&it)) {
        plzma_path component = plzma_path_iterator_component(&it);
        PLZMA_TESTS_ASSERT(component.object != nullptr)
        plzma_path path = plzma_path_iterator_path(&it);
        PLZMA_TESTS_ASSERT(path.object != nullptr)
        plzma_path fullPath = plzma_path_iterator_full_path(&it);
        PLZMA_TESTS_ASSERT(fullPath.object != nullptr)
#if defined(LIBPLZMA_OS_WINDOWS)
        if (plzma_path_wide_string(&component)) {
            std::wcout << "Comp: " << plzma_path_wide_string(&component) << std::endl;
        }
        if (plzma_path_wide_string(&path)) {
            std::wcout << "Path: " << plzma_path_wide_string(&path) << std::endl;
        }
        if (plzma_path_wide_string(&fullPath)) {
            std::wcout << "Full: " << plzma_path_wide_string(&fullPath) << std::endl;
        }
#else
        if (plzma_path_utf8_string(&component)) {
            std::flush(std::cout) << "Comp: " << plzma_path_utf8_string(&component) << std::endl;
        }
        if (plzma_path_utf8_string(&path)) {
            std::flush(std::cout) << "Path: " << plzma_path_utf8_string(&path) << std::endl;
        }
        if (plzma_path_utf8_string(&fullPath)) {
            std::flush(std::cout) << "Full: " << plzma_path_utf8_string(&fullPath) << std::endl;
        }
#endif
        bool isDir = false;
        PLZMA_TESTS_ASSERT(plzma_path_exists(&fullPath, &isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == plzma_path_iterator_is_dir(&it))
#if defined(LIBPLZMA_OS_WINDOWS)
        plzma_path_set_wide_string(&fullPath, plzma_path_wide_string(&rootPath));
        plzma_path_append_wide_component(&fullPath, plzma_path_wide_string(&path));
#else
        plzma_path_set_utf8_string(&fullPath, plzma_path_utf8_string(&rootPath));
        plzma_path_append_utf8_component(&fullPath, plzma_path_utf8_string(&path));
#endif
        PLZMA_TESTS_ASSERT(plzma_path_exists(&fullPath, &isDir) == true)
        PLZMA_TESTS_ASSERT(isDir == plzma_path_iterator_is_dir(&it))
        
        plzma_path_release(&component);
        plzma_path_release(&path);
        plzma_path_release(&fullPath);
        
        std::flush(std::cout) << std::endl;
    }
    plzma_path_iterator_release(&it);
    plzma_path_release(&rootPath);
    
    std::flush(std::cout) << "Done." << std::endl;
#endif // !LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int test_plzma_path_test_append_random_component(void) {
    Path path;
    path.appendRandomComponent();
    PLZMA_TESTS_ASSERT(path.exists() == false)
    PLZMA_TESTS_ASSERT(path.count() > 0)
    auto len = String::lengthMaxCount(path.utf8(), 9999);
    PLZMA_TESTS_ASSERT(len.second == path.count())
    
    path = Path::tmpPath();
    path.appendRandomComponent();
    PLZMA_TESTS_ASSERT(path.exists() == false)
    PLZMA_TESTS_ASSERT(path.count() > 0)
    len = String::lengthMaxCount(path.utf8(), 9999);
    PLZMA_TESTS_ASSERT(len.second == path.count())
    
    FILE * file = path.openFile("w+b");
    PLZMA_TESTS_ASSERT(file != nullptr)
    fclose(file);
    PLZMA_TESTS_ASSERT(path.exists() == true)
    path.remove();
    PLZMA_TESTS_ASSERT(path.exists() == false)
    
    path = Path::tmpPath();
#if defined(LIBPLZMA_OS_WINDOWS)
    std::wcout << L"Line: " << __LINE__ << ",wpath: '" << path.wide() << "', size: " << path.count() << std::endl;
#endif
    len = String::lengthMaxCount(path.utf8(), 9999);
    std::flush(std::cout) << "Line: " << __LINE__ << ", path: '" << path.utf8() << "', len,size: " << len.first << "," << len.second << ", size(): " << path.count() << ", strlen: " << strlen(path.utf8()) << std::endl;
    path.append("/");
#if defined(LIBPLZMA_OS_WINDOWS)
    std::wcout << L"Line: " << __LINE__ << ",wpath: '" << path.wide() << "', size: " << path.count() << std::endl;
#endif
    len = String::lengthMaxCount(path.utf8(), 9999);
    std::flush(std::cout) << "Line: " << __LINE__ << ", path: '" << path.utf8() << "', len,size: " << len.first << "," << len.second << ", size(): " << path.count() << ", strlen: " << strlen(path.utf8()) << std::endl;
    path.appendRandomComponent();
#if defined(LIBPLZMA_OS_WINDOWS)
    std::wcout << L"Line: " << __LINE__ << ",wpath: '" << path.wide() << "', size: " << path.count() << std::endl;
#endif
    len = String::lengthMaxCount(path.utf8(), 9999);
    std::flush(std::cout) << "Line: " << __LINE__ << ", path: '" << path.utf8() << "', len,size: " << len.first << "," << len.second << ", size(): " << path.count() << ", strlen: " << strlen(path.utf8()) << std::endl;
    PLZMA_TESTS_ASSERT(path.exists() == false)
    PLZMA_TESTS_ASSERT(path.count() > 0)
#if defined(LIBPLZMA_OS_WINDOWS)
    std::wcout << L"Line: " << __LINE__ << ",wpath: '" << path.wide() << "', size: " << path.count() << std::endl;
#endif
    len = String::lengthMaxCount(path.utf8(), 9999);
    std::flush(std::cout) << "Line: " << __LINE__ << ", path: '" << path.utf8() << "', len,size: " << len.first << "," << len.second << ", size(): " << path.count() << ", strlen: " << strlen(path.utf8()) << std::endl;
    PLZMA_TESTS_ASSERT(len.second == path.count())
    
    file = path.openFile("w+b");
    PLZMA_TESTS_ASSERT(file != nullptr)
    fclose(file);
    PLZMA_TESTS_ASSERT(path.exists() == true)
    path.remove();
    PLZMA_TESTS_ASSERT(path.exists() == false)
    
    return 0;
}

int test_plzma_path_test_remove_non_empty_dir(void) {
    Path path = Path::tmpPath();
    PLZMA_TESTS_ASSERT(path.exists() == true)
    PLZMA_TESTS_ASSERT(path.count() > 0)
    // work dir
    path.appendRandomComponent();
    PLZMA_TESTS_ASSERT(path.exists() == false)
    PLZMA_TESTS_ASSERT(path.count() > 0)
    PLZMA_TESTS_ASSERT(path.createDir(false) == true)
    bool isDir = false;
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // subdir
    path.appendRandomComponent();
    PLZMA_TESTS_ASSERT(path.createDir(false) == true)
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // file in subdir
    path.appendRandomComponent();
    FILE * file = path.openFile("w+b");
    PLZMA_TESTS_ASSERT(file != nullptr)
    fclose(file);
    
    // subdir
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // work dir
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // empty subdir
    path.appendRandomComponent();
    PLZMA_TESTS_ASSERT(path.createDir(false) == true)
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // work dir
    path.removeLastComponent();
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    PLZMA_TESTS_ASSERT(path.remove() == true)
    PLZMA_TESTS_ASSERT(path.exists(&isDir) == false)
    PLZMA_TESTS_ASSERT(isDir == false)
    
    return 0;
}

int test_plzma_path_test_remove_non_empty_dir2(void) {
#if !defined(LIBPLZMA_NO_C_BINDINGS)
    plzma_path path = plzma_path_create_with_tmp_dir();
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, nullptr) == true)
    PLZMA_TESTS_ASSERT(plzma_path_count(&path) > 0)
    // work dir
    
    plzma_path_append_random_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, nullptr) == false)
    PLZMA_TESTS_ASSERT(plzma_path_count(&path) > 0)
    PLZMA_TESTS_ASSERT(plzma_path_create_dir(&path, false) == true)
    bool isDir = false;
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // subdir
    plzma_path_append_random_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_create_dir(&path, false) == true)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // file in subdir
    plzma_path_append_random_component(&path);
    FILE * file = plzma_path_open_file(&path, "w+b");
    PLZMA_TESTS_ASSERT(file != nullptr)
    fclose(file);
    
    // subdir
    plzma_path_remove_last_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // work dir
    plzma_path_remove_last_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // empty subdir
    plzma_path_append_random_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_create_dir(&path, false) == true)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    // work dir
    plzma_path_remove_last_component(&path);
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == true)
    PLZMA_TESTS_ASSERT(isDir == true)
    
    PLZMA_TESTS_ASSERT(plzma_path_remove(&path, false) == true)
    PLZMA_TESTS_ASSERT(plzma_path_exists(&path, &isDir) == false)
    PLZMA_TESTS_ASSERT(isDir == false)
    plzma_path_release(&path);
    
#endif // !LIBPLZMA_NO_C_BINDINGS
    return 0;
}

int main(int argc, char* argv[]) {
    std::flush(std::cout) << plzma_version() << std::endl;
    int ret = 0;
    
    if ( (ret = test_plzma_path_remove_last1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_last_component1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_tmp1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_tmp2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_append1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_win1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_win2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_open_dir1()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_open_dir2()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_append_random_component()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_remove_non_empty_dir()) ) {
        return ret;
    }
    
    if ( (ret = test_plzma_path_test_remove_non_empty_dir2()) ) {
        return ret;
    }
    
//    while (1) {
//        usleep(50);
//    }
    
    return ret;
}
