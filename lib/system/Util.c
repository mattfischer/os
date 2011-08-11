#include <lib/system/Util.h>

char *strcpy(char *dest, const char *src)
{
	int i;

	for(i=0; src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	return dest;
}

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
