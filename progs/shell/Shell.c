#include <stdio.h>

void ls(const char *cmd)
{
	const char *dir = "/";
	cmd += 2;
	if(cmd[0] != '\0') {
		dir = cmd + 1;
	}

	char buffer[32];
	int obj = Name_OpenDir(dir);
	int status = File_ReadDir(obj, buffer);
	while(status == 0) {
		printf("%s\n", buffer);
		status = File_ReadDir(obj, buffer);
	}
	Object_Release(obj);
}

void processCommand(const char *cmd)
{
	if(strncmp(cmd, "ls", 2) == 0) {
		ls(cmd);
	} else {
		printf("Invalid command '%s'\n", cmd);
	}
}

int main(int argc, char *argv[])
{
	char buffer[1024];

	while(1) {
		printf("# ");
		fflush(stdout);
		gets(buffer);
		printf("\n");
		processCommand(buffer);
	}

	return 0;
}