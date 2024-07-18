// BcjRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BcjCoder.h"

namespace NCompress {
namespace NBcj {

REGISTER_FILTER_E(BCJ,
    CCoder2(z7_BranchConvSt_X86_Dec),
    CCoder2(z7_BranchConvSt_X86_Enc),
    0x3030103, "BCJ")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_6(void) {
    return static_cast<uint64_t>(NCompress::NBcj::g_CodecInfo_BCJ.Id);
}
#endif
