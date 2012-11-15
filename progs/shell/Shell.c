#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char cwd[PATH_MAX];
char dir[PATH_MAX];

void resolve(char *path)
{
	char *c = path;
	while(c[0] != '\0') {
		while(c[1] == '/') {
			strcpy(c, c + 1);
		}
		while(strncmp(c, "/..", 3) == 0) {
			char *c2 = c - 1;
			while(*c2 != '/') c2--;
			if(c[3] == '\0') {
				c2[1] = '\0';
			} else {
				strcpy(c2 + 1, c + 4);
			}
			c = c2;
		}
		c = strchr(c + 1, '/');
	}
}

void ls(const char *cmd)
{
	strcpy(dir, cwd);

	cmd += 2;
	if(cmd[0] != '\0') {
		const char *d = cmd + 1;
		if(d[0] == '/') {
			strcpy(dir, d);
		} else {
			strcat(dir, "/");
			strcat(dir, d);
			resolve(dir);
		}
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

void cd(const char *cmd)
{
	const char *d = cmd + 3;
	struct stat st;

	if(d[0] == '/') {
		strcpy(dir, d);
	} else {
		strcpy(dir, cwd);
		strcat(dir, "/");
		strcat(dir, d);
		resolve(dir);
	}

	if(stat(dir, &st) != 0) {
		printf("Directory '%s' does not exist\n", dir);
		return;
	}

	if(!S_ISDIR(st.st_mode)) {
		printf("'%s' is not a directory\n", dir);
		return;
	}

	strcpy(cwd, dir);
}

void runProgram(const char *cmd)
{
	struct stat st;
	const char *argv[2];

	sprintf(dir, "%s/%s", cwd, cmd);
	if(stat(dir, &st) != 0 || !S_ISREG(st.st_mode)) {
		printf("%s is not an executable program\n", cmd);
		return;
	}

	argv[0] = dir;
	argv[1] = NULL;
	int child = SpawnProcess(argv, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
	WaitProcess(child);
}

void processCommand(const char *cmd)
{
	if(strncmp(cmd, "ls", 2) == 0) {
		ls(cmd);
	} else if(strncmp(cmd, "cd", 2) == 0) {
		cd(cmd);
	} else if(strncmp(cmd, "pwd", 3) == 0) {
		printf("%s\n", cwd);
	} else {
		runProgram(cmd);
	}
}

int main(int argc, char *argv[])
{
	char buffer[1024];

	strcpy(cwd, "/");

	while(1) {
		printf("# ");
		fflush(stdout);
		gets(buffer);
		printf("\n");
		processCommand(buffer);
	}

	return 0;
}