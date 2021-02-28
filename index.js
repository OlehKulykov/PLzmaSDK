'use strict';

const path = require('path');
const dirs = [
    'build/Release',
    'build/Debug',
    'build',
    'out/Release',
    'out/Debug',
    'Release',
    'Debug',
    'build/default',
    'bin/winx64',
    'bin/winx86'
];
for (let dir of dirs) {
    try {
        module.exports = require(path.resolve(__dirname, dir, 'plzmasdk.node'));
        break;
    } catch { }
}
if (!module.exports) {
    throw `Can't locate native 'plzmasdk.node' module in directories: ${dirs}`;
}

// See 'NODEJSMODULE.md' for module API reference.

