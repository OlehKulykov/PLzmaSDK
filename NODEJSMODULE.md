## Node.js native module API reference.


* plzmasdk
  * [ErrorCode](#enum_errorcode)
    * [.unknown](#enum_errorcode_unknown) ⇒ ```Number```
    * [.invalidArguments](#enum_errorcode_invalidarguments) ⇒ ```Number```
    * [.notEnoughMemory](#enum_errorcode_notenoughmemory) ⇒ ```Number```
    * [.io](#enum_errorcode_io) ⇒ ```Number```
    * [.internal](#enum_errorcode_internal) ⇒ ```Number```
  * [Erase](#enum_erase)
    * [.none](#enum_erase_none) ⇒ ```Number```
    * [.zero](#enum_erase_zero) ⇒ ```Number```
  * [FileType](#enum_filetype)
    * [.sevenZ](#enum_filetype_sevenz) ⇒ ```Number```
    * [.xz](#enum_filetype_xz) ⇒ ```Number```
    * [.tar](#enum_filetype_tar) ⇒ ```Number```
  * [Method](#enum_method)
    * [.LZMA](#enum_method_lzma) ⇒ ```Number```
    * [.LZMA2](#enum_method_lzma2) ⇒ ```Number```
    * [.PPMd](#enum_method_ppmd) ⇒ ```Number```
  * [OpenDirMode](#enum_opendirmode)
    * [.followSymlinks](#enum_opendirmode_followsymlinks) ⇒ ```Number```
  * [Stat](#obj_stat)
    * [.size](#obj_stat_size) ⇒ ```BigInt```
    * [.creation](#obj_stat_creation) ⇒ ```Date```
    * [.lastAccess](#obj_stat_lastaccess) ⇒ ```Date```
    * [.lastModification](#obj_stat_lastmodification) ⇒ ```Date```
  * [Path](#class_path)
    * [new Path()](#class_path_new)
    * [new Path([path])](#class_path_new_path)
    * [Path()](#class_path_new) ⇒ <code>[new Path()](#class_path_new)</code>
    * [Path([path])](#class_path_new_path) ⇒ <code>[new Path([path])](#class_path_new_path)</code>
    * [.toString()](#class_path_tostring) ⇒ ```String```
    * [.openDir([options])](#class_path_openDir) ⇒ <code>[PathIterator](#obj_pathiterator)</code>
    * [.clear([erase])](#class_path_clear) ⇒ <code>[Path](#class_path)</code> this/self object.
    * [.set([path])](#class_path_set) ⇒ <code>[Path](#class_path)</code> this/self object.
    * [.append([path])](#class_path_append) ⇒ <code>[Path](#class_path)</code> this/self object.
    * [.appendRandomComponent()](#class_path_appendrandomcomponent) ⇒ <code>[Path](#class_path)</code> this/self object.
    * [.lastComponent()](#class_path_lastcomponent) ⇒ <code>[Path](#class_path)</code>
    * [.removeLastComponent()](#class_path_removelastcomponent) ⇒ <code>[Path](#class_path)</code> this/self object.
    * [.remove([skipErrors])](#class_path_remove) ⇒ ```Boolean```
    * [.createDir([withIntermediates])](#class_path_createdir) ⇒ ```Boolean```
    * [.count](#class_path_count) ⇒ ```Number```
    * [.exists](#class_path_exists) ⇒ ```Number```
    * [.stat](#class_path_stat) ⇒ <code>[Stat](#obj_stat)</code>
    * [.readable](#class_path_readable) ⇒ ```Boolean```
    * [.writable](#class_path_writable) ⇒ ```Boolean```
    * [.readableAndWritable](#class_path_readableandwritable) ⇒ ```Boolean```
  * [PathIterator](#obj_pathiterator)
    * [.next()](#obj_pathiterator_next) ⇒ ```Boolean```
    * [.close()](#obj_pathiterator_close)
    * [.path](#obj_pathiterator_path) ⇒ <code>[Path](#class_path)</code>
    * [.component](#obj_pathiterator_component) ⇒ <code>[Path](#class_path)</code>
    * [.fullPath](#obj_pathiterator_fullpath) ⇒ <code>[Path](#class_path)</code>
    * [.isDir](#obj_pathiterator_isdir) ⇒ ```Boolean```
  * [Item](#class_item)
    * [new Item()](#class_item_new)
    * [new Item([path, [index]])](#class_item_new_path_index)
    * [Item()](#class_item_new) ⇒ <code>[new Item()](#class_item_new)</code>
    * [Item([path, [index]])](#class_path_new_path) ⇒ <code>[new Item([path, [index]])](#class_item_new_path_index)</code>
    * [.toString()](#class_item_tostring) ⇒ ```String```
    * [.path](#class_item_path) ⇒ <code>[Path](#class_path)</code>
    * [.index](#class_item_index) ⇒ ```Number```
    * [.size](#class_item_size) ⇒ ```BigInt```, ⇐ ```BigInt```|```Number```
    * [.packSize](#class_item_packsize) ⇒ ```BigInt```, ⇐ ```BigInt```|```Number```
    * [.crc32](#class_item_crc32) ⇔ ```Number```
    * [.creationDate](#class_item_creationdate) ⇒ ```Date```, ⇐ ```Date```|```Number```
    * [.accessDate](#class_item_accessdate) ⇒ ```Date```, ⇐ ```Date```|```Number```
    * [.modificationDate](#class_item_modificationdate) ⇒ ```Date```, ⇐ ```Date```|```Number```
    * [.encrypted](#class_item_encrypted) ⇔ ```Boolean```
    * [.isDir](#class_item_isdir) ⇔ ```Boolean```
  * [OutStream](#class_outstream)
    * [new OutStream()](#class_outstream_new)
    * [new OutStream([path])](#class_outstream_new_path)
    * [OutStream()](#class_outstream_new) ⇒ <code>[new OutStream()](#class_outstream_new)</code>
    * [OutStream([path])](#class_outstream_new_path) ⇒ <code>[new OutStream([path])](#class_outstream_new_path)</code>
    * [.erase([type])](#class_outstream_erase) ⇒ ```Boolean```
    * [.copyContent()](#class_outstream_copycontent) ⇒ ```ArrayBuffer```
    * [.opened](#class_outstream_opened) ⇒ ```Boolean```
  * [InStream](#class_instream)
    * [new InStream(path)](#class_instream_new_path)
    * [new InStream(fileContent)](#class_instream_new_file_content)
    * [InStream(path)](#class_instream_new_path) ⇒ <code>[new InStream(path)](#class_instream_new_path)</code>
    * [InStream(fileContent)](#class_instream_new_file_content) ⇒ <code>[new InStream(fileContent)](#class_instream_new_file_content)</code>
    * [.erase([type])](#class_instream_erase) ⇒ ```Boolean```
    * [.opened](#class_instream_opened) ⇒ ```Boolean```
  * [Decoder](#class_decoder)
    * [new Decoder(inStream, fileType)](#class_decoder_new)
    * [Decoder(inStream, fileType)](#class_decoder_new) ⇒ <code>[new Decoder(inStream, fileType)](#class_decoder_new)</code>
    * [.setProgressDelegate([delegate])](#class_decoder_set_progress_delegate)
    * [.setPassword([password])](#class_decoder_set_password)
    * [.open()](#class_decoder_open) ⇒ ```Boolean```
    * [.openAsync()](#class_decoder_open_async) ⇒ ```Promise```
    * [.abort()](#class_decoder_abort)
    * [.itemAt(index)](#class_decoder_item_at) ⇒ <code>[Item](#class_item)</code>
    * [.extract(map< Item, OutStream >)](#class_decoder_extract_items_map) ⇒ ```Boolean```
    * [.extract(path, [usingItemsFullPath])](#class_decoder_extract_path) ⇒ ```Boolean```
    * [.extract(array< Item >, path, [usingItemsFullPath])](#class_decoder_extract_array) ⇒ ```Boolean```
    * [.extractAsync(map< Item, OutStream >)](#class_decoder_extract_items_async_map) ⇒ ```Promise```
    * [.extractAsync(path, [usingItemsFullPath])](#class_decoder_extract_async_path) ⇒ ```Promise```
    * [.extractAsync(array< Item >, path, [usingItemsFullPath])](#class_decoder_extract_async_array) ⇒ ```Promise```
    * [.test([array< Item >])](#class_decoder_test) ⇒ ```Boolean```
    * [.testAsync([array< Item >])](#class_decoder_test_async) ⇒ ```Promise```
    * [.count](#class_decoder_count) ⇒ ```Number```
    * [.items](#class_decoder_items) ⇒ ```Array```
  * [Encoder](#class_encoder)
    * [new Encoder(outStream, fileType, method)](#class_encoder_new)
    * [Encoder(outStream, fileType, method)](#class_encoder_new) ⇒ <code>[new Encoder(outStream, fileType, method)](#class_encoder_new)</code>
    * [.setProgressDelegate([delegate])](#class_encoder_set_progress_delegate)
    * [.setPassword([password])](#class_encoder_set_password)
    * [.add(path, [openDirMode, [archivePath]])](#class_encoder_add_path)
    * [.add(inStream, [archivePath])](#class_encoder_add_instream)
    * [.open()](#class_encoder_open) ⇒ ```Boolean```
    * [.openAsync()](#class_encoder_open_async) ⇒ ```Promise```
    * [.abort()](#class_encoder_abort)
    * [.compress()](#class_encoder_compress) ⇒ ```Boolean```
    * [.compressAsync()](#class_encoder_compress_async) ⇒ ```Promise```
    * [.shouldCreateSolidArchive](#class_encoder_should_create_solid_archive) ⇔ ```Boolean```
    * [.compressionLevel](#class_encoder_compression_level) ⇔ ```Number```
    * [.shouldCompressHeader](#class_encoder_should_compress_header) ⇔ ```Boolean```
    * [.shouldCompressHeaderFull](#class_encoder_should_compress_header_full) ⇔ ```Boolean```
    * [.shouldEncryptContent](#class_encoder_should_encrypt_content) ⇔ ```Boolean```
    * [.shouldEncryptHeader](#class_encoder_should_encrypt_header) ⇔ ```Boolean```
    * [.shouldStoreCreationDate](#class_encoder_should_store_creation_date) ⇔ ```Boolean```
    * [.shouldStoreAccessDate](#class_encoder_should_store_access_date) ⇔ ```Boolean```
    * [.shouldStoreModificationDate](#class_encoder_should_store_modification_date) ⇔ ```Boolean```    
    

### <a name="enum_errorcode"></a>ErrorCode
Exported object with exception error codes.

#### <a name="enum_errorcode_unknown"></a>ErrorCode.unknown ⇒ Number
The error type cannot be determined.
Might be used as a default value during the initialization or generic, unknown exception was catched during the execution.

#### <a name="enum_errorcode_invalidarguments"></a>ErrorCode.invalidArguments ⇒ Number
Provided function or method arguments are invalid.

#### <a name="enum_errorcode_notenoughmemory"></a>ErrorCode.notEnoughMemory ⇒ Number
The required amount of memory can't be alocated or can't instantiate some object.

#### <a name="enum_errorcode_io"></a>ErrorCode.io ⇒ Number
File or directory doesn't exists or there are no permissions to perform required action.

#### <a name="enum_errorcode_internal"></a>ErrorCode.internal ⇒ Number
Any internal errors or exceptions.

### <a name="enum_erase"></a>Erase
Exported object with types of the erasing content.

#### <a name="enum_erase_none"></a>Erase.none ⇒ Number
The content will not be erased, i.e. ignored.

#### <a name="enum_erase_zero"></a>Erase.zero ⇒ Number
Erase, rewrite the content with zeros.

### <a name="enum_filetype"></a>FileType
Exported object with types of the file, stream or data buffer.

#### <a name="enum_filetype_sevenz"></a>FileType.sevenZ ⇒ Number
[7-zip](https://www.7-zip.org/7z.html) type. This file type supports multiple archive items, password protected items list of the arhive and password protected content.
Supports 'LZMA', 'LZMA2' and 'PPMd' compression methods.

#### <a name="enum_filetype_xz"></a>FileType.xz ⇒ Number
[XZ](https://www.7-zip.org/7z.html) type. This file type supports only one arhive item, in a rest it same as 7-zip. Supports only 'LZMA2' compression method which is automatically selected.

#### <a name="enum_filetype_tar"></a>FileType.tar ⇒ Number
[TAR](https://en.wikipedia.org/wiki/Tar_(computing)) type. All archive items are combined and stored as one continuous stream without compression. For this type, the 'Method' parameter is ignored.

### <a name="enum_method"></a>Method
Exported object with compression methods.

#### <a name="enum_method_lzma"></a>Method.LZMA ⇒ Number
Default and general compression method of 7z format.

#### <a name="enum_method_lzma2"></a>Method.LZMA2 ⇒ Number
Improved version of LZMA.

#### <a name="enum_method_ppmd"></a>Method.PPMd ⇒ Number
Dmitry Shkarin's PPMdH with small changes.

### <a name="enum_opendirmode"></a>OpenDirMode
Exported object with options for opening directory path. Currently uses for defining behavior of directory iteration.

#### <a name="enum_opendirmode_followsymlinks"></a>OpenDirMode.followSymlinks ⇒ Number
Follow the symbolic links.

### <a name="obj_stat"></a>Stat
Exported object with stat info of the path.

#### <a name="obj_stat_size"></a>Stat.size ⇒ BigInt
Size in bytes.

#### <a name="obj_stat_creation"></a>Stat.creation ⇒ Date
Path creation date.

#### <a name="obj_stat_lastaccess"></a>Stat.lastAccess ⇒ Date
Last path access date.

#### <a name="obj_stat_lastmodification"></a>Stat.lastModification ⇒ Date
Last path modification date.


### <a name="class_path"></a>Path
Exported optional path's string presentation.

#### <a name="class_path_new"></a>new Path()
Constructs empty Path.

#### <a name="class_path_new_path"></a>new Path([path])
Constructs Path with another path presentation.
* <code>path</code> {String|[Path](#class_path)} Optional path.

#### <a name="class_path_tostring"></a>Path.toString() ⇒ String
Returns string presentation of the path.
```js
const path = Path('a/b/c');
console.log(`path: ${path}`); // => 'a/b/c'
```

#### <a name="class_path_openDir"></a>Path.openDir([options]) ⇒ [PathIterator](#obj_pathiterator)
Opens a directory associated with path for iterating the content.
* <code>options</code> {[OpenDirMode](#enum_opendirmode)|Array<[OpenDirMode](#enum_opendirmode)>} Optional mode.

#### <a name="class_path_clear"></a>Path.clear([erase]) ⇒ [Path](#class_path) this/self object
Clears the path object's string presentation.
* <code>erase</code> {[Erase](#enum_erase)} Optional type of erasing of the path data.

#### <a name="class_path_set"></a>Path.set([path]) ⇒ [Path](#class_path) this/self object
Set the new path from optional source path.
* <code>path</code> {[String|[Path](#class_path)} Optional source string or path object.

#### <a name="class_path_append"></a>Path.append([path]) ⇒ [Path](#class_path) this/self object
Appends path component to path object.
* <code>path</code> {[String|[Path](#class_path)} Optional string or path object.

#### <a name="class_path_appendrandomcomponent"></a>Path.appendRandomComponent() ⇒ [Path](#class_path) this/self object
Appends random path component to the path object.
If path successfully updated, then the updated path doesn't exists in a root directory.
The component consists of ASCII characters in range ['a'; 'z'].

#### <a name="class_path_lastcomponent"></a>Path.lastComponent() ⇒ [Path](#class_path)
Receives the last component of the path.
The result is new instance of the path object representing the last component.

#### <a name="class_path_removelastcomponent"></a>Path.removeLastComponent() ⇒ [Path](#class_path) this/self object.
Removes the last component from the path.

#### <a name="class_path_remove"></a>Path.remove([skipErrors]) ⇒ Boolean
Physically removes the directory with all content or file associated with the path.
* <code>skipErrors</code> {Boolean} Optionaly skips errors and continuing removing or stop on first error.

#### <a name="class_path_createdir"></a>Path.createDir([withIntermediates]) ⇒ Boolean
Creates the directory at specific path.
* <code>withIntermediates</code> {Boolean} Optionaly create intermediate directories for each component or not.

#### <a name="class_path_count"></a>Path.count ⇒ Number
The number of unicode characters in a path string presentation.

#### <a name="class_path_exists"></a>Path.exists ⇒ Number
Checks the path exists.
Returns: 1 - file exists, 2 - directory exists, 0 - doesn't exist.

#### <a name="class_path_stat"></a>Path.stat ⇒ [Stat](#obj_stat)
Provides the stat info of the path. 

#### <a name="class_path_readable"></a>Path.readable ⇒ Boolean
Checks the path exists and has read permissions.

#### <a name="class_path_writable"></a>Path.writable ⇒ Boolean
Checks the path exists and has write permissions.

#### <a name="class_path_readableandwritable"></a>Path.readableAndWritable ⇒ Boolean
Checks the path exists and has read-write permissions.


### <a name="obj_pathiterator"></a>PathIterator
Path iterator object.

#### <a name="obj_pathiterator_next"></a>PathIterator.next() ⇒ Boolean
Continue iteration.

#### <a name="obj_pathiterator_close"></a>PathIterator.close()
Closes iteration.

#### <a name="obj_pathiterator_path"></a>PathIterator.path ⇒ [Path](#class_path)
Recevies the current file or directory path.

#### <a name="obj_pathiterator_component"></a>PathIterator.component ⇒ [Path](#class_path)
Recevies the current file or directory component.

#### <a name="obj_pathiterator_fullpath"></a>PathIterator.fullPath ⇒ [Path](#class_path)
Recevies the current file or directory full path, prefixed with root path.

#### <a name="obj_pathiterator_isdir"></a>PathIterator.isDir ⇒ Boolean
Checks the current iterator's path is directory.


### <a name="class_item"></a>Item
The archive item.

#### <a name="class_item_new"></a>new Item()
Constructs empty Item.

#### <a name="class_item_new_path_index"></a>new Item([path,[index]])
Constructs the Item with optional path and optional index in the archive.
* <code>path</code> {String|[Path](#class_path)} Optional, associated item's path.
* <code>index</code> {Number} Optional index of the item in the archive.

#### <a name="class_item_tostring"></a>Item.toString() ⇒ String
Returns string presentation of the item.
```js
const item = Item('a/b/c', 0);
console.log(`item: ${item}`);
```

#### <a name="class_item_path"></a>Item.path ⇒ [Path](#class_path)
Receives the item's path object.

#### <a name="class_item_index"></a>Item.index ⇒ Number
Receives the item's index inside the archive.

#### <a name="class_item_size"></a>Item.size ⇒ BigInt, ⇐ BigInt|Number
Read-Write property: receives or updates the item's index inside the archive.

#### <a name="class_item_packsize"></a>Item.packSize ⇒ BigInt, ⇐ BigInt|Number
Read-Write property: receives or updates the packed size in bytes of the item.

#### <a name="class_item_crc32"></a>Item.crc32 ⇒ Number, ⇐ Number
Read-Write property: receives or updates the CRC-32 checksum of the items content.

#### <a name="class_item_creationdate"></a>Item.creationDate ⇒ Date, ⇐ Date|Number
Read-Write property: receives or updates the creation date of the item.

#### <a name="class_item_accessdate"></a>Item.accessDate ⇒ Date, ⇐ Date|Number
Read-Write property: receives or updates the access date of the item.

#### <a name="class_item_modificationdate"></a>Item.modificationDate ⇒ Date, ⇐ Date|Number
Read-Write property: receives or updates the modification date of the item.

#### <a name="class_item_encrypted"></a>Item.encrypted ⇒ Boolean, ⇐ Boolean
Read-Write property: receives or updates the item is encrypted.

#### <a name="class_item_isdir"></a>Item.isDir ⇒ Boolean, ⇐ Boolean
Read-Write property: receives or updates the item is directory or file.


### <a name="class_outstream"></a>OutStream
The output file stream.

#### <a name="class_outstream_new"></a>new OutStream()
Constructs the output file stream object for writing to memory.

#### <a name="class_outstream_new_path"></a>new OutStream([path])
Constructs the output file stream object for writing to file.
* <code>path</code> {String|[Path](#class_path)} Optional output file path. In case if there is no path argument, the memory stream will be created.

#### <a name="class_outstream_erase"></a>OutStream.erase([type]) ⇒ Boolean
Erases and removes the content of the stream.
* <code>type</code> {[Erase](#enum_erase)} Optional type of erasing the content.

#### <a name="class_outstream_copycontent"></a>OutStream.copyContent() ⇒ ArrayBuffer
Copies the content of the stream to a heap memory. The stream must be closed.

#### <a name="class_outstream_opened"></a>OutStream.opened ⇒ Boolean
Checks the output file stream is opened.


### <a name="class_instream"></a>InStream
The input file stream.

#### <a name="class_instream_new_path"></a>new InStream(path)
Constructs the input file stream object for reading a file content from path.
* <code>path</code> {String|[Path](#class_path)} File path or path string.

#### <a name="class_instream_new_file_content"></a>new InStream(fileContent)
Constructs the input file stream object for reading a file content.
* <code>fileContent</code> {ArrayBuffer} Input file content.

#### <a name="class_instream_erase"></a>InStream.erase([type]) ⇒ Boolean
Erases and removes the content of the stream.
* <code>type</code> {[Erase](#enum_erase)} Optional type of erasing the content.

#### <a name="class_instream_opened"></a>InStream.opened ⇒ Boolean
Checks the input file stream is opened.


### <a name="class_decoder"></a>Decoder
The decoder for extracting or testing the archive items.

#### <a name="class_decoder_new"></a>new Decoder(inStream, fileType)
Constructs the decoder with input file stream and file type.
* <code>inStream</code> {[InStream](#class_instream)} The input stream which contains the archive file content.
* <code>fileType</code> {[FileType](#enum_filetype)} The type of the arhive file content.

#### <a name="class_decoder_set_progress_delegate"></a>Decoder.setProgressDelegate([delegate])
Provides the extract or test progress delegate.
* <code>delegate</code> {function(path{String}, progress{Number})} Optional delegate function to report the progress.

#### <a name="class_decoder_set_password"></a>Decoder.setPassword([password])
Provides the archive password for opening, extracting or testing items.
* <code>password</code> {String} Optional password.

#### <a name="class_decoder_open"></a>Decoder.open() ⇒ Boolean
Opens the archive.

#### <a name="class_decoder_open"></a>Decoder.openAsync() ⇒ Promise
Asynchronously opens the archive. The opening progress might be aborted via [abort()](#class_decoder_abort) function.

#### <a name="class_decoder_abort"></a>Decoder.abort()
Aborts opening, extracting or testing process. The aborted decoder is no longer valid.

#### <a name="class_decoder_item_at"></a>Decoder.itemAt(index) ⇒ [Item](#class_item)
Receives a single archive item at a specific index.
* <code>index</code> {Number} The index of the item inside the arhive. Must be less then the number of items reported by the [count()](#class_decoder_count) property.

#### <a name="class_decoder_extract_items_map"></a>Decoder.extract(map<[Item](#class_item), [OutStream](#class_outstream)>) ⇒ Boolean
Extracts each archive item to a separate out-stream.
* <code>map</code> {Map} The item/out-stream map. The key is [Item](#class_item) and the value is [OutStream](#class_outstream).

#### <a name="class_decoder_extract_path"></a>Decoder.extract(path, [usingItemsFullPath]) ⇒ Boolean
Extracts all archive items to a specific path. 
* <code>path</code> {String|[Path](#class_path)} The directory path to extract all items.
* <code>usingItemsFullPath</code> {Boolean} Optionally extract item using it's full path or only last path component. Default is {true}.

#### <a name="class_decoder_extract_array"></a>Decoder.extract(array<[Item](#class_item)>, path, [usingItemsFullPath]) ⇒ Boolean
Extracts some archive items to a specific path.
* <code>array</code> {Array} The array of items to extract.
* <code>usingItemsFullPath</code> {Boolean} Optionally extract item using it's full path or only last path component. Default is {true}.

#### <a name="class_decoder_extract_items_async_map"></a>Decoder.extractAsync(map<[Item](#class_item), [OutStream](#class_outstream)>) ⇒ Promise
Asynchronously extracts each archive item to a separate out-stream. The extracting progress might be aborted via [abort()](#class_decoder_abort) function.
* <code>map</code> {Map} The item/out-stream map. The key is [Item](#class_item) and the value is [OutStream](#class_outstream).

#### <a name="class_decoder_extract_async_path"></a>Decoder.extractAsync(path, [usingItemsFullPath]) ⇒ Promise
Asynchronously extracts all archive items to a specific path. The extracting progress might be aborted via [abort()](#class_decoder_abort) function.
* <code>path</code> {String|[Path](#class_path)} The directory path to extract all items.
* <code>usingItemsFullPath</code> {Boolean} Optionally extract item using it's full path or only last path component. Default is {true}.

#### <a name="class_decoder_extract_async_array"></a>Decoder.extractAsync(array<[Item](#class_item)>, path, [usingItemsFullPath]) ⇒ Promise
Asynchronously extracts some archive items to a specific path. The extracting progress might be aborted via [abort()](#class_decoder_abort) function.
* <code>array</code> {Array} The array of items to extract.
* <code>usingItemsFullPath</code> {Boolean} Optionally extract item using it's full path or only last path component. Default is {true}.

#### <a name="class_decoder_test"></a>Decoder.test([array< Item >]) ⇒ Boolean
Tests all or specific archive items.
* <code>array</code> {Array} Optional array of items to test.

#### <a name="class_decoder_test_async"></a>Decoder.testAsync([array< Item >]) ⇒ Promise
Asynchronously tests all or specific archive items. The testing progress might be aborted via [abort()](#class_decoder_abort) function.
* <code>array</code> {Array} Optional array of items to test.

#### <a name="class_decoder_count"></a>Decoder.count ⇒ Number
Receives the number of items in archive. The decoder must be opened.

#### <a name="class_decoder_items"></a>Decoder.items ⇒ Array
Receives array with all archive items. The decoder must be opened.


### <a name="class_encoder"></a>Encoder
The decoder compressing the archive items.

#### <a name="class_encoder_new"></a>new Encoder(outStream, fileType, method)
Constructs the encoder with output file stream, file type and method.
* <code>outStream</code> {[OutStream](#class_outstream)} The output file stream to write the archive's file content.
* <code>fileType</code> {[FileType](#enum_filetype)} The type of the arhive file content.
* <code>method</code> {[Method](#enum_method)} The compresion method.

#### <a name="class_encoder_set_progress_delegate"></a>Encoder.setProgressDelegate([delegate])
Provides the compression progress delegate.
* <code>delegate</code> {function(path{String}, progress{Number})} Optional delegate function to report the progress.

#### <a name="class_encoder_set_password"></a>Encoder.setPassword([password])
Provides the archive password.
* <code>password</code> {String} Optional password.

#### <a name="class_encoder_add_path"></a>Encoder.add(path, [openDirMode, [archivePath]])
Adds the physical file or directory path to the encoder.
* <code>path</code> {String|[Path](#class_path)} The file or directory path.
* <code>openDirMode</code> {[OpenDirMode](#enum_opendirmode)|Array<[OpenDirMode](#enum_opendirmode)>} Optional mode for opening directory in case if path is a directory path.
* <code>archivePath</code> {String|[Path](#class_path)} Optional path of how the item's path will be presented in archive.

#### <a name="class_encoder_add_instream"></a>Encoder.add(inStream, [archivePath])
Adds the file in-stream to the encoder.
* <code>inStream</code> {[InStream](#class_instream)} The input file stream to add.
* <code>archivePath</code> {String|[Path](#class_path)} Optional path of how the item's path will be presented in archive.

#### <a name="class_encoder_open"></a>Encoder.open() ⇒ Boolean
Opens the archive.

#### <a name="class_encoder_open_async"></a>Encoder.openAsync() ⇒ Promise
Asynchronously opens the archive. The testing progress might be aborted via [abort()](#class_encoder_abort) function.

#### <a name="class_encoder_abort"></a>Encoder.abort()
Aborts the opening or compressing process.

#### <a name="class_encoder_compress"></a>Encoder.compress() ⇒ Boolean
Compresses the provided paths and streams.

#### <a name="class_encoder_compress"></a>Encoder.compressAsync() ⇒ Promise
Asynchronously compresses the provided paths and streams.

#### <a name="class_encoder_should_create_solid_archive"></a>Encoder.shouldCreateSolidArchive ⇔ Boolean
Read-Write property: receives or updates for a 'solid' archive property. Default true.

#### <a name="class_encoder_compression_level"></a>Encoder.compressionLevel ⇔ Number
Read-Write property: receives or updates compression level. The level in a range [0; 9].

#### <a name="class_encoder_should_compress_header"></a>Encoder.shouldCompressHeader ⇔ Boolean
Read-Write property: should encoder compress the archive header. Default true.

#### <a name="class_encoder_should_compress_header_full"></a>Encoder.shouldCompressHeaderFull ⇔ Boolean
Read-Write property: should encoder fully compress the archive header. Default true.

#### <a name="class_encoder_should_encrypt_content"></a>Encoder.shouldEncryptContent ⇔ Boolean
Read-Write property: should encoder encrypt the content of the archive items. The password will be required to decode/extract archive items.

#### <a name="class_encoder_should_encrypt_header"></a>Encoder.shouldEncryptHeader ⇔ Boolean
Read-Write property: should encoder encrypt the header with the list of archive items. The password will be required to open archive and list the items.

#### <a name="class_encoder_should_store_creation_date"></a>Encoder.shouldStoreCreationDate ⇔ Boolean
Read-Write property: should encoder store the creation date of each item to the header if such available. Default true.

#### <a name="class_encoder_should_store_access_date"></a>Encoder.shouldStoreAccessDate ⇔ Boolean
Read-Write property: should encoder store the last access date of each item to the header if such available. Default true.

#### <a name="class_encoder_should_store_modification_date"></a>Encoder.shouldStoreModificationDate ⇔ Boolean
Read-Write property: should encoder store the last modification date of each item to the header if such available. Default true.
