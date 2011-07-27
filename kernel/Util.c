#include "Util.h"

void *memcpy(void *dest, const void *src, unsigned n)
{
	int i;
	char *d = dest;
	const char *s = src;

	for(i=0; i<n; i++) {
		d[i] = s[i];
	}

	return dest;
}

void *memset(void *dest, int c, unsigned int n)
{
	int i;
	char *d = dest;

	for(i=0; i<n; i++) {
		d[i] = c;
	}

	return dest;
}

char *strcpy(char *dest, const char *src)
{
	int i;

	for(i=0; src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	return dest;
}

int strcmp(const char *str1, const char *str2)
{
	int i;

	for(i=0; ; i++) {
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