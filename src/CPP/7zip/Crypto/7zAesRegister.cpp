// 7zAesRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "7zAes.h"

namespace NCrypto {
namespace N7z {

REGISTER_FILTER_E(_7zAES,
    CDecoder,
    CEncoder,
    0x6F10701, "7zAES")

}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_14(void) {
    return static_cast<uint64_t>(NCrypto::N7z::g_CodecInfo__7zAES.Id);
}
#endif
