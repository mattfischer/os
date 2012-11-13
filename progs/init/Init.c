#include <System.h>
#include <Object.h>

extern int __NameServer;
int main(int argc, char *argv[])
{
	int console;
	int child;

	child = SpawnProcess("/boot/name", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Object_Release(child);

	child = SpawnProcess("/boot/console", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Object_Release(child);

	Name_Wait("/dev/console");
	console = open("/dev/console");
	child = SpawnProcess("/boot/shell", console, console, console);
	Object_Release(child);

	while(1) {
		Yield();
	}
}