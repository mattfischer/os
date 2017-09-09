#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/ProcessFmt.h>
#include <kernel/include/Objects.h>

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	struct ProcessMsg msg;

	msg.type = ProcessMapPhys;
	msg.mapPhys.vaddr = (unsigned int)vaddr;
	msg.mapPhys.paddr = paddr;
	msg.mapPhys.size = size;
	Object_Send(PROCESS_NO, &msg, sizeof(msg), NULL, 0);
}
