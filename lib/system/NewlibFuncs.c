#include "include/Message.h"
#include "include/IO.h"

#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <sys/stat.h>
#include <stddef.h>

void *_sbrk(int inc)
{
	struct ProcManagerMsg msg;

	msg.type = ProcManagerSbrk;
	msg.u.sbrk.increment = inc;
	return (void*)SendMessage(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}

int _write(int fd, char *buffer, int len)
{
	return Write(fd, buffer, len);
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