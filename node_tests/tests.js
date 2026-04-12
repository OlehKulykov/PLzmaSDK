
'use strict';

const plzma = require('./../');

console.log('plzma export:', plzma);
console.log('typeof plzma.ErrorCode.io:', typeof plzma.ErrorCode.io);
let test = 1;

if (typeof plzma.streamReadSize !== 'number') throw '';
if (plzma.streamReadSize <= 0) throw '';
plzma.streamReadSize = 3332;
if (plzma.streamReadSize != 3332) throw '';

if (typeof plzma.streamWriteSize !== 'number') throw '';
if (plzma.streamWriteSize <= 0) throw '';
plzma.streamWriteSize = 3332;
if (plzma.streamWriteSize != 3332) throw '';

if (typeof plzma.decoderReadSize !== 'number') throw '';
if (plzma.decoderReadSize <= 0) throw '';
plzma.decoderReadSize = 3332;
if (plzma.decoderReadSize != 3332) throw '';

if (typeof plzma.decoderWriteSize !== 'number') throw '';
if (plzma.decoderWriteSize <= 0) throw '';
plzma.decoderWriteSize = 3332;
if (plzma.decoderWriteSize != 3332) throw '';

let path, path1, path2;
for (let i = 1; i < 8; i++) {
    let arg;
    switch (i) {
        case 1: arg = ''; path = new plzma.Path(); break;
        case 2: arg = ''; path = new plzma.Path(arg); break;
        case 3: arg = __dirname; path = new plzma.Path(arg); break;
        case 4: arg = ''; path = plzma.Path(); break;
        case 5: arg = ''; path = plzma.Path(arg); break;
        case 6: arg = __dirname; path = plzma.Path(arg); break;
        case 7: arg = 'päth'; path = plzma.Path(arg); break;
    };
    if (typeof path.exists !== 'number') throw '';
    if (typeof path.count !== 'number') throw '';
    if (arg.length !== path.count) throw '';
    if (arg.length !== path.toString().length) throw '';
    if (arg !== path.toString()) throw '';
    console.log(`${test}.${i}. path: \'${path}\', arg: \'${arg}\'`, ', path.toString():\'' + path.toString() + '\'');
    path1 = path.lastComponent();
    if (!path1) throw '';
    console.log(`${test}.${i}. last component: \'${path1}\'`);
    let count = path.count;
    path.append('comp');
    if (count === path.count) throw '';
    path1 = path.lastComponent();
    if (path1.toString() !== 'comp') throw '';
    if (typeof path1.exists !== 'number') throw '';
    path.removeLastComponent();
    if (count !== path.count) throw '';
    path.clear(plzma.Erase.zero);
    if (path.count !== 0) throw '';
}

test++;
path = plzma.Path.tmpPath;
console.log(`${test}. tmp: \'${path}\'`);
if (path.exists !== 2) throw ''; // 1 -> file, 2 -> dir, 0 -> false
let stat = path.stat;
if (typeof stat !== 'object') throw '';
if (typeof stat.size !== 'bigint') throw '';
if ((stat.creation instanceof Date) !== true) throw '';
if ((stat.lastAccess instanceof Date) !== true) throw '';
if ((stat.lastModification instanceof Date) !== true) throw '';
console.log(''+test+'.1. stat:', stat);
path.appendRandomComponent();
if (path.exists !== 0) throw ''; // 1 -> file, 2 -> dir, 0 -> false
console.log(`${test}.2. append random component: \'${path}\'`);
if (path.createDir(true) !== true) throw '';
if (path.exists !== 2) throw ''; // 1 -> file, 2 -> dir, 0 -> false
if (path.readable !== true) throw '';
if (path.writable !== true) throw '';
if (path.readableAndWritable !== true) throw '';
if (path.remove(false) !== true) throw '';
if (path.exists !== 0) throw ''; // 1 -> file, 2 -> dir, 0 -> false
path.removeLastComponent();
console.log(`${test}.3. remove last component: \'${path}\'`);
if (path.exists !== 2) throw ''; // 1 -> file, 2 -> dir, 0 -> false

