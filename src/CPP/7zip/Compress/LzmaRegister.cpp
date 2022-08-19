// LzmaRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "LzmaDecoder.h"

#ifndef EXTRACT_ONLY
#include "LzmaEncoder.h"
#endif

namespace NCompress {
namespace NLzma {

REGISTER_CODEC_E(LZMA,
    CDecoder(),
    CEncoder(),
    0x30101,
    "LZMA")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_10(void) {
    return static_cast<uint64_t>(NCompress::NLzma::g_CodecInfo_LZMA.Id);
}
#endif
