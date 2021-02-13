#pragma once
#include <common/Logger.h>

#if defined(_WINDOWS) && defined(core_EXPORTS)
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif
