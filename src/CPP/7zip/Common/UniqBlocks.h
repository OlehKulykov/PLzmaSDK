// UniqBlocks.h

#ifndef __UNIQ_BLOCKS_H
#define __UNIQ_BLOCKS_H

#include "../../Common/MyTypes.h"
#include "../../Common/MyBuffer.h"
#include "../../Common/MyVector.h"

struct CUniqBlocks final
{
  CObjectVector<CByteBuffer> Bufs;
  CUIntVector Sorted;
  CUIntVector BufIndexToSortedIndex;

  unsigned AddUniq(const Byte *data, size_t size);
  UInt64 GetTotalSizeInBytes() const;
  void GetReverseMap();

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#endif

  bool IsOnlyEmpty() const
  {
    return (Bufs.Size() == 0 || Bufs.Size() == 1 && Bufs[0].Size() == 0);
  }

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
};

#endif
