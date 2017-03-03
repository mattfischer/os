#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/ProcManagerFmt.h>
#include <kernel/include/Objects.h>

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

void Interrupt_Unmask(int irq)
{
	union ProcManagerMsg msg;
	int ret;

	msg.msg.type = ProcManagerUnmaskInt;
	msg.msg.u.unmaskInt.irq = irq;

	Object_Send(PROCMAN_NO, &msg, sizeof(msg), &ret, sizeof(ret));
}
