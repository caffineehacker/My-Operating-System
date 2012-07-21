#pragma once

#define EDOM
#define EILSEQ
#define ERANGE

extern int* errno();

#define errno *errno()
