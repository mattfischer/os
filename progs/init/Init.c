#include <Yield.h>
#include <Spawn.h>
#include <Name.h>
#include <Object.h>

int main(int argc, char *argv[])
{
	int console;

	SpawnProcess("console", INVALID_OBJECT, INVALID_OBJECT, INVALID_OBJECT);
	Yield();

	console = LookupName("console");
	SpawnProcess("clientA", console, console, console);
	SpawnProcess("clientB", console, console, console);
	while(1) {
		Yield();
	}
}