#include <cstdio>
#include <va_list.h>
#include <cstdarg>
#include <cstring>
#include "systemcall.h"

FILE* _GetStdErr()
{
	/* TODO: Make a system call to get StdErr */
	return NULL;
}

FILE* _GetStdIn()
{
	/* TODO: Make a system call to get StdIn */
	return NULL;
}

FILE* _GetStdOut()
{
	return (FILE*)ExecuteSysCall(SYSCALL_GET_STDOUT);
}

char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void itoa(unsigned i, unsigned base, char* buf)
{
	char tbuf[32];
	int pos = 0;
	int opos = 0;
	int top = 0;

	if (i == 0 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (i != 0) {
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top=pos--;
	for (opos=0; opos<top; pos--,opos++) {
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}

void itoa_s(int i, unsigned base, char* buf)
{
   if (base > 16)
	   return;
   
   if (i < 0)
   {
      *buf++ = '-';
      i *= -1;
   }

   itoa((unsigned int)i, base, buf);
}

int vfprintf(FILE* stream, const char* format, va_list args)
{
	size_t i;
	for (i=0; i<strlen(format);i++)
	{
		switch (format[i])
		{
			case '%':
				switch (format[i+1]) {
					/*** characters ***/
					case 'c':
					{
						char c = va_arg(args, char);
						fputc(c, stream);
						i++;		// go to next character
						break;
					}

					/*** address of ***/
					case 's':
					{
						int c = (int&)va_arg(args, char);
						char str[64];
						strcpy(str,(const char*)c);
						fputs(str, stream);
						i++;		// go to next character
						break;
					}

					/*** integers ***/
					case 'd':
					case 'i':
					{
						int c = va_arg (args, int);
						char str[32]={0};
						itoa_s(c, 10, str);
						fputs(str, stream);
						i++;		// go to next character
						break;
					}

					/*** display in hex ***/
					case 'X':
					case 'x':
					{
						int c = va_arg(args, int);
						char str[32]={0};
						itoa_s(c,16,str);
						fputs(str, stream);
						i++;		// go to next character
						break;
					}

					default:
						va_end(args);
						return 1;
				}

				break;

			default:
				fputc(format[i], stream);
				break;
		}

	}

	va_end(args);
	return (int)i;
}

int fprintf(FILE* stream, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	int retVal = vfprintf(stream, format, args);

	va_end(args);
	return retVal;
}

int printf(const char* format, ...)
{
	int retVal = 0;
	va_list args;
	va_start(args, format);

	retVal = vfprintf(stdout, format, args);

	va_end(args);

	return retVal;
}

int fputs(char* str, FILE* stream)
{
	while (0 != *str)
	{
		fputc(*str, stream);
		str++;
	}

	return 0;
}

int fputc(char c, FILE* stream)
{
	if (stream->bufferPos >= stream->bufferLength)
		stream->flushFunc();
	stream->buffer[stream->bufferPos++] = c;

	return 0;
}

int fflush(FILE* stream)
{
	stream->flushFunc();

	return 0;
}
