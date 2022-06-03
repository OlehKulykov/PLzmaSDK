//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2022 Oleh Kulykov <olehkulykov@gmail.com>
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


import Foundation
#if SWIFT_PACKAGE
import libplzma
#endif


/// The full version string of the library generated on build time.
///
/// Contains version<major, minor, patch> with optional automatic build number,
/// library type, build date/time, os, compiler, environment, usage, features, etc. and original LZMA SDK version.
public var version: String {
    return String(utf8CString: plzma_version())
}


/// Receives or changes the current size in bytes of the stream's read block per single read request.
public var streamReadSize: Size {
    get {
        return plzma_stream_read_size()
    }
    set {
        plzma_set_stream_read_size(newValue)
    }
}


/// Receives or changes the current size in bytes of the stream's write block per single write request.
public var streamWriteSize: Size {
    get {
        return plzma_stream_write_size()
    }
    set {
        plzma_set_stream_write_size(newValue)
    }
}


/// Receives or changes the current size in bytes of the decoder's internal buffer for holding decoded data.
public var decoderReadSize: Size {
    get {
        return plzma_decoder_read_size()
    }
    set {
        plzma_set_decoder_read_size(newValue)
    }
}


/// Receives or changes the current size in bytes of the decoder's internal buffer for holding decoded data.
public var decoderWriteSize: Size {
    get {
        return plzma_decoder_write_size()
    }
    set {
        plzma_set_decoder_write_size(newValue)
    }
}
