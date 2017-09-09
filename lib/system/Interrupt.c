#include <System.h>
#include <Message.h>
#include <Object.h>

#include <kernel/include/MessageFmt.h>
#include <kernel/include/KernelFmt.h>
#include <kernel/include/Objects.h>

#include <stdlib.h>
#include <stddef.h>

int Interrupt_Subscribe(unsigned irq, int object, unsigned type, unsigned value)
{
	struct KernelMsg msg;
	int ret;

	msg.type = KernelSubInt;
	msg.subInt.irq = irq;
	msg.subInt.object = object;
	msg.subInt.type = type;
	msg.subInt.value = value;

	int objectsOffset = offsetof(struct KernelMsg, subInt.object);
	Object_Sendhs(KERNEL_NO, &msg, sizeof(msg), objectsOffset, 1, &ret, sizeof(ret));
	return ret;
}

void Interrupt_Unmask(int irq)
{
	struct KernelMsg msg;
	int ret;

	msg.type = KernelUnmaskInt;
	msg.unmaskInt.irq = irq;

	Object_Send(KERNEL_NO, &msg, sizeof(msg), &ret, sizeof(ret));
}
