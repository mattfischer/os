#include <stdio.h>

int main(int argc, char *argv[])
{
	char buffer[1024];

	while(1) {
		printf("# ");
		fflush(stdout);
		gets(buffer);
		printf("\n%s\n", buffer);
	}

	return 0;
}