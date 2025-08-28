#pragma once

#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

THIRD_PARTY_INCLUDES_START
#include <string>
#include <sstream>

#ifdef _WIN64
#define WIN32_LEAN_AND_MEAN
#include "Windows/AllowWindowsPlatformTypes.h"
#include <memoryapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

THIRD_PARTY_INCLUDES_END