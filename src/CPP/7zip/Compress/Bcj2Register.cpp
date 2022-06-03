// Bcj2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "Bcj2Coder.h"

namespace NCompress {
namespace NBcj2 {

REGISTER_CODEC_CREATE_2(CreateCodec, CDecoder(), ICompressCoder2)
#ifndef EXTRACT_ONLY
REGISTER_CODEC_CREATE_2(CreateCodecOut, CEncoder(), ICompressCoder2)
#else
#define CreateCodecOut NULL
#endif

REGISTER_CODEC_VAR(BCJ2)
  { CreateCodec, CreateCodecOut, 0x303011B, "BCJ2", 4, false };

REGISTER_CODEC(BCJ2)

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_5(void) {
    return static_cast<uint64_t>(NCompress::NBcj2::g_CodecInfo_BCJ2.Id);
}
#endif
