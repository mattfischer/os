#include <System.h>
#include <Object.h>

#include <stdlib.h>

extern int __NameServer;
int main(int argc, char *argv[])
{
	int console;
	int child;
	const char *childArgv[4];

	childArgv[0] = "/boot/name";
	childArgv[1] = NULL;
	child = SpawnProcess(childArgv, OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Object_Release(child);

	childArgv[0] = "/boot/uart-pl011";
	childArgv[1] = "/dev/console";
	childArgv[2] = "0x16000000";
	childArgv[3] = NULL;
	child = SpawnProcess(childArgv, OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Object_Release(child);

	Name_Wait("/dev/console");
	console = open("/dev/console");

	childArgv[0] = "/boot/shell";
	childArgv[1] = NULL;
	child = SpawnProcess(childArgv, console, console, console);
	Object_Release(child);

	while(1) {
		Yield();
	}
}