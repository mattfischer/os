#include <stdio.h>
#include <sys/stat.h>

void ls(const char *cmd)
{
	const char *dir = "/";
	cmd += 2;
	if(cmd[0] != '\0') {
		dir = cmd + 1;
	}

	struct stat st;
	if(stat(dir, &st) != 0) {
		printf("Directory '%s' does not exist\n", dir);
		return;
	}

	if(!S_ISDIR(st.st_mode)) {
		printf("'%s' is not a directory\n", dir);
		return;
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