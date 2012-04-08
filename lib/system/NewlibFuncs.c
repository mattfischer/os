#include <Object.h>
#include <IO.h>
#include <Name.h>

#include <kernel/include/ProcManagerFmt.h>

#include <sys/stat.h>
#include <stddef.h>

extern int __ProcessManager;

int _open(const char *name, int flags, int mode)
{
	return Name_Open(name);
}

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
	return File_Read(fd, buffer, len);
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

void _exit(int code)
{
	while(1) {}
}

void _kill()
{
}

void _getpid()
{
}

int _stat(char *file, struct stat *st)
{
	int obj;

	obj = Name_Open(file);
	if(obj != OBJECT_INVALID) {
		Object_Release(obj);
		st->st_mode = S_IFREG;
		return 0;
	}

	obj = Name_OpenDir(file);
	if(obj != OBJECT_INVALID) {
		Object_Release(obj);
		st->st_mode = S_IFDIR;
		return 0;
	}

	return -1;
}

int __dso_handle;
