#include "include/Map.h"
#include "include/Message.h"

#include <kernel/include/ProcManagerFmt.h>

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	struct MessageHeader hdr;
	struct ProcManagerMsg msg;

	hdr.size = sizeof(msg);
	hdr.body = &msg;
	hdr.objectsSize = 0;
	hdr.objectsOffset = 0;

	msg.type = ProcManagerMapPhys;
	msg.u.mapPhys.vaddr = (unsigned int)vaddr;
	msg.u.mapPhys.paddr = paddr;
	msg.u.mapPhys.size = size;
	SendMessage(0, &hdr, NULL);
}
