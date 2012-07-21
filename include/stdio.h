#pragma once

#include <_null.h>
#include <cstdint>

#define fpos_t long

enum FILE_FLAGS
{
	FS_FILE = 0x01,
	FS_DIRECTORY = 0x02,
	FS_INVALID = 0x04,
	FILE_FLAG_EOF = 0x08
};

typedef struct __FILE
{
	char *buffer;
	long bufferStart;
	long bufferLength;
	long bufferPos;
	fpos_t bufferStartFilePos;
	uint8_t flags;
	uint32_t deviceID;
	void (_cdecl *flushFunc)(void);
	void* tag;
} FILE, *PFILE;

#define _IOFBF -1
#define _IOLBF -2
#define _IONBF -3
#define BUFSIZ 4096

#define EOF -4

#define FOPEN_MAX 256 /* I only allow 256 files open by a single process at once */
#define FILENAME_MAX 256 /* 256 chars in a filename */
#define L_tmpnam 256 /* 256 chars in a temp filename */

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

#define TMP_MAX 256 /* Only 256 possible temp filenames */

#define stderr _GetStdErr()
#define stdin _GetStdIn()
#define stdout _GetStdOut()

extern FILE* _GetStdErr();
extern FILE* _GetStdIn();
extern FILE* _GetStdOut();

extern int fprintf(FILE* stream, const char* format, ...);
extern int printf(const char* format, ...);

extern int fputc(char c, FILE* stream);
extern int fputs(char* s, FILE* stream);

extern void itoa(unsigned i, unsigned base, char* buf);
extern void itoa_s(int i, unsigned base, char* buf);
