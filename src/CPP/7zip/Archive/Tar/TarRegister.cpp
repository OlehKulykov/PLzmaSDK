// TarRegister.cpp

#include "StdAfx.h"

#if !defined(LIBPLZMA_NO_TAR)

#include "../../Common/RegisterArc.h"

#include "TarHandler.h"

namespace NArchive {
namespace NTar {

static const Byte k_Signature[] = { 'u', 's', 't', 'a', 'r' };

REGISTER_ARC_IO(
  "tar", "tar ova", 0, 0xEE,
  k_Signature,
  NFileHeader::kUstarMagic_Offset,
  NArcInfoFlags::kStartOpen |
  NArcInfoFlags::kSymLinks |
  NArcInfoFlags::kHardLinks,
  IsArc_Tar)
 
}}

#if defined(LIBPLZMA_USING_REGISTRATORS)
uint64_t plzma_registrator_17(void) {
    return NArchive::NTar::g_ArcInfo.Flags;
}
#endif

#endif // !LIBPLZMA_NO_TAR
