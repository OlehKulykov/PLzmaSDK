Travis CI: macOS, Linux | AppVeyor: Windows
----------------------- | -----------------
[![Build Status](https://travis-ci.org/OlehKulykov/PLzmaSDK.svg?branch=master)](https://travis-ci.org/OlehKulykov/PLzmaSDK) | [![Build status](https://ci.appveyor.com/api/projects/status/1mb5w6nlht1ar2p8/branch/master?svg=true)](https://ci.appveyor.com/project/OlehKulykov/plzmasdk/branch/master)


**P**LzmaSDK is (**P**ortable, **P**atched, **P**ackage, cross-**P**-latform) Lzma SDK.
Based on original [LZMA SDK] version 19.00 (1900 - latest for now) and patched for unix platforms.
Available for all Apple's platforms(iOS, macOS, tvOS, watchOS), Android, Windows and any unix'es.


### Features / detailed description
-----------
- The SDK is available for the next programming languages:
  * [Swift](https://swift.org/) via [Swift Package Manager] or [CocoaPods].
  * JavaScript via [npm].
  * C/C++ via git+CMake.
- Supports next archives:
  * [7z]. Both, encrypted/password-protected and unencrypted. [Lzma] and [Lzma2] compression methods.
  * [xz]. [Lzma2] compression method.
  * [tar]/[tarball].
- Supports list, test, extract and compress operations. All these operations can be executed in a separate thread and aborted.
- Thread safe encoder, decoder and progress tracking.
- Supports memory and file IO streams. The extracting and compressing might be from <-> to memory or file.
- Support extracting and compressing archive files with size more than 4GB.
- Track smoothed progress.
- UTF8 support.
- Available for any platform with compiller which supports the [C++11] standard. Let's say, almost everywhere nowadays.
- No external dependencies also no [STL].
- The SDK is organized as C **and** C++ library at the same time. Supports static and dynamic linking.
  * The [libplzma.h] - the library header for a pure C environment. Contains generic functions, types and bindings to the whole functionality of the library. Currently uses with [Swift Package Manager] and [CocoaPods].
  * The [libplzma.hpp] - the library header for a C++ environment and must be used together with [libplzma.h] header.
  * The [swift](https://github.com/OlehKulykov/PLzmaSDK/tree/master/swift) directory contains Swift part of the SDK and available via the [Swift Package Manager] and [CocoaPods], see ```Installation``` section.

### Optional features
All optional features are enabled by default, but might be disabled during the build to reduce the binary size if you are not planning to use them.

- [tar]/[tarball] archive support. To disable use CMake boolean option `LIBPLZMA_OPT_NO_TAR:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_TAR=1`
- Thread safety. To disable use CMake boolean option `LIBPLZMA_OPT_THREAD_UNSAFE:BOOL=YES` or preprocessor definition `LIBPLZMA_THREAD_UNSAFE=1`
- Progress tracking. To disable use CMake boolean option `LIBPLZMA_OPT_NO_PROGRESS:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_PROGRESS=1`
- C bindings to the whole functionality of the library in [libplzma.h] header. To disable use CMake boolean option `LIBPLZMA_OPT_NO_C_BINDINGS:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_C_BINDINGS=1`


### Installation
-----------
<details>
<summary>Swift Package Manager</summary>

```swift
.package(url: "https://github.com/OlehKulykov/PLzmaSDK.git", .exact("0.0.5"))
```

</details>
<details>
<summary>CocoaPods Podfile</summary>

```ruby
use_frameworks!
platform :ios, '8.0'

target '<REPLACE_WITH_YOUR_TARGET>' do
    pod 'PLzmaSDK', :inhibit_warnings => true
end
```

</details>
<details>
<summary>Android NDK</summary>
  
```bash
cd <PATH_TO_ANDROID_NDK>
./ndk-build NDK_PROJECT_PATH=<PATH_TO_PLZMASDK>/PLzmaSDK/android
```

</details>
<details>
<summary>CMake</summary>

<details>
<summary>Unix</summary>
  
```bash
cd PLzmaSDK
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```
  
</details>
<details>
<summary>Windows</summary>
  
```bash
cd PLzmaSDK
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 4
```
  
</details>
</details>


### Examples
-----------
<details>
<summary>Open, list and select archive items for extracting or testing</summary> 
  
The process consists of three steps:
1. Create a source input stream for reading archive file content. The input stream might be created with:
   1. The path to an archive file.
   2. The archive's file content in memory.
   3. The custom read/seek callbacks(C/C++ only).
2. Create decoder with source input stream, type of archive and optional delegate.
   1. Optionaly provide the password to open the archive.
3. Select archive items for extracting or testing.
   1. Select all archive items.
   2. Get the number of items, iterate items by index, filter and select items.
  
<details>
<summary>Swift</summary>

```swift
do {
    // 1. Create a source input stream for reading archive file content.
    //  1.1. Create a source input stream with the path to an archive file.
    let archivePath = try Path("path/to/archive")
    let archivePathInStream = try InStream(path: archivePath)

    //  1.2. Create a source input stream with the file content in memory.
    let archiveData = Data(bytesNoCopy: <FILE DATA>, count: <FILE SIZE>, deallocator: .none)
    let archiveDataInStream = try InStream(dataNoCopy: archiveData) // also available Data(dataCopy: Data)

    // 2. Create decoder with source input stream, type of archive and optional delegate.
    let decoder = try Decoder(stream: archiveDataInStream /* archivePathInStream */, fileType: .sevenZ, delegate: self)
    
    //  2.1. Optionaly provide the password to open/list/test/extract the archive items.
    try decoder.setPassword("1234")
    
    let opened = try decoder.open()
    
    // 3. Select archive items for extracting or testing.
    //  3.1. Select all archive items.
    let allArchiveItems = try decoder.items()
    
    //  3.2. Get the number of items, iterate items by index, filter and select items.
    let numberOfArchiveItems = try decoder.count()
    let selectedItemsDuringIteration = try ItemArray(capacity: numberOfArchiveItems)
    for itemIndex in 0..<numberOfArchiveItems {
        let item = try decoder.item(at: itemIndex)
        try selectedItemsDuringIteration.add(item: item)
    }
    ...
} catch let exception as Exception {
    print("Exception: \(exception)")
}
```

</details>
<details>
<summary>C++</summary>

```cpp
try {
    // 1. Create a source input stream for reading archive file content.
    //  1.1. Create a source input stream with the path to an archive file.
    Path archivePath("path/to/archive"); // Path(L"C:\\\\path\\to\\archive");
    auto archivePathInStream = makeSharedInStream(archivePath /* std::move(archivePath) */);
    
    //  1.2. Create a source input stream with the file content in memory.
    auto archiveDataInStream = makeSharedInStream(<FILE DATA>, <FILE SIZE>);
    
    // 2. Create decoder with source input stream, type of archive and provide optional delegate.
    auto decoder = makeSharedDecoder(archiveDataInStream, plzma_file_type_7z);
    decoder->setProgressDelegate(this);
    
    //  2.1. Optionaly provide the password to open/list/test/extract the archive items.
    decoder->setPassword("1234"); // decoder->setPassword(L"1234");
        
    bool opened = decoder->open();
    
    // 3. Select archive items for extracting or testing.
    //  3.1. Select all archive items.
    auto allArchiveItems = decoder->items();

    //  3.2. Get the number of items, iterate items by index, filter and select items.
    size_t numberOfArchiveItems = decoder->count();
    auto selectedItemsDuringIteration = makeShared<ItemArray>(numberOfArchiveItems);
    for (size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
        auto item = decoder->itemAt(itemIndex);
        selectedItemsDuringIteration->push(item /* std::move(item) */);
    }
    ...
} catch (const Exception & exception) {
    std::cout << "Exception: " << exception.what() << std::endl;
}
```

</details>
<details>
<summary>C</summary>

```c
// 1. Create a source input stream for reading archive file content.
//  1.1. Create a source input stream with the path to an archive file.
plzma_path archivePath = plzma_path_create_with_utf8_string("path/to/archive"); // plzma_path_create_with_wide_string(L"C:\\\\path\\to\\archive");
plzma_in_stream archivePathInStream = plzma_in_stream_create_with_path(&archivePath); // plzma_in_stream_create_with_pathm(...);
plzma_path_release(&archivePath);
plzma_in_stream_release(&archivePathInStream); // when no longer needed

//  1.2. Create a source input stream with the file content in memory.
plzma_in_stream archiveDataInStream = plzma_in_stream_create_with_memory_copy(<FILE DATA>, <FILE SIZE>); // plzma_in_stream_create_with_memory(...);

// 2. Create decoder with source input stream, type of archive, context for optional delegate and provide optional delegate callback.
plzma_decoder decoder = plzma_decoder_create(&archiveDataInStream, plzma_file_type_7z, plzma_context{ nullptr, nullptr }); // C2059 = { .context = nullptr, .deinitializer = nullptr }
plzma_in_stream_release(&archiveDataInStream); // when no longer needed

plzma_decoder_set_progress_delegate_utf8_callback(&decoder, <UTF8 C CALLBACK>);  // plzma_decoder_set_progress_delegate_wide_callback(...);

//  2.1. Optionaly provide the password to open/list/test/extract the archive items.
plzma_decoder_set_password_utf8_string(&decoder, "1234"); // plzma_decoder_set_password_wide_string(&decoder, L"1234");

bool opened = plzma_decoder_open(&decoder);    

// 3. Select archive items for extracting or testing.
//  3.1. Select all archive items.
plzma_item_array allArchiveItems = plzma_decoder_items(&decoder);

//  3.2. Get the number of items, iterate items by index, filter and select items.
size_t numberOfArchiveItems = plzma_decoder_count(&decoder);
plzma_item_array selectedItemsDuringIteration = plzma_item_array_create(numberOfArchiveItems);
for (size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
    plzma_item item = plzma_decoder_item_at(&decoder, itemIndex);
    plzma_item_array_add(&selectedItemsDuringIteration, &item);
    plzma_item_release(&item);
}
...
plzma_item_array_release(&selectedItemsDuringIteration); // when no longer needed 
plzma_item_array_release(&allArchiveItems); // when no longer needed
plzma_decoder_release(&decoder); // when no longer needed
```

</details>
</details>

<details>
<summary>Extract all or selected archive items</summary>
  
Example description
  
<details>
<summary>Swift</summary>

```swift
// Hello swift: Extract all or selected archive items
```

</details>
<details>
<summary>C++</summary>

```cpp
// Hello cpp: Extract all or selected archive items
```

</details>
<details>
<summary>C</summary>

```cpp
// Hello c: Extract all or selected archive items
```

</details>
</details>


<details>
<summary>Compress individual files, directories or memory streams</summary>
  
Example description
  
<details>
<summary>Swift</summary>

```swift
// Hello swift: Compress individual files, directories or memory streams
```

</details>
<details>
<summary>C++</summary>

```cpp
// Hello cpp: Compress individual files, directories or memory streams
```

</details>
<details>
<summary>C</summary>

```cpp
// Hello c: Compress individual files, directories or memory streams
```

</details>
</details>


### License
-----------
By using this all you are accepting original [LZMA SDK] and MIT license (*see below*):

The MIT License (MIT)

Copyright (c) 2015 - 2020 Oleh Kulykov <olehkulykov@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


[LZMA SDK]:http://www.7-zip.org/sdk.html
[7z]:https://www.7-zip.org/7z.html
[LZMA]:https://www.7-zip.org/7z.html
[LZMA2]:https://www.7-zip.org/7z.html
[xz]:https://tukaani.org/xz
[tar]:https://en.wikipedia.org/wiki/Tar_(computing)
[tarball]:https://en.wikipedia.org/wiki/Tar_(computing)
[C++11]:https://en.cppreference.com/w/cpp/11
[STL]:https://en.wikipedia.org/wiki/Standard_Template_Library
[libplzma.h]:https://github.com/OlehKulykov/PLzmaSDK/blob/master/libplzma.h
[libplzma.hpp]:https://github.com/OlehKulykov/PLzmaSDK/blob/master/libplzma.hpp
[Swift Package Manager]:https://swift.org/package-manager
[CocoaPods]:https://cocoapods.org/pods/PLzmaSDK
[npm]:https://www.npmjs.com/package/plzmasdk
