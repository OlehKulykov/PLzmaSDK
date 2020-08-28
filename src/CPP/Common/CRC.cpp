// Common/CRC.cpp

#include "StdAfx.h"

#include "../../C/7zCrc.h"

#if !defined(LIBPLZMA_USING_REGISTRATORS)
struct CCRCTableInit final { CCRCTableInit() { CrcGenerateTable(); } } g_CRCTableInit;
#endif
