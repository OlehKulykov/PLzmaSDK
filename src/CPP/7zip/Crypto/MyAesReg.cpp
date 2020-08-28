// MyAesReg.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "MyAes.h"

namespace NCrypto {

REGISTER_FILTER_E(AES256CBC,
    CAesCbcDecoder(32),
    CAesCbcEncoder(32),
    0x6F00181, "AES256CBC")

}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_15(void) {
    return static_cast<uint64_t>(NCrypto::g_CodecInfo.Id);
}
#endif
