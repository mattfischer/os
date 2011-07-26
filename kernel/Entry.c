#include "Sched.h"
#include "Page.h"
#include "Map.h"
#include "Defs.h"

char InitStack[256];

SECTION_LOW void EntryLow()
{
	PageInitLow();
	MapInitLow();
}

void Entry()
{
	MapInit();
	SchedInit();

	StartStub();
}

void SysEntry(int code)
{
	Schedule();
}