#include "Util.h"

void *memcpy(void *dest, const void *src, unsigned n)
{
	char *d = (char*)dest;
	const char *s = (const char*)src;

	for(int i=0; i<n; i++) {
		d[i] = s[i];
	}

	return dest;
}

void *memset(void *dest, int c, unsigned int n)
{
	char *d = (char*)dest;

	for(int i=0; i<n; i++) {
		d[i] = c;
	}

	return dest;
}

char *strcpy(char *dest, const char *src)
{
	for(int i=0; src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	return dest;
}

int strcmp(const char *str1, const char *str2)
{
	for(int i=0; ; i++) {
		if(str1[i] == str2[i]) {
			if(str1[i] == 0) {
				return 0;
			} else {
				continue;
			}
		}

		if(str1[i] > str2[i]) {
			return -1;
		} else {
			return 1;
		}
	}
}