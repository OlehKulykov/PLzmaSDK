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
  * [PathIterator](#obj_pathiterator)
    * [.next()](#obj_pathiterator_next) ⇒ ```Boolean```
    * [.close()](#obj_pathiterator_close)
    * [.path](#obj_pathiterator_path) ⇒ <code>[Path](#class_path)</code>
    * [.component](#obj_pathiterator_component) ⇒ <code>[Path](#class_path)</code>
    * [.fullPath](#obj_pathiterator_fullpath) ⇒ <code>[Path](#class_path)</code>
    * [.isDir](#obj_pathiterator_isdir) ⇒ ```Boolean```


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

