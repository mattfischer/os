#include <System.h>
#include <Message.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>

#include "Internal.h"

#include <stdlib.h>
#include <stddef.h>

int Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value)
{
	union ProcManagerMsg msg;
	int ret;

	msg.msg.type = ProcManagerSubInt;
	msg.msg.u.subInt.irq = irq;
	msg.msg.u.subInt.object = object;
	msg.msg.u.subInt.type = type;
	msg.msg.u.subInt.value = value;

	int objectsOffset = offsetof(union ProcManagerMsg, msg.u.subInt.object);
	Object_Sendhs(PROCMAN_NO, &msg, sizeof(msg), objectsOffset, 1, &ret, sizeof(ret));
	return ret;
}

void Interrupt_Unsubscribe(int sub)
{
	union ProcManagerMsg msg;
	int ret;

	msg.msg.type = ProcManagerUnsubInt;
	msg.msg.u.unsubInt.sub = sub;

	Object_Send(PROCMAN_NO, &msg, sizeof(msg), &ret, sizeof(ret));
}

void Interrupt_Acknowledge(int sub)
{
	union ProcManagerMsg msg;

	msg.msg.type = ProcManagerAckInt;
	msg.msg.u.ackInt.sub = sub;

	Object_Send(PROCMAN_NO, &msg, sizeof(msg), NULL, 0);
}
