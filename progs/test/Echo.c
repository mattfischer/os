#include <stdio.h>

int main(int argc, char *argv[])
{
	while(1) {
		char c = getchar();
		putchar(c);
		fflush(stdout);
	}
	return 0;
}