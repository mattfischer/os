#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcManager.h"
#include "Message.h"

char InitStack[256];

SECTION_LOW void EntryLow()
{
	PageInitLow();
	AddressSpaceInitLow();
}

void Entry()
{
	AddressSpaceInit();
	SchedInit();
	MessageInit();

	ProcManagerStart();
}

void SysEntry(int code)
{
	Schedule();
}