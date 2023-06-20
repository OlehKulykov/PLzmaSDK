//
// By using this Software, you are accepting original [LZMA SDK] and MIT license below:
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2023 Oleh Kulykov <olehkulykov@gmail.com>
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


#include <exception>

#import "PLzmaSDKGlobal.h"
#import "PLzmaSDKGlobal.inl"

NSExceptionName const PLzmaSDKGenericExceptionName = @"PLzmaSDKGenericException";
NSErrorUserInfoKey const PLzmaSDKExceptionCodeKey = @"code";
NSErrorUserInfoKey const PLzmaSDKExceptionFileKey = @"file";
NSErrorUserInfoKey const PLzmaSDKExceptionLineKey = @"line";

NSException * PLzmaSDKPLExceptionToNS(const plzma::Exception & exception) {
    const char * str = exception.reason();
    NSString * reason = str ? [NSString stringWithUTF8String:str] : nil;
    NSMutableDictionary * userInfo = [NSMutableDictionary dictionaryWithCapacity:4];
    if ( (str = exception.what()) ) {
        userInfo[NSLocalizedDescriptionKey] = [NSString stringWithUTF8String:str];
    }
    if ( (str = exception.file()) ) {
        userInfo[PLzmaSDKExceptionFileKey] = [NSString stringWithUTF8String:str];
    }
    [userInfo setObject:[NSNumber numberWithInt:exception.line()] forKey:PLzmaSDKExceptionLineKey];
    [userInfo setObject:[NSNumber numberWithInt:exception.code()] forKey:PLzmaSDKExceptionCodeKey];
    
    return [[NSException alloc] initWithName:PLzmaSDKGenericExceptionName reason:reason userInfo:userInfo];
}

NSException * PLzmaSDKSTDExceptionToNS(const std::exception & exception) {
    const char * what = exception.what();
    NSString * reason = what ? [NSString stringWithUTF8String:what] : nil;
    return [[NSException alloc] initWithName:PLzmaSDKGenericExceptionName reason:reason userInfo:nil];
}

NSException * PLzmaSDKExceptionToNS(void) {
    return [[NSException alloc] initWithName:PLzmaSDKGenericExceptionName reason:nil userInfo:nil];
}

NSException * PLzmaSDKExceptionCreate(const PLzmaSDKErrorCode errorCode, NSString * what, NSString * reason, const char * file, const int line) {
    NSMutableDictionary * userInfo = [NSMutableDictionary dictionaryWithCapacity:4];
    if (what) {
        [userInfo setObject:what forKey:NSLocalizedDescriptionKey];
    }
    NSString * nsFile = file ? [[NSString stringWithUTF8String:file] lastPathComponent] : nil;
    if (nsFile) {
        [userInfo setObject:nsFile forKey:PLzmaSDKExceptionFileKey];
    }
    [userInfo setObject:[NSNumber numberWithInt:line] forKey:PLzmaSDKExceptionLineKey];
    [userInfo setObject:[NSNumber numberWithInt:errorCode] forKey:PLzmaSDKExceptionCodeKey];
    
    return [[NSException alloc] initWithName:PLzmaSDKGenericExceptionName reason:reason userInfo:userInfo];
}

NSString * const PLzmaSDKVersion(void) {
    return [NSString stringWithUTF8String:plzma_version()];
}

PLzmaSDKSize PLzmaSDKGetStreamReadSize(void) {
    return plzma_stream_read_size();
}

void PLzmaSDKSetStreamReadSize(const PLzmaSDKSize size) {
    plzma_set_stream_read_size(size);
}

PLzmaSDKSize PLzmaSDKGetStreamWriteSize(void) {
    return plzma_stream_write_size();
}

void PLzmaSDKSetStreamWriteSize(const PLzmaSDKSize size) {
    plzma_set_stream_write_size(size);
}

PLzmaSDKSize PLzmaSDKGetDecoderReadSize(void) {
    return plzma_decoder_read_size();
}

void PLzmaSDKSetDecoderReadSize(const PLzmaSDKSize size) {
    plzma_set_decoder_read_size(size);
}

PLzmaSDKSize PLzmaSDKGetDecoderWriteSize(void) {
    return plzma_decoder_write_size();
}

void PLzmaSDKSetDecoderWriteSize(const PLzmaSDKSize size) {
    plzma_set_decoder_write_size(size);
}
