#include "include/Message.h"

#include <kernel/include/ProcManagerFmt.h>

#include <stddef.h>

void *_sbrk(int inc)
{
	struct ProcManagerMsg msg;

	msg.type = ProcManagerSbrk;
	msg.u.sbrk.increment = inc;
	return (void*)SendMessage(0, &msg, sizeof(msg), NULL, 0);
}