test++;
console.log(`${test}.4. appending: \'${path}\'`);
path1 = plzma.Path('a');
path2 = plzma.Path('b');
if (path1.toString() !== 'a' || path2.toString() !== 'b') throw '';
path = path1.appending('b');
if ( !(path.toString() === 'a/b' || path.toString() === 'a\\b') ) throw '';
path = path1.appending(path2);
if ( !(path.toString() === 'a/b' || path.toString() === 'a\\b') ) throw '';
path = path1.appending('');
if (path.toString() !== path1.toString() || path.toString() !== 'a') throw '';

test++;
console.log(`${test}.5. appendingRandomComponent`);
path = plzma.Path().appendingRandomComponent();
console.log(`${path}`);
if (path.count === 0) throw '';
let count = path.count;
path1 = path.appendingRandomComponent();
console.log(`${path1}`);
if (path1.count <= path.count || count !== path.count) throw '';

test++;
console.log(`${test}.5. removingLastComponent`);
path = plzma.Path();
if (path.count !== 0) throw '' ;
count = path.count;
path1 = path.removingLastComponent();
if (path1.count !== 0 || path.count !== 0 || count !== path.count) throw '';

path = plzma.Path('a');
if (path.count === 0) throw '' ;
count = path.count;
path1 = path.removingLastComponent();
if (path.count !== count) throw '' ;
if (path1.count > 0) throw '' ;

path = plzma.Path('a/b');
if (path.count === 0) throw '' ;
count = path.count;
path1 = path.removingLastComponent();
if (path.count !== count) throw '' ;
if (path1.count === 0) throw '' ;
if (path1.count >= path.count) throw '' ;
if (path1.toString() !== 'a') throw '' ;
if ( !(path.toString() === 'a/b' || path.toString() === 'a\\b') ) throw '' ;

test++;
path = plzma.Path.tmpPath;
let error = undefined;
let iterator;
try { iterator = path.appendRandomComponent().openDir(); } catch (e) { error = e; }
if (!error) throw '';
path = plzma.Path.tmpPath;
if (path.appendRandomComponent().createDir() !== true) throw '';
path.removeLastComponent();
iterator = path.openDir([plzma.OpenDirMode.followSymlinks]);
console.log(`${test}. iterator: ${iterator}`, iterator);
while (iterator.next()) {
    console.log(`${test}.1. iterator: isDir: ${iterator.isDir}, component: ${iterator.component}, path: ${iterator.path}, fullPath: ${iterator.fullPath}`);
}
iterator.close();

test++;
path = plzma.Path.tmpPath;
error = undefined;
try { path1 = new plzma.Path({id:1}); } catch (e) { error = e; }
if (!error) throw '';
console.log(`${test}.1. path1: ${path1}`);
path1 = new plzma.Path(path);
console.log(`${test}.2. path1: ${path1}`);
if (path1.count !== path.count) throw '';

test++;
let item = new plzma.Item('a\\b/c', 1);
console.log(`${test}. ${item}`, item);
console.log(`${test}.1 ${item.modificationDate}`);
item.modificationDate = Date.now();
console.log(`${test}.2 ${item.modificationDate}`);

test++;
console.log(`${test}.1 ${item.size}`);
item.size = 33;
console.log(`${test}.2 ${item.size}`);
item.size = 0;
console.log(`${test}.2 ${item.size}`);

test++; // decoder.open
async function decoderAsyncTest1() {
    const t = test;
    console.log(`${t}. decoderAsyncTest1 enter`);
    const path = plzma.Path(__dirname).append('../test_files/1.7z');
    const decoder = plzma.Decoder(plzma.InStream(path), plzma.FileType.sevenZ);
    decoder.setProgressDelegate((path, progress) => console.log(`${t}. testing: ${path} progress: ${progress}`) );
    const openRes = await decoder.openAsync();
    console.log(`${t}. openRes: ${openRes}`);
    const testRes = await decoder.testAsync();
    console.log(`${t}. testRes: ${testRes}`);
    decoder.setProgressDelegate((path, progress) => console.log(`${t}. testing2: ${path} progress: ${progress}`) );
    console.log(`${t}. trying test 2`);
    const testRes2 = await decoder.testAsync([]);
    console.log(`${t}. testRes2: ${testRes2}`);

    const items = [];
    for (let i = 0, n = decoder.count; i < n; i++) { 
        items.push(decoder.itemAt(i)); 
    }
    decoder.setProgressDelegate((path, progress) => console.log(`${t}. testing3: ${path} progress: ${progress}`) );
    const testRes3 = await decoder.testAsync(items);
    console.log(`${t}. testRes3: ${testRes3}`);
    console.log(`${t}. decoderAsyncTest1 exit`);
    
    const itemsMap = new Map();
    for (let i = 0, n = decoder.count; i < n; i++) { 
        itemsMap.set(decoder.itemAt(i), plzma.OutStream()); 
    }
    console.log(`${t}. itemsMap: ${itemsMap}`);
    decoder.setProgressDelegate((path, progress) => console.log(`${t}. extracting1: ${path} progress: ${progress}`) );
    const extractRes1 = await decoder.extractAsync(itemsMap);
    console.log(`${t}. extractRes1: ${extractRes1}`);
}

