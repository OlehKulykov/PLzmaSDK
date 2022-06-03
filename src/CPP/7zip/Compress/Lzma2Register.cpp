// Lzma2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "Lzma2Decoder.h"

#ifndef EXTRACT_ONLY
#include "Lzma2Encoder.h"
#endif

namespace NCompress {
namespace NLzma2 {

REGISTER_CODEC_E(LZMA2,
    CDecoder(),
    CEncoder(),
    0x21,
    "LZMA2")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_12(void) {
    return static_cast<uint64_t>(NCompress::NLzma2::g_CodecInfo_LZMA2.Id);
}
#endif
