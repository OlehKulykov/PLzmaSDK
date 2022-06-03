// Common/CRC.cpp

#include "StdAfx.h"

#include "../../C/7zCrc.h"

#if !defined(LIBPLZMA_USING_REGISTRATORS)
static struct CCRCTableInit { CCRCTableInit() { CrcGenerateTable(); } } g_CRCTableInit;
#endif
