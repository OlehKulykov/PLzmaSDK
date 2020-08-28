
'use strict';

var plzma = require('./../build/Release/plzma');

console.log('plzma export:', plzma);
console.log('typeof plzma.ErrorCode.io:', typeof plzma.ErrorCode.io);
let test = 1;

let path, path1;
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
    decoder.setProgressDelegate((path, progress) => console.log(`testing: ${path} progress: ${progress}`) );
    const openRes = await decoder.openAsync();
    console.log(`${t}. openRes: ${openRes}`);
    const testRes = await decoder.testAsync();
    console.log(`${t}. testRes: ${testRes}`);
    decoder.setProgressDelegate((path, progress) => console.log(`testing2: ${path} progress: ${progress}`) );
    console.log(`${t}. trying test 2`);
    const testRes2 = await decoder.testAsync([]);
    console.log(`${t}. testRes2: ${testRes2}`);

    const items = [];
    for (let i = 0, n = decoder.count; i < n; i++) { 
        items.push(decoder.itemAt(i)); 
    }
    decoder.setProgressDelegate((path, progress) => console.log(`testing3: ${path} progress: ${progress}`) );
    const testRes3 = await decoder.testAsync(items);
    console.log(`${t}. testRes3: ${testRes3}`);
    console.log(`${t}. decoderAsyncTest1 exit`);
    
    const itemsMap = new Map();
    for (let i = 0, n = decoder.count; i < n; i++) { 
        itemsMap.set(decoder.itemAt(i), plzma.OutStream()); 
    }
    console.log(`${t}. itemsMap: ${itemsMap}`);
    decoder.setProgressDelegate((path, progress) => console.log(`extracting1: ${path} progress: ${progress}`) );
    const extractRes1 = await decoder.extractAsync(itemsMap);
    console.log(`${t}. extractRes1: ${extractRes1}`);
}

decoderAsyncTest1();

test++; // encoder.open
async function encoderAsyncTest1() {
    const t = test;
    console.log(`${t}. encoderAsyncTest1 enter`);
    const encoder = plzma.Encoder(plzma.OutStream(), plzma.FileType.sevenZ, plzma.Method.LZMA2);
    encoder.setProgressDelegate((path, progress) => console.log(`compressing: ${path} progress: ${progress}`) );
}

encoderAsyncTest1();

// let decoder = plzma.Decoder(plzma.InStream('/tmp/7z1900-src.7z'), plzma.FileType.sevenZ);
// if (decoder.open() !== true) throw '';
// decoder = plzma.Decoder(plzma.InStream('/tmp/7z1900-src.7z'), plzma.FileType.sevenZ);
// decoder.openAsync()
// .then(() => {
//     console.log(`${test}. resolve: decoder.openAsync`);
// })
// .catch((error) => {
//     console.log(`${test}. catch: decoder.openAsync error: ${error}`);
// });

// if (decoder.count <= 0) throw '';
// 
// // console.log(`${test}. decoder.items: ${decoder.items}`);
// console.log(`${test}. decoder.itemAt: ${decoder.itemAt(1)}`);
// decoder.setProgressDelegate((itemPath, progress) => {
//     console.log(`path: ${itemPath} progress: ${progress}`);
// });
// let res;
// res = decoder.test();
// console.log(`${test}.1 res: ${res}`);
// let wait = { value: true };
// res = decoder.testAsync(null, (ar) => {
//     console.log(`async result: ${ar}`);
//     wait.value = false;
// });
// console.log(`${test}.1 res: ${res}`);

// let counter = 1;
// while (wait.value) {
//     counter++;
//     if (counter === 999999) {
//         counter = 1;
//     }
// }

iterator = plzma.Path.tmpPath.openDir(plzma.OpenDirMode.followSymlinks);
while (iterator.next()) {
    iterator.fullPath.remove();
}
iterator.close();

console.log('exit.');
