#include <cstring>
#include <cstdint>
#include <_null.h>

/* Sets a block of memory to the specified value */
void *memset(void *dest, char val, size_t count)
{
    unsigned char *temp = (unsigned char *)dest;
	for(; count != 0; count--, temp[count] = (unsigned char)val);
	return dest;
}

void *memcpy(void* dest, const void* source, size_t count)
{
	uint8_t* tmpDest = (uint8_t*)dest;
	uint8_t* tmpSource = (uint8_t*)source;
	
	/* Copying by 32-bit chunks is better performance */
	for (; count >= 4; count -= 4, tmpDest += 4, tmpSource += 4)
	{
		*(uint32_t*)tmpDest = *(uint32_t*)tmpSource;
	}

	for (; count > 0; count--, tmpDest++, tmpSource++)
	{
		*tmpDest = *tmpSource;
	}

	return dest;
}

size_t strlen(const char *s)
{
	size_t retVal = 0;

	for(; *s != '\0'; s++)
		retVal++;

	return retVal;
}

char *strcpy(char* s1, const char* s2)
{
	char *s1_p = s1;
    while (*s1++ = *s2++);
    return s1_p;
}

int strcmp(const char *s1, const char *s2)
{
	/* TODO: Comparing 32-bit chunks as integers would be faster */
	char* tmpS1 = (char*)s1;
	char* tmpS2 = (char*)s2;
	for (; *tmpS1 != NULL; tmpS1++, tmpS2++)
	{
		if (*tmpS1 != *tmpS2)
			return *tmpS1 - *tmpS2;
	}

	return 0;
}

char *strchr(const char *str, int c)
{
	char* tmpStr = (char *)str;

	while (*tmpStr != '\0')
		if (*tmpStr == (char)c)
			return tmpStr;
		else
			tmpStr++;

	return NULL;
}
