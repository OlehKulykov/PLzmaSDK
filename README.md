![platform](https://img.shields.io/badge/platform-iOS%20%7C%20macOS%20%7C%20tvOS%20%7C%20watchOS%20%7C%20Android%20%7C%20Windows%20%7C%20Linux%20%7C%20Unix-lightgrey.svg)
![language](https://img.shields.io/badge/language-Swift%20%7C%20Objective%E2%80%93C%20%7C%20C%20%7C%20C++%20%7C%20JavaScript-brightgreen.svg)
[![Cocoapods](https://img.shields.io/cocoapods/l/PLzmaSDK)](https://cocoapods.org/pods/PLzmaSDK)
[![Cocoapods](https://img.shields.io/cocoapods/v/PLzmaSDK)](https://cocoapods.org/pods/PLzmaSDK)
[![SwiftPM compatible](https://img.shields.io/badge/SwiftPM-compatible-brightgreen.svg)](https://swift.org/package-manager)
[![GitHub release](https://img.shields.io/github/release/OlehKulykov/PLzmaSDK.svg)](https://github.com/OlehKulykov/PLzmaSDK/releases)
[![node-current](https://img.shields.io/node/v/plzmasdk)](https://www.npmjs.com/package/plzmasdk)
[![Build Status](https://travis-ci.org/OlehKulykov/PLzmaSDK.svg?branch=master)](https://travis-ci.org/OlehKulykov/PLzmaSDK)
[![Build status](https://ci.appveyor.com/api/projects/status/1mb5w6nlht1ar2p8/branch/master?svg=true)](https://ci.appveyor.com/project/OlehKulykov/plzmasdk/branch/master)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/OlehKulykov/PLzmaSDK.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/OlehKulykov/PLzmaSDK/context:cpp)
[![Language grade: JavaScript](https://img.shields.io/lgtm/grade/javascript/g/OlehKulykov/PLzmaSDK.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/OlehKulykov/PLzmaSDK/context:javascript)

[![Tier badge](https://opencollective.com/plzmasdk/tiers/backer/badge.svg?label=Contribute%20to%20a%20Project&color=brightgreen)](https://opencollective.com/plzmasdk)


**P**LzmaSDK is (**P**ortable, **P**atched, **P**ackage, cross-**P**-latform) Lzma SDK.
Based on original [LZMA SDK] version 22.01 (2201 - latest for now) and patched for unix platforms.
Available for all Apple's platforms(iOS, macOS, tvOS, watchOS), Android, Windows, Linux and any unix'es.


### Features / detailed description
-----------
- The SDK is available for the next programming languages:
  * [Swift] via [Swift Package Manager] or [CocoaPods].
  * [Objective-C] via [CocoaPods].
  * [JavaScript] via [npm].
  * Pure C++ via [git] + [CMake] or copy 2 main lib headers([libplzma.h] and [libplzma.hpp] files) and [src] folder to your project.
  * Pure C, also via [git] + [CMake] or copy 2 main lib headers([libplzma.h] and [libplzma.hpp] files) and [src] folder to your project. But this internal C bindings code might be disabled via [CMake]'s boolean option `LIBPLZMA_OPT_NO_C_BINDINGS:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_C_BINDINGS=1`, see below.
- Supports next archives:
  * [7z]. Both, encrypted/password-protected and unencrypted archive items-list and it's content. [Lzma] and [Lzma2] compression methods.
  * [xz]. [Lzma2] compression method.
  * [tar]/[tarball]. *.tar, *.tar.xz and *.tar.7z archives.
- Supports list, test, extract and compress operations. All these operations can be executed in a separate thread and aborted during the process.
- Supports [7z] multivolume archives.
- Thread safe encoder, decoder and progress tracking. Depending of usage, you can disable all thread synchronizations via [CMake]'s boolean option `LIBPLZMA_OPT_THREAD_UNSAFE:BOOL=YES` or preprocessor definition `LIBPLZMA_THREAD_UNSAFE=1`.
- Supports memory and file IO streams. The extracting and compressing might be from ⇔ to memory or file.
- Support extracting and compressing archive files with size more than 4GB(x64 support).
- Track smoothed progress.
- Full UTF8 support.
- Available for any platform with compiller which supports the [C++11] standard. Let's say, almost everywhere nowadays.
- No external dependencies. Also no [STL] (of couse not in a public interface and internally).
- The SDK is organized as C **and** C++ library at the same time. And supports static and dynamic linking.
  * The [libplzma.h] - the library header for a pure C environment. Contains generic functions, types and optional bindings to the whole functionality of the library. Currently uses with [Swift Package Manager] and [CocoaPods].
  * The [libplzma.hpp] - the library header for a pure C++ environment and must be used together with [libplzma.h] header. Currently uses with [npm] native module and [Objective-C].
  * The [swift](https://github.com/OlehKulykov/PLzmaSDK/tree/master/swift) directory contains [Swift] part of the SDK and available via [Swift Package Manager] and [CocoaPods], see ```Installation``` section.
  * The [objc](https://github.com/OlehKulykov/PLzmaSDK/tree/master/objc) directory contains [Objective-C] part of the SDK and available via [CocoaPods], see ```Installation``` section.
  * The [node](https://github.com/OlehKulykov/PLzmaSDK/tree/master/node) directory contains Node.js native, inline module implementation. 

### Optional features
All optional features are enabled by default, but they might be disabled during the build process to reduce the binary size, and of course, if you are not planning to use them.

- [tar]/[tarball] archive support. To disable, use the [CMake]'s boolean option `LIBPLZMA_OPT_NO_TAR:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_TAR=1`
- Thread safety. To disable, use the [CMake]'s boolean option `LIBPLZMA_OPT_THREAD_UNSAFE:BOOL=YES` or preprocessor definition `LIBPLZMA_THREAD_UNSAFE=1`
- Progress tracking. To disable, use the [CMake]'s boolean option `LIBPLZMA_OPT_NO_PROGRESS:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_PROGRESS=1`
- C bindings to the whole functionality of the library in [libplzma.h] header. To disable, use the [CMake]'s boolean option `LIBPLZMA_OPT_NO_C_BINDINGS:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_C_BINDINGS=1`
- Crypto functionality. Not recommended! But possible. Do this only if you know what are you doing! To disable, use the [CMake]'s boolean option `LIBPLZMA_OPT_NO_CRYPTO:BOOL=YES` or preprocessor definition `LIBPLZMA_NO_CRYPTO=1`

### Installation
-----------
#### Swift Package Manager
```swift
.package(url: "https://github.com/OlehKulykov/PLzmaSDK.git", .exact("1.2.5"))
```

#### CocoaPods Podfile (Swift)
```ruby
use_frameworks!
platform :ios, '11.0'

target '<REPLACE_WITH_YOUR_TARGET>' do
    pod 'PLzmaSDK', '1.2.5'
end
```

#### CocoaPods Podfile (Objective-C)
```ruby
use_frameworks!
platform :ios, '9.0'

target '<REPLACE_WITH_YOUR_TARGET>' do
    pod 'PLzmaSDK-ObjC', '1.2.5'
end
```

#### npm via 'package.json'
```json
{
  "engines": {
    "node": ">=13.0.0",
    "npm": ">=6.0.0"
  },
  "dependencies": {
    "plzmasdk": "1.2.5"
  }
}
```

#### Android NDK
```bash
cd <PATH_TO_ANDROID_NDK>
./ndk-build NDK_PROJECT_PATH=<PATH_TO_PLZMASDK>/PLzmaSDK/android
```

#### CMake Unix
```bash
cd PLzmaSDK
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

#### CMake Windows
```bash
cd PLzmaSDK
md build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 4
```

### Examples
-----------
#### Extract or test
##### Open, list and select archive items for extracting or testing.  
The process consists of four steps:
1. Create a source input stream for reading archive file content. The input stream might be created with:
   1. The path to the archive file.
   2. The archive file content in memory, i.e. using const memory or it's copy for internal usage.
   3. The custom read/seek callbacks(C/C++ only).
2. Create decoder with source input stream, type of the archive and optional progress delegate.
   1. Optionaly provide the password to open and list encrypted archive and for a future extracting or testing.
3. Select archive items for extracting or testing. Migth be skiped if you want to process all items(the whole archive), see below.
   1. Select all archive items as is.
   2. Retrieve the number of items, iterate them by index, filter and/or select.
4. Extract or test selected archive items. The extract process might be:
   1. Extract all items to a directory. In this case, you can skip the step #3.
   2. Extract selected items to a directory.
   3. Extract each item to a custom out-stream. The out-stream might be a file or memory. I.e. extract 'item #1' to a file stream, extract 'item #2' to a memory stream(then take extacted memory) and so on.

##### Swift
```swift
do {
    // 1. Create a source input stream for reading archive file content.
    //  1.1. Create a source input stream with the path to an archive file.
    let archivePath = try Path("path/to/archive.7z")
    let archivePathInStream = try InStream(path: archivePath)

    //  1.2. Create a source input stream with the file content.
    let archiveData = Data(...)
    let archiveDataInStream = try InStream(dataNoCopy: archiveData) // also available Data(dataCopy: Data)

    // 2. Create decoder with source input stream, type of archive and optional delegate.
    let decoder = try Decoder(stream: archiveDataInStream /* archivePathInStream */, fileType: .sevenZ, delegate: self)
    
    //  2.1. Optionaly provide the password to open/list/test/extract encrypted archive items.
    try decoder.setPassword("1234")
    
    let opened = try decoder.open()
    
    // 3. Select archive items for extracting or testing.
    //  3.1. Select all archive items.
    let allArchiveItems = try decoder.items()
    
    //  3.2. Get the number of items, iterate items by index, filter and select items.
    let numberOfArchiveItems = try decoder.count()
    let selectedItemsDuringIteration = try ItemArray(capacity: numberOfArchiveItems)
    let selectedItemsToStreams = try ItemOutStreamArray()
    for itemIndex in 0..<numberOfArchiveItems {
        let item = try decoder.item(at: itemIndex)
        try selectedItemsDuringIteration.add(item: item)
        try selectedItemsToStreams.add(item: item, stream: OutStream()) // to memory stream
    }
    
    // 4. Extract or test selected archive items. The extract process might be:
    //  4.1. Extract all items to a directory. In this case, you can skip the step #3.
    let extracted = try decoder.extract(to: Path("path/outdir"))
    
    //  4.2. Extract selected items to a directory.
    let extracted = try decoder.extract(items: selectedItemsDuringIteration, to: Path("path/outdir"))
    
    //  4.3. Extract each item to a custom out-stream. 
    //       The out-stream might be a file or memory. I.e. extract 'item #1' to a file stream, extract 'item #2' to a memory stream(then take extacted memory) and so on.
    let extracted = try decoder.extract(itemsToStreams: selectedItemsToStreams)
} catch let exception as Exception {
    print("Exception: \(exception)")
}
```

##### JavaScript
```javascript
const plzma = require('plzmasdk');

try {
    // 1. Create a source input stream for reading archive file content.
    //  1.1. Create a source input stream with the path to an archive file.
    const archivePath = plzma.Path(__dirname).append('path/to/archive.7z');
    const archivePathInStream = new plzma.InStream(archivePath /* 'path/to/archive.7z' */);

    //  1.2. Create a source input stream with the file content.
    const archiveData = new ArrayBuffer(...);
    const archiveDataInStream = new plzma.InStream(archiveData);

    // 2. Create decoder with source input stream, type of archive and optional delegate.
    const decoder = new plzma.Decoder(archivePathInStream, plzma.FileType.sevenZ);
    decoder.setProgressDelegate((path, progress) => console.log(`Delegating progress, path: ${path}, progress: ${progress}`) );

    //  2.1. Optionaly provide the password to open/list/test/extract encrypted archive items.
    decoder.setPassword('1234');

    const opened = await decoder.openAsync(); // also available sync. version 'decoder.open()'

    // 3. Select archive items for extracting or testing.
    //  3.1. Select all archive items.
    const allArchiveItems = decoder.items;

    //  3.2. Get the number of items, iterate items by index, filter and select items.
    const selectedItemsDuringIteration = [];
    const selectedItemsToStreams = new Map();
    for (let itemIndex = 0, numberOfArchiveItems = decoder.count; itemIndex <  numberOfArchiveItems; itemIndex++) {
        const item = decoder.itemAt(itemIndex);
        selectedItemsDuringIteration.push(item);
        selectedItemsToStreams.set(item, plzma.OutStream()); // to memory stream
    }
    
    // 4. Extract or test selected archive items. The extract process might be:
    //  4.1. Extract all items to a directory. In this case, you can skip the step #3.
    const extracted = await decoder.extractAsync('path/outdir'); // also available sync. version 'decoder.extract()'
    
    //  4.2. Extract selected items to a directory.
    const extracted = await decoder.extractAsync(selectedItemsDuringIteration, 'path/outdir'); // also available sync. version 'decoder.extract()'
    
    //  4.3. Extract each item to a custom out-stream. 
    //       The out-stream might be a file or memory. I.e. extract 'item #1' to a file stream, extract 'item #2' to a memory stream(then take extacted memory) and so on.
    const extracted = await decoder.extractAsync(selectedItemsToStreams); // also available sync. version 'decoder.extract()'
} catch (error) {
    console.log(`Exception: ${error}`);
}
```

##### C++
```cpp
try {
    // 1. Create a source input stream for reading archive file content.
    //  1.1. Create a source input stream with the path to an archive file.
    Path archivePath("path/to/archive.7z"); // Path(L"C:\\\\path\\to\\archive.7z");
    auto archivePathInStream = makeSharedInStream(archivePath /* std::move(archivePath) */);
    
    //  1.2. Create a source input stream with the file content.
    auto archiveDataInStream = makeSharedInStream(<FILE DATA>, <FILE SIZE>);
    
    // 2. Create decoder with source input stream, type of archive and provide optional delegate.
    auto decoder = makeSharedDecoder(archiveDataInStream, plzma_file_type_7z);
    decoder->setProgressDelegate(this);
    
    //  2.1. Optionaly provide the password to open/list/test/extract encrypted archive items.
    decoder->setPassword("1234"); // decoder->setPassword(L"1234");
        
    bool opened = decoder->open();
    
    // 3. Select archive items for extracting or testing.
    //  3.1. Select all archive items.
    auto allArchiveItems = decoder->items();

    //  3.2. Get the number of items, iterate items by index, filter and select items.
    size_t numberOfArchiveItems = decoder->count();
    auto selectedItemsDuringIteration = makeShared<ItemArray>(numberOfArchiveItems);
    auto selectedItemsToStreams = makeShared<ItemOutStreamArray>();
    for (size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
        auto item = decoder->itemAt(itemIndex);
        selectedItemsDuringIteration->push(item /* std::move(item) */);
        selectedItemsToStreams->push(Pair<SharedPtr<Item>, SharedPtr<OutStream> >(item, makeSharedOutStream())); // to memory stream
    }
    
    // 4. Extract or test selected archive items. The extract process might be:
    //  4.1. Extract all items to a directory. In this case, you can skip the step #3.
    bool extracted = decoder->extract(Path("path/outdir"));
    
    //  4.2. Extract selected items to a directory.
    bool extracted = decoder->extract(selectedItemsDuringIteration, Path("path/outdir"));
    
    //  4.3. Extract each item to a custom out-stream. 
    //       The out-stream might be a file or memory. I.e. extract 'item #1' to a file stream, extract 'item #2' to a memory stream(then take extacted memory) and so on.
    bool extracted = decoder->extract(selectedItemsToStreams);
} catch (const Exception & exception) {
    std::cout << "Exception: " << exception.what() << std::endl;
}
```

##### C
```c
// 1. Create a source input stream for reading archive file content.
//  1.1. Create a source input stream with the path to an archive file.
plzma_path archivePath = plzma_path_create_with_utf8_string("path/to/archive.7z"); // plzma_path_create_with_wide_string(L"C:\\\\path\\to\\archive.7z");
plzma_in_stream archivePathInStream = plzma_in_stream_create_with_path(&archivePath); // plzma_in_stream_create_with_pathm(...);
plzma_path_release(&archivePath);
plzma_in_stream_release(&archivePathInStream); // when no longer needed

//  1.2. Create a source input stream with the file content in memory.
plzma_in_stream archiveDataInStream = plzma_in_stream_create_with_memory_copy(<FILE DATA>, <FILE SIZE>); // plzma_in_stream_create_with_memory(...);

// 2. Create decoder with source input stream, type of archive, context for optional delegate and provide optional delegate callback.
plzma_decoder decoder = plzma_decoder_create(&archiveDataInStream, plzma_file_type_7z, plzma_context{ nullptr, nullptr }); // C2059 = { .context = nullptr, .deinitializer = nullptr }
plzma_in_stream_release(&archiveDataInStream); // when no longer needed

plzma_decoder_set_progress_delegate_utf8_callback(&decoder, <UTF8 C CALLBACK>);  // plzma_decoder_set_progress_delegate_wide_callback(...);

//  2.1. Optionaly provide the password to open/list/test/extract encrypted archive items.
plzma_decoder_set_password_utf8_string(&decoder, "1234"); // plzma_decoder_set_password_wide_string(&decoder, L"1234");

bool opened = plzma_decoder_open(&decoder);    

// 3. Select archive items for extracting or testing.
//  3.1. Select all archive items.
plzma_item_array allArchiveItems = plzma_decoder_items(&decoder);

//  3.2. Get the number of items, iterate items by index, filter and select items.
size_t numberOfArchiveItems = plzma_decoder_count(&decoder);
plzma_item_array selectedItemsDuringIteration = plzma_item_array_create(numberOfArchiveItems);
plzma_item_out_stream_array selectedItemsToStreams = plzma_item_out_stream_array_create(numberOfArchiveItems);
for (size_t itemIndex = 0; itemIndex < numberOfArchiveItems; itemIndex++) {
    plzma_item item = plzma_decoder_item_at(&decoder, itemIndex);
    plzma_item_array_add(&selectedItemsDuringIteration, &item);
    
    plzma_out_stream outItemStream = plzma_out_stream_create_memory_stream(); // to memory stream
    plzma_item_out_stream_array_add(&selectedItemsToStreams, &item, &outItemStream);
    plzma_out_stream_release(&outItemStream);
    
    plzma_item_release(&item);
}

// 4. Extract or test selected archive items. The extract process might be:
//  4.1. Extract all items to a directory. In this case, you can skip the step #3.
plzma_path extractPath = plzma_path_create_with_utf8_string("path/outdir");
bool extracted = plzma_decoder_extract_all_items_to_path(&decoder, &extractPath, true);
plzma_path_release(&extractPath);

//  4.2. Extract selected items to a directory.
plzma_path extractPath = plzma_path_create_with_utf8_string("path/outdir");
bool extracted = plzma_decoder_extract_items_to_path(&decoder, &selectedItemsDuringIteration, &extractPath, true);
plzma_path_release(&extractPath);

//  4.3. Extract each item to a custom out-stream. 
//       The out-stream might be a file or memory. I.e. extract 'item #1' to a file stream, extract 'item #2' to a memory stream(then take extacted memory) and so on.
bool extracted = plzma_decoder_extract_item_out_stream_array(&decoder, &selectedItemsToStreams);

plzma_item_array_release(&selectedItemsDuringIteration); // when no longer needed 
plzma_item_array_release(&allArchiveItems); // when no longer needed
plzma_item_out_stream_array_release(&selectedItemsToStreams); // when no longer needed
plzma_decoder_release(&decoder); // when no longer needed
```

-----------
#### Compress
##### Create output, setup encoder/archive, add content, open & compress.  
The process consists of 4 steps:
1. Create output stream for writing archive's file content. The output stream might be created with:
   1. The path to the archive file.
   2. Create memory stream without any arguments.
2. Create encoder with output stream, type of the archive, compression method and optional progress delegate.
   1. Optionaly provide the password in case of header and/or content encryption.
   2. Setup archive properties.
3. Add content for archiving. The content might be next:
   1. Single file path with optional path inside the archive.
   2. Single directory path with optional directory iteration option and optional path inside the archive.
   3. Any input stream with required path inside the archive.
4. Open & compress.

##### Swift
```swift
do {
    // 1. Create output stream for writing archive's file content.
    //  1.1. Using file path.
    let archivePath = Path("path/out.7z");
    let archivePathOutStream = try OutStream(path: archivePath)
    
    // 2. Create encoder with output stream, type of the archive, compression method and optional progress delegate.
    let encoder = try Encoder(stream: archivePathOutStream, fileType: .sevenZ, method: .LZMA2, delegate: self)
    
    //  2.1. Optionaly provide the password in case of header and/or content encryption.
    try encoder.setPassword("1234")
    
    //  2.2. Setup archive properties.
    try encoder.setShouldEncryptHeader(true)  // use this option with password.
    try encoder.setShouldEncryptContent(true) // use this option with password.
    try encoder.setCompressionLevel(9)
    
    // 3. Add content for archiving.
    //  3.1. Single file path with optional path inside the archive.
    try encoder.add(path: Path("dir/my_file1.txt")) // store as "dir/my_file1.txt", as is.
    try encoder.add(path: Path("dir/my_file2.txt"), mode: .default, archivePath: Path("renamed_file2.txt")) // store as "renamed_file2.txt"
    
    //  3.2. Single directory path with optional directory iteration option and optional path inside the archive.
    try encoder.add(path: Path("dir/dir1")) // store as "dir1/..."
    try encoder.add(path: Path("dir/dir2"), mode: .followSymlinks, archivePath: Path("renamed_dir2")) // store as "renamed_dir2/..."
    
    //  3.3. Any input stream with required path inside the archive.
    let itemStream = try InStream(dataCopy: <Data>) // InStream(dataNoCopy: <Data>)
    try encoder.add(stream: itemStream, archivePath: Path("my_file3.txt")) // store as "my_file3.txt"
    
    // 4. Open.
    let opened = try encoder.open()
    
    // 4. Compress.
    let compressed = try encoder.compress()
} catch let exception as Exception {
    print("Exception: \(exception)")
}
```

##### JavaScript
```javascript
const plzma = require('plzmasdk');

try {
    // 1. Create output stream for writing archive's file content.
    //  1.1. Using file path.
    const archivePathOutStream = new plzma.OutStream('path/out.7z');

    // 2. Create encoder with output stream, type of the archive, compression method and optional progress delegate.
    const encoder = plzma.Encoder(archivePathOutStream, plzma.FileType.sevenZ, plzma.Method.LZMA2);
    encoder.setProgressDelegate((path, progress) => console.log(`Delegating progress, path: ${path}, progress: ${progress}`) );
    
    //  2.1. Optionaly provide the password in case of header and/or content encryption.
    encoder.setPassword('1234');

    //  2.2. Setup archive properties.
    encoder.shouldEncryptHeader = true;  // use this option with password.
    encoder.shouldEncryptContent = true; // use this option with password.
    encoder.compressionLevel = 9;
    
    // 3. Add content for archiving.
    //  3.1. Single file path with optional path inside the archive.
    encoder.add('dir/my_file1.txt'); // store as "dir/my_file1.txt", as is.
    encoder.add('dir/my_file2.txt', 0, 'renamed_file2.txt'); // store as "renamed_file2.txt"
    
    //  3.2. Single directory path with optional directory iteration option and optional path inside the archive.
    encoder.add('dir/dir1'); // store as "dir1/..."
    encoder.add('dir/dir2', plzma.OpenDirMode.followSymlinks, 'renamed_dir2'); // store as "renamed_dir2/..."

    //  3.3. Any input stream with required path inside the archive.
    const itemStream = plzma.InStream(new ArrayBuffer(...));
    encoder.add(itemStream, 'my_file3.txt'); // store as "my_file3.txt"
    
    // 4. Open.
    const opened = await encoder.openAsync(); // also available sync. version 'encoder.open()'
    
    // 4. Compress.
    const compressed = await encoder.compressAsync(); // also available sync. version 'encoder.compress()'
} catch (error) {
    console.log(`Exception: ${error}`);
}
```

##### C++
```cpp
try {
    // 1. Create output stream for writing archive's file content.
    //  1.1. Using file path.
    const auto archivePathOutStream = makeSharedOutStream(Path("path/out.7z"));

    // 2. Create encoder with output stream, type of the archive, compression method and optional progress delegate.
    auto encoder = makeSharedEncoder(archivePathOutStream, plzma_file_type_7z, plzma_method_LZMA2);
    encoder->setProgressDelegate(_progressDelegate);

    //  2.1. Optionaly provide the password in case of header and/or content encryption.
    encoder->setPassword("1234");
    
    //  2.2. Setup archive properties.
    encoder->setShouldEncryptHeader(true);   // use this option with password.
    encoder->setShouldEncryptContent(true);  // use this option with password.
    encoder->setCompressionLevel(9);

    // 3. Add content for archiving.
    //  3.1. Single file path with optional path inside the archive.
    encoder->add(Path("dir/my_file1.txt"));  // store as "dir/my_file1.txt", as is.
    encoder->add(Path("dir/my_file2.txt"), 0, Path("renamed_file2.txt")); // store as "renamed_file2.txt"

    //  3.2. Single directory path with optional directory iteration option and optional path inside the archive.
    encoder->add(Path("dir/dir1")); // store as "dir1/..."
    encoder->add(Path("dir/dir2"), plzma_open_dir_mode_follow_symlinks, Path("renamed_dir2")); // store as "renamed_dir2/..."

    //  3.3. Any input stream with required path inside the archive.
    auto itemStream = makeSharedInStream(<DATA>, <DATA_SIZE>);
    encoder->add(itemStream, Path("my_file3.txt")); // store as "my_file3.txt"
    
    // 4. Open.
    bool opened = encoder->open();

    // 4. Compress.
    bool compressed = encoder->compress();
} catch (const Exception & exception) {
    std::cout << "Exception: " << exception.what() << std::endl;
}
```

##### C
```c
// 1. Create output stream for writing archive's file content.
//  1.1. Using file path.
plzma_path archivePath = plzma_path_create_with_utf8_string("path/out.7z");
plzma_out_stream archivePathOutStream = plzma_out_stream_create_with_path(&archivePath);

// 2. Create encoder with output stream, type of the archive, compression method and optional progress delegate.
plzma_context context;
plzma_encoder encoder = plzma_encoder_create(&archivePathOutStream, plzma_file_type_7z, plzma_method_LZMA2, context);
plzma_encoder_set_progress_delegate_utf8_callback(&encoder, <C_CALLBACK_FUNCTION>);

//  2.1. Optionaly provide the password in case of header and/or content encryption.
plzma_encoder_set_password_utf8_string(&encoder, "1234");

//  2.2. Setup archive properties.
plzma_encoder_set_should_encrypt_header(&encoder, true);   // use this option with password.
plzma_encoder_set_should_encrypt_content(&encoder, true);  // use this option with password.
plzma_encoder_set_compression_level(&encoder, 9);

// 3. Add content for archiving.
//  3.1. Single file path with optional path inside the archive.
plzma_path itemPath = plzma_path_create_with_utf8_string("dir/my_file1.txt");
plzma_encoder_add_path(&encoder, &itemPath, 0, NULL); // store as "dir/my_file1.txt", as is.
plzma_path_release(&itemPath);

itemPath = plzma_path_create_with_utf8_string("dir/my_file2.txt");
plzma_path itemArchivePath = plzma_path_create_with_utf8_string("renamed_file2.txt");
plzma_encoder_add_path(&encoder, &itemPath, 0, &itemArchivePath); // store as "renamed_file2.txt"
plzma_path_release(&itemPath);
plzma_path_release(&itemArchivePath);

//  3.2. Single directory path with optional directory iteration option and optional path inside the archive.
itemPath = plzma_path_create_with_utf8_string("dir/dir1");
plzma_encoder_add_path(&encoder, &itemPath, 0, NULL); // store as "dir1/..."
plzma_path_release(&itemPath);

itemPath = plzma_path_create_with_utf8_string("dir/dir2");
itemArchivePath = plzma_path_create_with_utf8_string("renamed_dir2");
plzma_encoder_add_path(&encoder, &itemPath, plzma_open_dir_mode_follow_symlinks, &itemArchivePath); // store as "renamed_dir2/..."
plzma_path_release(&itemPath);
plzma_path_release(&itemArchivePath);

//  3.3. Any input stream with required path inside the archive.
itemArchivePath = plzma_path_create_with_utf8_string("my_file3.txt");
plzma_in_stream itemStream = plzma_in_stream_create_with_memory(<DATA>, <DATA_SIZE>);
plzma_encoder_add_stream(&encoder, &itemStream, &itemArchivePath); // store as "my_file3.txt"
plzma_in_stream_release(&itemStream);
plzma_path_release(&itemArchivePath);

// 4. Open.
bool opened = plzma_encoder_open(&encoder);

// 4. Compress.
bool compressed = plzma_encoder_compress(&encoder);

plzma_out_stream_release(&archivePathOutStream); // when no longer needed
plzma_path_release(&archivePath); // when no longer needed
plzma_encoder_release(&encoder); // when no longer needed
```


### License
-----------
By using this all you are accepting original [LZMA SDK] and MIT license (*see below*):

The MIT License (MIT)

Copyright (c) 2015 - 2022 Oleh Kulykov <olehkulykov@gmail.com>

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
[src]:https://github.com/OlehKulykov/PLzmaSDK/tree/master/src
[Swift]:https://swift.org
[Objective-C]:https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ProgrammingWithObjectiveC/Introduction/Introduction.html
[Swift Package Manager]:https://swift.org/package-manager
[CocoaPods]:https://cocoapods.org/pods/PLzmaSDK
[npm]:https://www.npmjs.com/package/plzmasdk
[CMake]:https://cmake.org
[git]:https://git-scm.com
[JavaScript]:https://en.wikipedia.org/wiki/JavaScript
