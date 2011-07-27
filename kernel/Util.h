#ifndef UTIL_H
#define UTIL_H

#include "Defs.h"

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);

int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);

#endif