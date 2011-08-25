#include "include/System.h"
#include "include/Message.h"

#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	struct ProcManagerMsg msg;

	msg.type = ProcManagerMapPhys;
	msg.u.mapPhys.vaddr = (unsigned int)vaddr;
	msg.u.mapPhys.paddr = paddr;
	msg.u.mapPhys.size = size;
	Object_Send(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}
