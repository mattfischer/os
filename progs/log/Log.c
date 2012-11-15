#include <stdio.h>

#define BUFFER_SIZE 128

void main()
{
	char buffer[BUFFER_SIZE];
	FILE *file;
	int size;

	file = fopen("/dev/log", "r");
	size = fread(buffer, 1, BUFFER_SIZE - 1, file);
	while(size > 0) {
		buffer[size] = '\0';
		printf(buffer);
		size = fread(buffer, 1, BUFFER_SIZE - 1, file);
	}
}