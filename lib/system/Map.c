#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/ProcManagerFmt.h>
#include <kernel/include/Objects.h>

#include <stddef.h>

void MapPhys(void *vaddr, unsigned int paddr, unsigned int size)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerMapPhys;
	msg.msg.u.mapPhys.vaddr = (unsigned int)vaddr;
	msg.msg.u.mapPhys.paddr = paddr;
	msg.msg.u.mapPhys.size = size;
	Object_Send(PROCMAN_NO, &msg, sizeof(msg), NULL, 0);
}
