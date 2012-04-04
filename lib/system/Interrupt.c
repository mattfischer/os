#include <System.h>
#include <Message.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <stdlib.h>
#include <stddef.h>

void Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value)
{
	struct MessageHeader hdr;
	union ProcManagerMsg msg;
	struct BufferSegment segs[] = { &msg, sizeof(msg) };

	msg.msg.type = ProcManagerSubInt;
	msg.msg.u.subInt.irq = irq;
	msg.msg.u.subInt.object = object;
	msg.msg.u.subInt.type = type;
	msg.msg.u.subInt.value = value;

	hdr.segments = segs;
	hdr.numSegments = 1;
	hdr.objectsOffset = offsetof(union ProcManagerMsg, msg.u.subInt.object);
	hdr.objectsSize = 1;

	Object_Sendxs(__ProcessManager, &hdr, NULL, 0);
}

void Interrupt_Unsubscribe(unsigned irq, int sub)
{
	union ProcManagerMsg msg;
	int ret;

	msg.msg.type = ProcManagerUnsubInt;
	msg.msg.u.unsubInt.irq = irq;
	msg.msg.u.unsubInt.sub = sub;

	Object_Send(__ProcessManager, &msg, sizeof(msg), &ret, sizeof(ret));
}

void Interrupt_Acknowledge(unsigned irq, int sub)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerAckInt;
	msg.msg.u.ackInt.irq = irq;
	msg.msg.u.ackInt.sub = sub;

	Object_Send(__ProcessManager, &msg, sizeof(msg), NULL, 0);
}
