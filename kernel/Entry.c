#include "Sched.h"
#include "Page.h"
#include "Map.h"
#include "Defs.h"
#include "ProcManager.h"

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

	ProcManagerStart();
}

void SysEntry(int code)
{
	Schedule();
}