//decoderAsyncTest1();

test++; // decoder.open
async function decoderAsyncExample1() {
    const t = test;
    console.log(`${t}. decoderAsyncExample1 enter`);
    try {
        // 1. Create a source input stream for reading archive file content.
        //  1.1. Create a source input stream with the path to an archive file.
        const archivePath = plzma.Path(__dirname).append('../test_files/1.7z');
        const archivePathInStream = new plzma.InStream(archivePath);

        //  1.2. Create a source input stream with the file content in memory.
        const archiveData = new ArrayBuffer(8);
        const archiveDataInStream = new plzma.InStream(archiveData);

        // 2. Create decoder with source input stream, type of archive and optional delegate.
        const decoder = new plzma.Decoder(archivePathInStream, plzma.FileType.sevenZ);
        decoder.setProgressDelegate((path, progress) => console.log(`${t}. Delegating progress, path: ${path}, progress: ${progress}`) );

        //  2.1. Optionaly provide the password to open/list/test/extract the archive items.
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
            selectedItemsToStreams.set(item, plzma.OutStream());
        }

        const extracted = await decoder.extractAsync(selectedItemsToStreams);
    } catch (error) {
        console.log(`${t}. Exception: ${error}`);
    }
}

decoderAsyncExample1();

test++; // testDecodeMultiVolume
async function testDecodeMultiVolume() {
    const t = test;
    console.log(`${t}. testDecodeMultiVolume enter`);
    try {
        let streams = [ 
            plzma.InStream(plzma.Path(__dirname).append('../test_files/18.7z.001')),
            plzma.InStream(plzma.Path(__dirname).append('../test_files/18.7z.002')),
            plzma.InStream(plzma.Path(__dirname).append('../test_files/18.7z.003')) 
        ];
        
        let decoder = plzma.Decoder(plzma.InStream(streams), plzma.FileType.sevenZ);
        decoder.setPassword('1234');
        let opened = await decoder.openAsync();
        if (!opened) { throw 'opened' }
        let numberOfArchiveItems = decoder.count;
        if (numberOfArchiveItems !== 5) { throw 'numberOfArchiveItems !== 5' }

        let memoryMap = new Map();
        let i = 0;
        while (i < 5) {
            memoryMap.set(decoder.itemAt(i), new plzma.OutStream());
            i++;
        }
        if (!(memoryMap.size === 5)) { throw 'memoryMap.size === 5' }
        
        let extracted = await decoder.extractAsync(memoryMap);
        if (!extracted) { throw 'extracted' }

        for (const [item, stream] of memoryMap) {
            let content = stream.copyContent();
            let erased = stream.erase(plzma.Erase.zero);
            if (!erased) { throw 'erased' }
            let itemPathString = item.path.toString();
            switch (itemPathString) {
                case 'shutuptakemoney.jpg':
                    if (!(content.byteLength === 33398)) { throw 'content.byteLength === 33398' }
                    console.log(`${t}. Check ${itemPathString} ok`);
                    break;
                case 'SouthPark.jpg':
                    if (!(content.byteLength === 40778)) { throw 'content.byteLength === 40778' }
                    console.log(`${t}. Check ${itemPathString} ok`);
                    break;
                case 'zombies.jpg':
                    if (!(content.byteLength === 83127)) { throw 'content.byteLength === 83127' }
                    console.log(`${t}. Check ${itemPathString} ok`);
                    break;
                case 'Мюнхен.jpg':
                case 'München.jpg':
                    if (!(content.byteLength === 8764)) { throw 'content.byteLength === 8764' }
                    console.log(`${t}. Check ${itemPathString} ok`);
                    break;
                default:
                    throw '';
            }
        }
    } catch (error) {
        console.log(`${t}. Exception: ${error}`);
    }
    console.log(`${t}. testDecodeMultiVolume exit`);
}

