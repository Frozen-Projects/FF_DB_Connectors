#pragma once

THIRD_PARTY_INCLUDES_START

#ifdef _WIN64
#define WIN32_LEAN_AND_MEAN
#include "Windows/AllowWindowsPlatformTypes.h"
#include <sqlext.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include <string>
#include <sstream>
THIRD_PARTY_INCLUDES_END