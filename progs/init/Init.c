#include <System.h>
#include <Object.h>

extern int __NameServer;
int main(int argc, char *argv[])
{
	int console;

	SpawnProcess("/boot/name", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	SpawnProcess("/boot/console", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);

	Name_Wait("/dev/console");
	console = open("/dev/console");
	SpawnProcess("/boot/shell", console, console, console);

	while(1) {
		Yield();
	}
}