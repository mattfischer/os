#include <System.h>
#include <Object.h>

extern int __NameServer;
int main(int argc, char *argv[])
{
	int console;

	SpawnProcess("name", INVALID_OBJECT, INVALID_OBJECT, INVALID_OBJECT);
	Yield();

	SpawnProcess("console", INVALID_OBJECT, INVALID_OBJECT, INVALID_OBJECT);
	Yield();
	Yield();

	console = Name_Lookup("console");
	SpawnProcess("clientA", console, console, console);
	SpawnProcess("clientB", console, console, console);

	while(1) {
		Yield();
	}
}