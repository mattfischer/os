#include "Log.hpp"

#include <algorithm>
#include <string.h>
#include <stdarg.h>

#define LOG_BUFFER_SIZE 4096
char buffer[LOG_BUFFER_SIZE];
int writePointer = 0;
bool full = false;

static void printHex(unsigned int x, bool fill)
{
	char buffer[9];
	int n = 0;

	for(int i=0; i<8; i++) {
		int f = (x >> (28 - i * 4)) & 0xf;
		if(fill || f != 0) {
			if(f < 10) {
				buffer[n] = '0' + f;
			} else {
				buffer[n] = 'a' + f - 10;
			}
			n++;
		}
	}
	buffer[n] = '\0';

	Log::puts(buffer);
}

static void printInt(unsigned int x)
{
	char buffer[16];
	int pow;

	for(pow = 1; pow * 10 < x; pow *= 10) ;

	int n = 0;
	for(; pow >= 1; pow /= 10) {
		buffer[n] = '0' + x / pow;
		n++;
		x -= pow * (x / pow);
	}
	buffer[n] = '\0';

	Log::puts(buffer);
}

void Log::printf(const char *s, ...)
{
	const char *start = s;
	const char *end;
	va_list va;

	va_start(va, s);
	while(*start != '\0') {
		end = start;
		while(*end != '\0' && *end != '%') end++;
		write(start, end - start);
		start = end;

		if(*start == '%') {
			start++;
			switch(*start) {
				case 's':
				{
					const char *s2 = va_arg(va, const char *);
					puts(s2);
					break;
				}

				case 'i':
				{
					unsigned int x = va_arg(va, int);
					printInt(x);
					break;
				}

				case 'x':
				{
					unsigned int x = va_arg(va, int);
					printHex(x, false);
					break;
				}

				case 'p':
				{
					unsigned int x = va_arg(va, int);
					puts("0x");
					printHex(x, true);
					break;
				}
			}
			start++;
		}
	}

	va_end(va);
}

void Log::puts(const char *s)
{
	int size = strlen(s);
	write(s, size);
}

void Log::write(const char *s, int size)
{
	int len = std::min(size, LOG_BUFFER_SIZE - writePointer);
	memcpy(buffer + writePointer, s, len);
	writePointer += len;
	if(writePointer == LOG_BUFFER_SIZE) {
		writePointer = 0;
		full = true;
	}

	if(len < size) {
		memcpy(buffer, s + len, size - len);
		writePointer += size - len;
	}
}

int Log::read(int offset, const char **data)
{
	int len;
	int start;

	if(full) {
		start = writePointer;
		len = LOG_BUFFER_SIZE;
	} else {
		start = 0;
		len = writePointer;
	}

	start += offset;
	start %= LOG_BUFFER_SIZE;

	*data = buffer + start;
	return std::min(len - start, LOG_BUFFER_SIZE - start);
}
