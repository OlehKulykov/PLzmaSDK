// MyAesReg.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "MyAes.h"

namespace NCrypto {

#ifndef _SFX

#define REGISTER_AES_2(name, nameString, keySize, isCtr) \
  REGISTER_FILTER_E(name, \
    CAesCoder(false, keySize, isCtr), \
    CAesCoder(true , keySize, isCtr), \
    0x6F00100 | ((keySize - 16) * 8) | (isCtr ? 4 : 1), \
    nameString) \

#define REGISTER_AES(name, nameString, isCtr) \
  /* REGISTER_AES_2(AES128 ## name, "AES128" nameString, 16, isCtr) */ \
  /* REGISTER_AES_2(AES192 ## name, "AES192" nameString, 24, isCtr) */ \
  REGISTER_AES_2(AES256 ## name, "AES256" nameString, 32, isCtr) \

REGISTER_AES(CBC, "CBC", false)
// REGISTER_AES(CTR, "CTR", true)

#endif

}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_15(void) {
#ifdef _SFX
    return 1;
#else
    return static_cast<uint64_t>(NCrypto::g_CodecInfo_AES256CBC.Id);
#endif
}
#endif
