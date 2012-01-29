#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

extern "C" {
	void *memcpy(void *dest, const void *src, size_t n);
	void *memset(void *dest, int c, size_t n);

	int strcmp(const char *str1, const char *str2);
	int strncmp(const char *str1, const char *str2, size_t num);
	char *strcpy(char *dest, const char *src);
	int strlen(const char *str);
}

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#endif