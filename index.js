
'use strict';

var plzma = null;
if (process && process.platform === "win32" && process.arch === "x64") {
    plzma = require('./bin/winx64/plzma');  
} else if (process && process.platform === "win32" && process.arch === "ia32") {
    plzma = require('./bin/winx86/plzma');  
} else {
    plzma = require('./build/Release/plzma');  
}
module.exports = plzma;

/**
enum ErrorCode {
    unknown
    invalidArguments
    notEnoughMemory
    io
    internal
}
*/

// enum Erase {
//     none
//     zero
// }

// enum FileType {
//     sevenZ
//     xz
//     tar
// }

// enum Method {
//     LZMA
//     LZMA2
//     PPMd
// }

// enum OpenDirMode {
//     followSymlinks
// }

// version

// class Stat {
//     size
//     creation
//     lastAccess
//     lastModification
// }

// class Path {
//     constructor()
//     toString()
//     openDir()
//     clear()
//     set()
//     append()
//     appendRandomComponent()
//     lastComponent()
//     removeLastComponent()
//     remove()
//     createDir()
//     count
//     exists
//     stat
//     readable
//     writable
//     readableAndWritable
// }

// class PathIterator {
//     next()
//     close()
//     path
//     component
//     fullPath
//     isDir
// }

// class Item {
//     constructor()
//     toString()
//     path
//     index
//     size
//     packSize
//     crc32
//     creationDate
//     accessDate
//     modificationDate
//     encrypted
//     isDir
// }


// class OutStream {
//     constructor()
//     erase()
//     copyContent()
//     opened
// }

// class InStream {
//     constructor()
//     erase()
//     opened
// }

// class Decoder {
//     constructor()
//     setProgressDelegate()
//     setPassword()
//     open()
//     openAsync()
//     abort()
//     itemAt()
//     extract()
//     extractAsync()
//     test()
//     testAsync()
//     count
//     items
// }

// class Encoder {
//     constructor()
//     setProgressDelegate()
//     setPassword()
//     add()
//     open()
//     openAsync()
//     abort()
//     compress()
//     compressAsync()
//     shouldCreateSolidArchive
//     compressionLevel
//     shouldCompressHeader
//     shouldCompressHeaderFull
//     shouldEncryptContent
//     shouldEncryptHeader
//     shouldStoreCreationTime
//     shouldStoreAccessTime
//     shouldStoreModificationTime
// }
