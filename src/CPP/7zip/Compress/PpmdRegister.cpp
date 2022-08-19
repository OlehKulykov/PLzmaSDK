// PpmdRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "PpmdDecoder.h"

#ifndef EXTRACT_ONLY
#include "PpmdEncoder.h"
#endif

namespace NCompress {
namespace NPpmd {

REGISTER_CODEC_E(PPMD,
    CDecoder(),
    CEncoder(),
    0x30401,
    "PPMD")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_13(void) {
    return static_cast<uint64_t>(NCompress::NPpmd::g_CodecInfo_PPMD.Id);
}
#endif
