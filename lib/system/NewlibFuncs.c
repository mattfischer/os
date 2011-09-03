#include <Object.h>
#include <IO.h>

#include <kernel/include/ProcManagerFmt.h>

#include <sys/stat.h>
#include <stddef.h>

extern int __ProcessManager;

void *_sbrk(int inc)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerSbrk;
	msg.msg.u.sbrk.increment = inc;
	return (void*)Object_Send(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}

int _write(int fd, char *buffer, int len)
{
	return File_Write(fd, buffer, len);
}

int _read(int fd, char *buffer, int len)
{
	return 0;
}

int _close(int fd)
{
	return -1;
}

int _isatty(int fd)
{
	return 0;
}

int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}