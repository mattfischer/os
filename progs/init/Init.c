#include <System.h>
#include <Object.h>

extern int __NameServer;
int main(int argc, char *argv[])
{
	int console;

	SpawnProcess("name", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Yield();

	SpawnProcess("console", OBJECT_INVALID, OBJECT_INVALID, OBJECT_INVALID);
	Yield();
	Yield();

	console = open("console");
	SpawnProcess("clientA", console, console, console);
	SpawnProcess("clientB", console, console, console);

	while(1) {
		Yield();
	}
}