#include <System.h>
#include <Message.h>

#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerMapPhys;
	msg.msg.u.mapPhys.vaddr = (unsigned int)vaddr;
	msg.msg.u.mapPhys.paddr = paddr;
	msg.msg.u.mapPhys.size = size;
	Object_Send(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}
