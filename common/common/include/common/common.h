#pragma once
//#define DISABLE_LOGGING
#if defined(_WINDOWS) && defined(common_EXPORTS)
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API
#endif

