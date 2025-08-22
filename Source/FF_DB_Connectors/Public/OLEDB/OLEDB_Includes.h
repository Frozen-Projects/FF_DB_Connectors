#pragma once

THIRD_PARTY_INCLUDES_START

#ifdef _WIN64

#define WIN32_LEAN_AND_MEAN
#include "Windows/AllowWindowsPlatformTypes.h"

#pragma push_macro("ITransaction")
#define ITransaction OLEDB_ITransaction

#include <transact.h>
#include <oledb.h>
#include <oledberr.h>
#include <msdasc.h>

#pragma pop_macro("ITransaction")

#include "Windows/HideWindowsPlatformTypes.h"

#endif

THIRD_PARTY_INCLUDES_END