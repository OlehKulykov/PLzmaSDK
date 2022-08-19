// CopyRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "CopyCoder.h"

namespace NCompress {

REGISTER_CODEC_CREATE(CreateCodec, CCopyCoder())

REGISTER_CODEC_2(Copy, CreateCodec, CreateCodec, 0, "Copy")

}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_9(void) {
    return static_cast<uint64_t>(NCompress::g_CodecInfo_Copy.Id);
}
#endif
