#include "Util.hpp"

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
	int i;
	for(i=0; src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	dest[i] = '\0';

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

int strncmp(const char *str1, const char *str2, size_t size)
{
	for(int i=0; i<size; i++) {
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

	return 0;
}

int strlen(const char *str)
{
	int i;
	for(i=0; str[i] != '\0'; i++) {}

	return i;
}

extern "C" {
	void __cxa_pure_virtual() {}
}