
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
