#include <System.h>
#include <Message.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <stdlib.h>
#include <stddef.h>

int Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value)
{
	struct MessageHeader hdr;
	union ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };
	int ret;

	msg.msg.type = ProcManagerSubInt;
	msg.msg.u.subInt.irq = irq;
	msg.msg.u.subInt.object = object;
	msg.msg.u.subInt.type = type;
	msg.msg.u.subInt.value = value;

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsOffset = offsetof(union ProcManagerMsg, msg.u.subInt.object);
	hdr.objectsSize = 1;

	Object_Sendxs(__ProcessManager, &hdr, &ret, sizeof(ret));
	return ret;
}

void Interrupt_Unsubscribe(int sub)
{
	union ProcManagerMsg msg;
	int ret;

	msg.msg.type = ProcManagerUnsubInt;
	msg.msg.u.unsubInt.sub = sub;

	Object_Send(__ProcessManager, &msg, sizeof(msg), &ret, sizeof(ret));
}

void Interrupt_Acknowledge(int sub)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerAckInt;
	msg.msg.u.ackInt.sub = sub;

	Object_Send(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}