testDecodeMultiVolume();

test++; // testEncodeMultiVolume
async function testEncodeMultiVolume() {
    const t = test;
    console.log(`${t}. testEncodeMultiVolume enter`);
    try {
        //case fileNoPassword = 0 , memNoPassword = 1, fileWithPassword = 2, memWithPassword = 3
        for (let useCaseIndex = 0; useCaseIndex < 4; useCaseIndex++) {
            console.log(`${t}. testEncodeMultiVolume useCaseIndex: ${useCaseIndex}`);
            let path = plzma.Path.tmpPath.appendingRandomComponent();
            let partSize = 32 * 1024;
            let multiStream;
            switch (useCaseIndex) {
                case 0:
                case 2:
                    multiStream = new plzma.OutMultiStream(path, 'file', '7z', plzma.MultiStreamPartNameFormat.nameExt00x, partSize);
                    break;
                case 1:
                case 3:
                    multiStream = new plzma.OutMultiStream(partSize);
                    break;
                default:
                    break;
            }
            let encoder = new plzma.Encoder(multiStream, plzma.FileType.sevenZ, plzma.Method.LZMA);
            if (useCaseIndex === 2 || useCaseIndex === 3) {
                encoder.setPassword('hello');
            }
            encoder.add(plzma.InStream(plzma.Path(__dirname).append('../test_files/shutuptakemoney.jpg')), 'shutuptakemoney.jpg');
            encoder.add(plzma.InStream(plzma.Path(__dirname).append('../test_files/SouthPark.jpg')), plzma.Path('SouthPark.jpg'));
            encoder.add(plzma.InStream(plzma.Path(__dirname).append('../test_files/zombies.jpg')), plzma.Path('zombies.jpg'));
            encoder.add(plzma.InStream(plzma.Path(__dirname).append('../test_files/Мюнхен.jpg')), 'Мюнхен.jpg');
            encoder.add(plzma.InStream(plzma.Path(__dirname).append('../test_files/München.jpg')), 'München.jpg');
            //encoder.setProgressDelegate((path, progress) => console.log(`${t}. Encode ${useCaseIndex}: delegating progress, path: ${path}, progress: ${progress}`) );

            let opened;
            if (useCaseIndex % 2) {
                opened = await encoder.openAsync();
            } else {
                opened = encoder.open();
            }
            if (!opened) { throw 'opened' }
            
            let compressed;
            if (useCaseIndex % 2) {
                compressed = encoder.compress();
            } else {
                compressed = await encoder.compressAsync();
            }
            if (!compressed) { throw 'compressed' }

            encoder.abort();

            let content = multiStream.copyContent();
            let totalPartsSize = 0;
            switch (useCaseIndex) {
                case 0:
                case 2: {
                    let pathIterator = path.openDir();
                    while (pathIterator.next()) {
                        let fullPath = pathIterator.fullPath;
                        console.log(`${fullPath}`);
                        let partStat = fullPath.stat;
                        totalPartsSize += Number(partStat.size);
                    }
                    if (content.byteLength !== totalPartsSize) { throw 'content.byteLength !== totalPartsSize' }
                } break;
            
                default:
                    break;
            }
            
            let outMultiStreams = multiStream.streams;
            if (!(outMultiStreams.length > 0)) { throw 'outMultiStreams.length > 0' }

            totalPartsSize = 0;
            let i = 0;
            for (let outMultiStream of outMultiStreams) {
                let partContent = outMultiStream.copyContent();
                if (i + 1 == outMultiStreams.length) {
                    if (!(partContent.byteLength <= partSize)) { throw 'partContent.byteLength <= partSize' }
                } else {
                    if (!(partContent.byteLength === partSize)) { throw 'partContent.byteLength === partSize' }
                }
                totalPartsSize += partContent.byteLength;
                i += 1;
            }
            if (!(content.byteLength === totalPartsSize)) { throw 'content.byteLength === totalPartsSize' }
            let erased = multiStream.erase();
            if (!erased) { throw 'erased' }
            
            let decoder = plzma.Decoder(plzma.InStream(content), plzma.FileType.sevenZ);
            //decoder.setProgressDelegate((path, progress) => console.log(`${t}. Decode ${useCaseIndex}: delegating progress, path: ${path}, progress: ${progress}`) );
            if (useCaseIndex === 2 || useCaseIndex === 3) {
                decoder.setPassword('hello');
            }
            console.log(`${t}. `);
            opened = decoder.open();
            if (!erased) { throw 'opened' }
            
            let itemsCount = decoder.count;
            if (!(itemsCount === 5)) { throw 'itemsCount === 5' }
                
            let memoryMap = new Map();
            let fileMap = new Map();
            i = 0;
            while (i < 5) {
                memoryMap.set(decoder.itemAt(i), new plzma.OutStream());
                fileMap.set(decoder.itemAt(i), new plzma.OutStream(plzma.Path.tmpPath.appendingRandomComponent()));
                i += 1;
            }
            if (!(memoryMap.size === 5)) { throw 'memoryMap.size === 5' }
            if (!(fileMap.size === 5)) { throw 'fileMap.size === 5' }
            

            let extracted;
            if (useCaseIndex % 2) {
                extracted = decoder.extract(memoryMap);
                if (!extracted) { throw 'extracted' }
                console.log(`${t}. `);
                extracted = await decoder.extractAsync(fileMap);
                if (!extracted) { throw 'extracted' }
            } else {
                extracted = await decoder.extractAsync(memoryMap);
                if (!extracted) { throw 'extracted' }
                console.log(`${t}. `);
                extracted = decoder.extract(fileMap);
                if (!extracted) { throw 'extracted' }
            }
            
            for (const [item, stream] of memoryMap) {
                let content = stream.copyContent();
                erased = stream.erase(plzma.Erase.zero);
                if (!erased) { throw 'erased' }
                let itemPathString = item.path.toString();
                switch (itemPathString) {
                    case 'shutuptakemoney.jpg':
                        if (!(content.byteLength === 33398)) { throw 'content.byteLength === 33398' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'SouthPark.jpg':
                        if (!(content.byteLength === 40778)) { throw 'content.byteLength === 40778' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'zombies.jpg':
                        if (!(content.byteLength === 83127)) { throw 'content.byteLength === 83127' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'Мюнхен.jpg':
                    case 'München.jpg':
                        if (!(content.byteLength === 8764)) { throw 'content.byteLength === 8764' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    default:
                        throw '';
                }
            }

            for (const [item, stream] of fileMap) {
                let content = stream.copyContent();
                erased = stream.erase(plzma.Erase.zero);
                if (!erased) { throw 'erased' }
                let itemPathString = item.path.toString();
                switch (itemPathString) {
                    case 'shutuptakemoney.jpg':
                        if (!(content.byteLength === 33398)) { throw 'content.byteLength === 33398' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'SouthPark.jpg':
                        if (!(content.byteLength === 40778)) { throw 'content.byteLength === 40778' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'zombies.jpg':
                        if (!(content.byteLength === 83127)) { throw 'content.byteLength === 83127' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    case 'Мюнхен.jpg':
                    case 'München.jpg':
                        if (!(content.byteLength === 8764)) { throw 'content.byteLength === 8764' }
                        console.log(`${t}. Check ${itemPathString} ok`);
                        break;
                    default:
                        throw '';
                }
            }
            
            decoder.abort();
            path.remove();
            
            for (const [item, stream] of memoryMap) {
                erased = stream.erase();
                if (!erased) { throw 'erased' }
            }
            for (const [item, stream] of fileMap) {
                erased = stream.erase();
                if (!erased) { throw 'erased' }
            }
        }
    } catch (error) {
        console.log(`${t}. Exception: ${error}`);
    }
    console.log(`${t}. testMultiVolume exit`);
}

testEncodeMultiVolume();

console.log('Global exit.');
