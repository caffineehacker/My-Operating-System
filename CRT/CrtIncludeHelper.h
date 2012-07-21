#pragma once

#ifdef  __cplusplus

#define ExternCStart extern "C" {
#define ExternCEnd }

#else

#define ExternCStart
#define ExternCEnd

#endif
