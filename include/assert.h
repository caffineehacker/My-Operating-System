#include "CrtIncludeHelper.h"

#ifdef assert
#undef assert
#endif

#ifndef NDEBUG

ExternCStart

extern void _crtAssert(void *x, char* file, char* line);

ExternCEnd

#define assert(x) x ? void : _crtAssert(x, __FILE__, __LINE__)

#else /* NDEBUG defined */

#define assert(ignore) ((void)0)

#endif
