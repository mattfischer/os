#include "Sched.h"
#include "Page.h"
#include "AddressSpace.h"
#include "Defs.h"
#include "ProcManager.h"
#include "Object.h"

char InitStack[256];

SECTION_LOW void EntryLow()
{
	Page_InitLow();
	AddressSpace_InitLow();
}

void Entry()
{
	AddressSpace_Init();
	Sched_Init();
	Object_Init();

	ProcManager_Start();
}

void SysEntry(int code)
{
	Sched_RunNext();
}