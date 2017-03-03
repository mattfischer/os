#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/ProcessFmt.h>
#include <kernel/include/Objects.h>

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	union ProcessMsg msg;

	msg.msg.type = ProcessMapPhys;
	msg.msg.u.mapPhys.vaddr = (unsigned int)vaddr;
	msg.msg.u.mapPhys.paddr = paddr;
	msg.msg.u.mapPhys.size = size;
	Object_Send(PROCESS_NO, &msg, sizeof(msg), NULL, 0);
}
