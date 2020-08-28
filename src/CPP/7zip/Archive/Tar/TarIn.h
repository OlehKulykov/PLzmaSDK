// TarIn.h

#ifndef __ARCHIVE_TAR_IN_H
#define __ARCHIVE_TAR_IN_H

#include "../../../../C/7zTypes.h"
#include "../../IStream.h"

#include "TarItem.h"

namespace NArchive {
namespace NTar {
  
enum EErrorType
{
  k_ErrorType_OK,
  k_ErrorType_Corrupted,
  k_ErrorType_UnexpectedEnd,
  k_ErrorType_Warning
};

EXTERN_C_BEGIN

HRESULT ReadItem(ISequentialInStream *stream, bool &filled, CItemEx &itemInfo, EErrorType &error);
UInt32 WINAPI IsArc_Tar(const Byte *p, size_t size);

EXTERN_C_END

}}
  
#endif
